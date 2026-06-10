#pragma once

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace NAMParams {
    // Parameter IDs
    enum ParamID : Steinberg::Vst::ParamID {
        kParamGain = 0,
        kParamBypass = 1,
        kParamModelLoaded = 2,
        kParamSampleRate = 3,
    };

    // Parameter tags for automation
    inline constexpr Steinberg::Vst::ParamID kNumParams = 4;

    // Default values
    inline constexpr float kDefaultGain = 0.0f;           // dB
    inline constexpr float kDefaultBypass = 0.0f;         // 0 = off, 1 = on
    inline constexpr float kDefaultModelLoaded = 0.0f;    // 0 = no, 1 = yes
    inline constexpr float kDefaultSampleRate = 48000.0f; // Hz
}