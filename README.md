# Hulusi-Synthesizer

这个仓库现在分为两层：

1. **算法核心（MATLAB）**：以后只需要人工修改这里的 `.m` 文件。
2. **插件工程（C++ / JUCE）**：基于 MATLAB 提炼出的参数与模型，构建可在 REAPER 运行的 VST3。

---

## 目录结构

- `algorithm-core/matlab/`
  - `harmAnalysis.m`
  - `readAllAnalysisAll.m`
  - `playYueguang.m`
  - `HulusiVSTPlugin.m`（MATLAB 原型插件，便于快速试听和算法验证）
- `vst-juce/`
  - `CMakeLists.txt`
  - `include/HulusiEngine.h`（从 MATLAB 模型迁移到 C++ 的核心合成逻辑）
  - `src/PluginProcessor.*`
  - `src/PluginEditor.*`

---

## 工作流（推荐）

### 第 1 步：在 MATLAB 算法核心中迭代
只在 `algorithm-core/matlab/` 修改算法。

可以继续使用：
- `harmAnalysis.m` 做录音分析
- `playYueguang.m` 做音色验证
- `HulusiVSTPlugin.m` 用 `audioTestBench` 快速试听

### 第 2 步：把稳定参数迁移到 JUCE
当前 `vst-juce/include/HulusiEngine.h` 已内置：
- 10 组音高谐波表
- log2 频率轴插值（扩展到完整 MIDI 音域）
- 包络 / 呼吸噪声 / 颤音

当更新 MATLAB 参数后，把对应数值同步到 `HulusiEngine.h` 即可。

### 第 3 步：构建 VST3（可用于 REAPER）

```bash
cd vst-juce
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

如果的环境不能访问 GitHub，请先准备本地 JUCE 源码，然后这样构建：

```bash
cd vst-juce
cmake -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_SOURCE_DIR=/path/to/JUCE
cmake --build build --config Release
```

构建产物通常在类似路径：
- `vst-juce/build/HulusiVST_artefacts/Release/VST3/HulusiVST.vst3`

### 第 4 步：在 REAPER 中加载
把 `.vst3` 复制到系统 VST3 目录：
- Windows: `C:\Program Files\Common Files\VST3`
- macOS: `/Library/Audio/Plug-Ins/VST3`
- Linux: `~/.vst3` 或 `/usr/lib/vst3`

然后在 REAPER:
1. Preferences -> Plug-ins -> VST
2. Re-scan
3. 新建 track，插入 `HulusiVST`

---

## 打包发布

本仓库提供基础打包脚本：

```bash
bash tools/package_vst.sh
```

同样支持离线 JUCE：

```bash
JUCE_SOURCE_DIR=/path/to/JUCE bash tools/package_vst.sh
```

该脚本会：
1. 配置并编译 Release
2. 查找 `HulusiVST.vst3`
3. 打包为 `dist/hulusi-vst3-<platform>.zip`

可以把 zip 直接用于内部测试分发，或附到 GitHub Release。

---

## 说明

- 当前 JUCE 工程使用 CMake + FetchContent 拉取 JUCE（首次构建需要网络）。
- 当前版本是**单声部**原型，后续可扩展：ADSR、Legato、Portamento、多音复音、滤波器与动态响应。
