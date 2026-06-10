#pragma once

#include <string>
#include <vector>
#include <optional>
#include <Eigen/Dense>
#include "json.hpp"

struct NAMLayerConfig {
    int input_size = 1;
    int condition_size = 1;
    int head_size = 8;
    int channels = 16;
    int kernel_size = 3;
    std::vector<int> dilations;
    std::string activation = "Tanh";
    bool gated = false;
    bool head_bias = false;
};

struct NAMModelConfig {
    std::string version;
    std::string architecture;
    std::vector<NAMLayerConfig> layers;
    // head config (optional)
    std::optional<int> head;
    float head_scale = 0.02f;
    double sample_rate = 48000.0;
    
    // Metadata
    struct Metadata {
        std::string name;
        std::string modeled_by;
        std::string gear_make;
        std::string gear_model;
        std::string gear_type;      // amp, pedal, pedal_amp, amp_cab, etc.
        std::string tone_type;      // clean, overdrive, crunch, hi_gain, fuzz
        double input_level_dbu = 0.0;
        double output_level_dbu = 0.0;
        double loudness = 0.0;
        double gain = 0.0;
    } metadata;
    
    // Weights (flattened)
    std::vector<float> weights;
};

class NAMFileParser {
public:
    static std::optional<NAMModelConfig> parse(const std::string& filepath);
    static std::optional<NAMModelConfig> parseFromJSON(const std::string& json_string);
    
    // Extract layer weights from flat weight array
    static std::vector<Eigen::MatrixXf> extractLayerWeights(const NAMModelConfig& config);
    static std::vector<Eigen::VectorXf> extractLayerBiases(const NAMModelConfig& config);
    
private:
    static NAMLayerConfig parseLayerConfig(const nlohmann::json& j);
};