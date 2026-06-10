#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/base/ibstream.h"
#include "NAMWaveNet.h"
#include "NAMFileParser.h"
#include "NAMParameters.h"
#include "NAMUIDs.h"
#include <string>
#include <atomic>
#include <mutex>

class NAMProcessor : public Steinberg::Vst::AudioEffect {
public:
    NAMProcessor();
    ~NAMProcessor() override;
    
    // Factory method
    static Steinberg::FUnknown* createInstance(void* context) {
        return (Steinberg::Vst::IAudioProcessor*)new NAMProcessor();
    }
    
    // AudioEffect overrides
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    
    // Internal
    void loadModel(const std::string& filepath);
    void unloadModel();
    bool hasModel() const { return m_wavenet.isReady(); }
    const std::string& getModelPath() const { return m_model_path; }
    const std::string& getModelName() const { return m_wavenet.getModelName(); }
    
    // Parameter handling
    void updateParameters(Steinberg::Vst::IParameterChanges* paramChanges);
    
protected:
    NAMWaveNet m_wavenet;
    NAMModelConfig m_model_config;
    std::string m_model_path;
    
    // Parameters (atomic for thread safety)
    std::atomic<float> m_gain_db{0.0f};
    std::atomic<float> m_bypass{0.0f};
    
    // Sample rate
    double m_sample_rate = 48000.0;
    
    // For model loading from UI thread
    mutable std::mutex m_model_mutex;
    std::string m_pending_model_path;
    std::atomic<bool> m_model_load_pending{false};
};