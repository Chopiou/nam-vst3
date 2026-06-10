#include "NAMWaveNet.h"
#include "NAMFileParser.h"
#include <cmath>
#include <algorithm>
#include <iostream>

NAMWaveNet::NAMWaveNet(const NAMModelConfig& config) {
    initialize(config);
}

bool NAMWaveNet::initialize(const NAMModelConfig& config) {
    if (config.layers.empty() || config.weights.empty()) {
        std::cerr << "Invalid config: no layers or weights" << std::endl;
        return false;
    }
    
    m_model_name = config.metadata.name;
    m_sample_rate = config.sample_rate;
    m_head_scale = config.head_scale;
    
    // Extract weights and biases
    auto layer_weights = NAMFileParser::extractLayerWeights(config);
    auto layer_biases = NAMFileParser::extractLayerBiases(config);
    
    if (layer_weights.size() != config.layers.size() || 
        layer_biases.size() != config.layers.size()) {
        std::cerr << "Weight/bias extraction failed: size mismatch" << std::endl;
        return false;
    }
    
    m_layers.clear();
    m_layers.reserve(config.layers.size());
    
    for (size_t i = 0; i < config.layers.size(); ++i) {
        Layer layer;
        layer.config = config.layers[i];
        layer.weights = std::move(layer_weights[i]);
        layer.bias = std::move(layer_biases[i]);
        
        // Initialize history buffer
        int max_dilation = *std::max_element(layer.config.dilations.begin(), 
                                             layer.config.dilations.end());
        int history_size = max_dilation * layer.config.kernel_size;
        layer.state.history = Eigen::MatrixXf::Zero(layer.config.channels, history_size);
        layer.state.write_pos = 0;
        
        // Precompute dilation offsets for efficient indexing
        layer.dilation_offsets.reserve(layer.config.dilations.size());
        for (int d : layer.config.dilations) {
            layer.dilation_offsets.push_back(d * (layer.config.kernel_size - 1));
        }
        
        m_layers.push_back(std::move(layer));
    }
    
    m_latency = computeLatency();
    m_ready = true;
    
    std::cout << "NAMWaveNet initialized: " << m_layers.size() << " layers, "
              << "latency=" << m_latency << " samples, model=" << m_model_name << std::endl;
    
    return true;
}

void NAMWaveNet::reset() {
    for (auto& layer : m_layers) {
        layer.state.history.setZero();
        layer.state.write_pos = 0;
    }
}

float NAMWaveNet::applyActivation(float x, const std::string& activation) const {
    if (activation == "Tanh") return std::tanh(x);
    if (activation == "ReLU") return std::max(0.0f, x);
    if (activation == "HardTanh") return std::max(-1.0f, std::min(1.0f, x));
    if (activation == "Sigmoid") return 1.0f / (1.0f + std::exp(-x));
    return std::tanh(x); // default
}

Eigen::VectorXf NAMWaveNet::processLayer(Layer& layer, const Eigen::VectorXf& input, 
                                          const Eigen::VectorXf& condition) {
    int out_channels = layer.config.channels;
    int in_channels = layer.config.input_size + layer.config.condition_size;
    int kernel_size = layer.config.kernel_size;
    
    Eigen::VectorXf output(out_channels);
    
    // For each output channel
    for (int oc = 0; oc < out_channels; ++oc) {
        float sum = layer.bias(oc);
        
        // For each dilation
        for (size_t dil_idx = 0; dil_idx < layer.config.dilations.size(); ++dil_idx) {
            int dilation = layer.config.dilations[dil_idx];
            int offset = layer.dilation_offsets[dil_idx];
            
            // Read from history buffer at dilated positions
            for (int k = 0; k < kernel_size; ++k) {
                int hist_idx = (layer.state.write_pos - offset - k * dilation) % layer.state.history.cols();
                if (hist_idx < 0) hist_idx += layer.state.history.cols();
                
                // Input channels
                for (int ic = 0; ic < layer.config.input_size; ++ic) {
                    int weight_idx = oc * (in_channels * kernel_size) + ic * kernel_size + k;
                    sum += layer.weights(oc, weight_idx) * layer.state.history(oc, hist_idx);
                }
                
                // Condition channels
                for (int cc = 0; cc < layer.config.condition_size; ++cc) {
                    int weight_idx = oc * (in_channels * kernel_size) + 
                                   (layer.config.input_size + cc) * kernel_size + k;
                    sum += layer.weights(oc, weight_idx) * condition(cc);
                }
            }
        }
        
        output(oc) = applyActivation(sum, layer.config.activation);
    }
    
    // Update history buffer for input channels
    // We store the input (not output) in history for next layer
    for (int ic = 0; ic < layer.config.input_size; ++ic) {
        int hist_idx = layer.state.write_pos % layer.state.history.cols();
        layer.state.history(ic, hist_idx) = input(ic);
    }
    layer.state.write_pos = (layer.state.write_pos + 1) % layer.state.history.cols();
    
    return output;
}

float NAMWaveNet::processSample(float input, float condition) {
    if (!m_ready) return input;
    
    Eigen::VectorXf x(1);
    x(0) = input;
    Eigen::VectorXf cond(1);
    cond(0) = condition;
    
    // Process through all layers
    for (auto& layer : m_layers) {
        x = processLayer(layer, x, cond);
    }
    
    // Apply head scale
    return x(0) * m_head_scale;
}

void NAMWaveNet::processBlock(const float* input, float* output, int num_samples, float condition) {
    if (!m_ready) {
        std::copy(input, input + num_samples, output);
        return;
    }
    
    for (int i = 0; i < num_samples; ++i) {
        output[i] = processSample(input[i], condition);
    }
}

int NAMWaveNet::computeLatency() const {
    int total_latency = 0;
    for (const auto& layer : m_layers) {
        int max_dil = *std::max_element(layer.config.dilations.begin(), 
                                        layer.config.dilations.end());
        total_latency += max_dil * (layer.config.kernel_size - 1);
    }
    return total_latency;
}