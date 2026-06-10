#include "NAMFileParser.h"
#include <fstream>
#include <iostream>

std::optional<NAMModelConfig> NAMFileParser::parse(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return std::nullopt;
    }
    
    std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parseFromJSON(json_str);
}

std::optional<NAMModelConfig> NAMFileParser::parseFromJSON(const std::string& json_string) {
    try {
        auto j = nlohmann::json::parse(json_string);
        NAMModelConfig config;
        
        config.version = j.value("version", "0.5.0");
        config.architecture = j.value("architecture", "WaveNet");
        config.sample_rate = j.value("sample_rate", 48000.0);
        
        // Parse head config
        if (j.contains("config") && j["config"].contains("head")) {
            config.head = j["config"]["head"].get<int>();
        }
        config.head_scale = j.value("config.head_scale", 0.02f);
        
        // Parse layers
        if (j.contains("config") && j["config"].contains("layers")) {
            for (const auto& layer_j : j["config"]["layers"]) {
                config.layers.push_back(parseLayerConfig(layer_j));
            }
        }
        
        // Parse metadata
        if (j.contains("metadata")) {
            auto& m = j["metadata"];
            config.metadata.name = m.value("name", "");
            config.metadata.modeled_by = m.value("modeled_by", "");
            config.metadata.gear_make = m.value("gear_make", "");
            config.metadata.gear_model = m.value("gear_model", "");
            config.metadata.gear_type = m.value("gear_type", "");
            config.metadata.tone_type = m.value("tone_type", "");
            config.metadata.input_level_dbu = m.value("input_level_dbu", 0.0);
            config.metadata.output_level_dbu = m.value("output_level_dbu", 0.0);
            config.metadata.loudness = m.value("loudness", 0.0);
            config.metadata.gain = m.value("gain", 0.0);
        }
        
        // Parse weights
        if (j.contains("weights")) {
            config.weights.reserve(j["weights"].size());
            for (const auto& w : j["weights"]) {
                config.weights.push_back(w.get<float>());
            }
        }
        
        return config;
        
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

NAMLayerConfig NAMFileParser::parseLayerConfig(const nlohmann::json& layer_j) {
    NAMLayerConfig layer;
    layer.input_size = layer_j.value("input_size", 1);
    layer.condition_size = layer_j.value("condition_size", 1);
    layer.head_size = layer_j.value("head_size", 8);
    layer.channels = layer_j.value("channels", 16);
    layer.kernel_size = layer_j.value("kernel_size", 3);
    layer.activation = layer_j.value("activation", "Tanh");
    layer.gated = layer_j.value("gated", false);
    layer.head_bias = layer_j.value("head_bias", false);
    
    if (layer_j.contains("dilations")) {
        for (const auto& d : layer_j["dilations"]) {
            layer.dilations.push_back(d.get<int>());
        }
    }
    return layer;
}

std::vector<Eigen::MatrixXf> NAMFileParser::extractLayerWeights(const NAMModelConfig& config) {
    std::vector<Eigen::MatrixXf> result;
    size_t weight_idx = 0;
    
    for (const auto& layer : config.layers) {
        // For WaveNet: weight shape is [channels, input_size + condition_size, kernel_size]
        int in_channels = layer.input_size + layer.condition_size;
        int out_channels = layer.channels;
        int kernel_size = layer.kernel_size;
        
        int weight_count = out_channels * in_channels * kernel_size;
        if (weight_idx + weight_count > config.weights.size()) break;
        
        Eigen::MatrixXf W(out_channels, in_channels * kernel_size);
        for (int oc = 0; oc < out_channels; ++oc) {
            for (int ic = 0; ic < in_channels * kernel_size; ++ic) {
                W(oc, ic) = config.weights[weight_idx++];
            }
        }
        result.push_back(W);
    }
    
    return result;
}

std::vector<Eigen::VectorXf> NAMFileParser::extractLayerBiases(const NAMModelConfig& config) {
    std::vector<Eigen::VectorXf> result;
    size_t weight_idx = 0;
    
    // First skip all weights
    for (const auto& layer : config.layers) {
        int in_channels = layer.input_size + layer.condition_size;
        int out_channels = layer.channels;
        int kernel_size = layer.kernel_size;
        weight_idx += out_channels * in_channels * kernel_size;
    }
    
    // Then extract biases (one per output channel)
    for (const auto& layer : config.layers) {
        int out_channels = layer.channels;
        if (weight_idx + out_channels > config.weights.size()) break;
        
        Eigen::VectorXf b(out_channels);
        for (int oc = 0; oc < out_channels; ++oc) {
            b(oc) = config.weights[weight_idx++];
        }
        result.push_back(b);
    }
    
    return result;
}