#include "NAMController.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/fstrdefs.h"
#include <cstring>

using namespace Steinberg;
using namespace Steinberg::Vst;

NAMController::NAMController() : EditControllerEx1() {
}

NAMController::~NAMController() {
    terminate();
}

tresult PLUGIN_API NAMController::initialize(FUnknown* context) {
    tresult result = EditControllerEx1::initialize(context);
    if (result != kResultOk) return result;
    
    // Use the simpler addParameter on the parameters container
    // Gain (-60 to +24 dB)
    parameters.addParameter(STR16("Gain"), STR16("dB"), 0, 0.5, ParameterInfo::kCanAutomate, NAMParams::kParamGain, kRootUnitId, STR16("Gain"));
    
    // Bypass
    parameters.addParameter(STR16("Bypass"), STR16(""), 1, 0.0, ParameterInfo::kCanAutomate, NAMParams::kParamBypass, kRootUnitId, STR16("Bypass"));
    
    // Model Loaded (read-only)
    parameters.addParameter(STR16("Model Loaded"), STR16(""), 1, 0.0, ParameterInfo::kIsReadOnly | ParameterInfo::kIsHidden, NAMParams::kParamModelLoaded, kRootUnitId, STR16("Loaded"));
    
    // Sample Rate (read-only)
    parameters.addParameter(STR16("Sample Rate"), STR16("Hz"), 0, 0.25, ParameterInfo::kIsReadOnly, NAMParams::kParamSampleRate, kRootUnitId, STR16("SR"));
    
    return kResultOk;
}

tresult PLUGIN_API NAMController::terminate() {
    return EditControllerEx1::terminate();
}

IPlugView* PLUGIN_API NAMController::createView(const char* name) {
    return nullptr;
}

tresult PLUGIN_API NAMController::setComponentState(IBStream* state) {
    if (!state) return kResultFalse;
    
    int32 path_len = 0;
    state->read(&path_len, sizeof(path_len));
    if (path_len > 0 && path_len < 1024) {
        std::string path(path_len, '\0');
        state->read(&path[0], path_len);
        m_model_path = path;
        m_model_loaded = !path.empty();
    }
    
    return kResultOk;
}

tresult PLUGIN_API NAMController::setParamNormalized(ParamID tag, ParamValue value) {
    return EditControllerEx1::setParamNormalized(tag, value);
}

tresult PLUGIN_API NAMController::getParamStringByValue(ParamID tag, ParamValue valueNormalized, Vst::String128 string) {
    char temp[128] = {0};
    
    switch (tag) {
        case NAMParams::kParamGain: {
            float db = -60.0f + valueNormalized * 84.0f;
            snprintf(temp, sizeof(temp), "%.1f dB", db);
            UString(string, str16BufferSize(String128)).assign(temp);
            return kResultOk;
        }
        case NAMParams::kParamBypass: {
            const char* str = valueNormalized > 0.5f ? "ON" : "OFF";
            UString(string, str16BufferSize(String128)).assign(str);
            return kResultOk;
        }
        case NAMParams::kParamModelLoaded: {
            const char* str = valueNormalized > 0.5f ? "Yes" : "No";
            UString(string, str16BufferSize(String128)).assign(str);
            return kResultOk;
        }
        case NAMParams::kParamSampleRate: {
            float sr = valueNormalized * 192000.0f;
            snprintf(temp, sizeof(temp), "%.0f Hz", sr);
            UString(string, str16BufferSize(String128)).assign(temp);
            return kResultOk;
        }
    }
    return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

tresult PLUGIN_API NAMController::getParamValueByString(ParamID tag, TChar* string, ParamValue& valueNormalized) {
    return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}