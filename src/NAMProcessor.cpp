#include "NAMProcessor.h"
#include "pluginterfaces/base/ibstream.h"
#include <cmath>
#include <iostream>

using namespace Steinberg;
using namespace Steinberg::Vst;

NAMProcessor::NAMProcessor() : AudioEffect() {
    // Set controller class UID
    setControllerClass(NAMControllerUID);
}

NAMProcessor::~NAMProcessor() {
    terminate();
}

tresult PLUGIN_API NAMProcessor::initialize(FUnknown* context) {
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk) return result;
    
    // Add audio buses: stereo in, stereo out
    addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);
    
    return kResultOk;
}

tresult PLUGIN_API NAMProcessor::terminate() {
    m_wavenet.reset();
    return AudioEffect::terminate();
}

tresult PLUGIN_API NAMProcessor::setActive(TBool state) {
    if (state) {
        m_wavenet.reset();
    }
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API NAMProcessor::setupProcessing(ProcessSetup& setup) {
    m_sample_rate = setup.sampleRate;
    return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API NAMProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
    // Support 32-bit float and 64-bit double
    return (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64) ? kResultOk : kResultFalse;
}

tresult PLUGIN_API NAMProcessor::process(ProcessData& data) {
    // Update parameters from controller
    if (data.inputParameterChanges) {
        updateParameters(data.inputParameterChanges);
    }
    
    // Check for pending model load (from UI thread)
    if (m_model_load_pending.load()) {
        std::lock_guard<std::mutex> lock(m_model_mutex);
        if (!m_pending_model_path.empty()) {
            loadModel(m_pending_model_path);
            m_pending_model_path.clear();
            m_model_load_pending.store(false);
        }
    }
    
    // Process audio
    if (data.numInputs == 0 || data.numOutputs == 0) return kResultOk;
    
    int32 num_channels = std::min(data.inputs[0].numChannels, data.outputs[0].numChannels);
    int32 num_samples = data.numSamples;
    
    // Get gain factor
    float gain_linear = std::pow(10.0f, m_gain_db.load() / 20.0f);
    bool bypass = m_bypass.load() > 0.5f;
    
    // Process each channel
    for (int32 ch = 0; ch < num_channels; ++ch) {
        float* in = static_cast<float*>(data.inputs[0].channelBuffers32[ch]);
        float* out = static_cast<float*>(data.outputs[0].channelBuffers32[ch]);
        
        if (bypass || !m_wavenet.isReady()) {
            // Passthrough with gain
            for (int32 i = 0; i < num_samples; ++i) {
                out[i] = in[i] * gain_linear;
            }
        } else {
            // Process through WaveNet
            m_wavenet.processBlock(in, out, num_samples, 0.0f);
            
            // Apply gain
            for (int32 i = 0; i < num_samples; ++i) {
                out[i] *= gain_linear;
            }
        }
    }
    
    return kResultOk;
}

tresult PLUGIN_API NAMProcessor::setState(IBStream* state) {
    if (!state) return kResultFalse;
    
    // Read model path
    int32 path_len = 0;
    state->read(&path_len, sizeof(path_len));
    if (path_len > 0 && path_len < 1024) {
        std::string path(path_len, '\0');
        state->read(&path[0], path_len);
        m_pending_model_path = path;
        m_model_load_pending.store(true);
    }
    
    // Read gain
    float gain = 0.0f;
    state->read(&gain, sizeof(gain));
    m_gain_db.store(gain);
    
    // Read bypass
    float bypass = 0.0f;
    state->read(&bypass, sizeof(bypass));
    m_bypass.store(bypass);
    
    return kResultOk;
}

tresult PLUGIN_API NAMProcessor::getState(IBStream* state) {
    if (!state) return kResultFalse;
    
    // Write model path
    std::string path = m_model_path;
    int32 path_len = static_cast<int32>(path.size());
    state->write(&path_len, sizeof(path_len));
    if (path_len > 0) {
        state->write(const_cast<void*>(static_cast<const void*>(path.c_str())), path_len);
    }
    
    // Write gain
    float gain = m_gain_db.load();
    state->write(&gain, sizeof(gain));
    
    // Write bypass
    float bypass = m_bypass.load();
    state->write(&bypass, sizeof(bypass));
    
    return kResultOk;
}

void NAMProcessor::updateParameters(IParameterChanges* paramChanges) {
    int32 paramCount = paramChanges->getParameterCount();
    for (int32 i = 0; i < paramCount; ++i) {
        auto* paramQueue = paramChanges->getParameterData(i);
        if (!paramQueue) continue;
        
        ParamValue value;
        int32 sampleOffset;
        int32 numPoints = paramQueue->getPointCount();
        
        // Use the last point
        if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultOk) {
            switch (paramQueue->getParameterId()) {
                case NAMParams::kParamGain:
                    m_gain_db.store(value);
                    break;
                case NAMParams::kParamBypass:
                    m_bypass.store(value);
                    break;
            }
        }
    }
}

void NAMProcessor::loadModel(const std::string& filepath) {
    auto config = NAMFileParser::parse(filepath);
    if (!config) {
        std::cerr << "Failed to load model: " << filepath << std::endl;
        return;
    }
    
    if (m_wavenet.initialize(*config)) {
        m_model_config = *config;
        m_model_path = filepath;
        std::cout << "Model loaded: " << filepath << std::endl;
    }
}

void NAMProcessor::unloadModel() {
    m_wavenet.reset();
    m_model_config = NAMModelConfig{};
    m_model_path.clear();
}