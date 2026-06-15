#include "NAMParameters.h"
#include <cmath>
#include <algorithm>

namespace NAMParams {
    
    // Convert normalized [0,1] to dB [-60, +24]
    float normalizedToDB(float normalized) {
        return -60.0f + normalized * 84.0f;
    }
    
    // Convert dB [-60, +24] to normalized [0,1]
    float dbToNormalized(float db) {
        return (db + 60.0f) / 84.0f;
    }
    
    // Clamp value
    float clamp(float value, float min, float max) {
        return std::max(min, std::min(max, value));
    }
}