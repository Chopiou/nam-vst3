#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/ibstream.h"
#include "NAMParameters.h"
#include "NAMUIDs.h"
#include <string>

class NAMController : public Steinberg::Vst::EditControllerEx1 {
public:
    NAMController();
    ~NAMController() override;
    
    // Factory method
    static Steinberg::FUnknown* createInstance(void* context) {
        return (Steinberg::Vst::IEditController*)new NAMController();
    }
    
    // EditController
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::IPlugView* PLUGIN_API createView(const char* name) override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    
    // Parameter handling
    Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value) override;
    Steinberg::tresult PLUGIN_API getParamStringByValue(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized, Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API getParamValueByString(Steinberg::Vst::ParamID tag, Steinberg::Vst::TChar* string, Steinberg::Vst::ParamValue& valueNormalized) override;
    
protected:
    std::string m_model_path;
    bool m_model_loaded = false;
};