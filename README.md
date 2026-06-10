# NAMLoader - VST3 Neural Amp Modeler Plugin

Minimal VST3 plugin for loading and playing `.nam` files (Neural Amp Modeler format).

## Features

- ✅ Loads `.nam` files (WaveNet architecture)
- ✅ Real-time neural network inference (dilated causal convolutions)
- ✅ VST3 parameters: Gain (-60 to +24 dB), Bypass, Model Loaded, Sample Rate
- ✅ Cross-platform: Linux, macOS, Windows
- ⏳ Custom GUI with drag-drop file loading (planned)

## Build

### Prerequisites

- CMake 3.16+
- C++17 compiler
- Eigen3
- VST3 SDK (included as submodule)

### Linux

```bash
sudo apt-get install cmake build-essential libeigen3-dev libx11-dev libxext-dev libxrender-dev libxrandr-dev libxi-dev
git clone --recursive <repo>
cd nam-vst3
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j$(nproc)
# Plugin at: build/NAMLoader.vst3
```

### macOS

```bash
brew install cmake eigen
git clone --recursive <repo>
cd nam-vst3
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build . --config Release
codesign --force --deep --sign "Developer ID Application: Your Name" build/NAMLoader.vst3
# Plugin at: build/NAMLoader.vst3
```

### Windows (Visual Studio)

```bash
git clone --recursive <repo>
cd nam-vst3
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
# Plugin at: build/Release/NAMLoader.vst3
```

## Install

### Linux
```bash
mkdir -p ~/.vst3
cp -r build/NAMLoader.vst3 ~/.vst3/
```

### macOS
```bash
cp -r build/NAMLoader.vst3 ~/Library/Audio/Plug-Ins/VST3/
# or system-wide:
sudo cp -r build/NAMLoader.vst3 /Library/Audio/Plug-Ins/VST3/
```

### Windows
```
%LOCALAPPDATA%\Programs\Common\VST3\
# or system-wide:
C:\Program Files\Common Files\VST3\
```

## Usage

1. Load plugin in your DAW (Ardour, Reaper, Bitwig, Carla, etc.)
2. Currently no custom GUI - host shows generic parameter editor
3. Parameters available:
   - **Gain**: -60 to +24 dB
   - **Bypass**: On/Off
   - **Model Loaded**: Read-only status
   - **Sample Rate**: Read-only

## .nam Format

Supports Neural Amp Modeler `.nam` files (JSON):
- Version, architecture (WaveNet/LSTM)
- Layer configurations with dilated convolutions
- Flattened weight arrays
- Metadata: name, gear type, tone type, gain, loudness

Example: `Manley varimu limit -slow ATT- med REL -gain2.nam` (WaveNet, 2 layers, 16/8 channels)

## CI/CD

GitHub Actions workflow (`.github/workflows/build.yml`) builds for all 3 platforms on push/tag.

## License

MIT