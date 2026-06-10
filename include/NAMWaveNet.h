#pragma once

#include <vector>
#include <Eigen/Dense>
#include "NAMFileParser.h"

class NAMWaveNet {
public:
    struct LayerState {
        Eigen::MatrixXf history;  // [channels, max_dilation * kernel_size]
        int write_pos = 0;
    };
    
    NAMWaveNet() = default;
    explicit NAMWaveNet(const NAMModelConfig& config);
    
    bool initialize(const NAMModelConfig& config);
    void reset();
    
    // Process single sample (for real-time)
    float processSample(float input, float condition = 0.0f);
    
    // Process block (for offline/render)
    void processBlock(const float* input, float* output, int num_samples, float condition = 0.0f);
    
    bool isReady() const { return m_ready; }
    int getLatency() const { return m_latency; }
    double getSampleRate() const { return m_sample_rate; }
    const std::string& getModelName() const { return m_model_name; }
    
private:
    struct Layer {
        NAMLayerConfig config;
        Eigen::MatrixXf weights;   // [out_channels, in_channels * kernel_size]
        Eigen::VectorXf bias;      // [out_channels]
        LayerState state;
        std::vector<int> dilation_offsets;
    };
    
    std::vector<Layer> m_layers;
    bool m_ready = false;
    int m_latency = 0;
    double m_sample_rate = 48000.0;
    std::string m_model_name;
    float m_head_scale = 0.02f;
    
    // Activation functions
    float applyActivation(float x, const std::string& activation) const;
    
    // Process one layer
    Eigen::VectorXf processLayer(Layer& layer, const Eigen::VectorXf& input, const Eigen::VectorXf& condition);
    
    // Compute receptive field (max latency)
    int computeLatency() const;
};