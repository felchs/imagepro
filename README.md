# 🖼️ ImagePro

**A real-time image processing framework built in modern C++** using OpenCV, OpenGL, and optionally AI modules (e.g., age detection).  
Designed for real-time visuals, GPU acceleration, and UI overlays with ImGui.

<img width="1282" height="752" alt="image" src="https://github.com/user-attachments/assets/bbf2c7a2-0790-4ab2-b653-49445754997f" />


## 📚 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Repository Structure](#repository-structure)
- [Dependencies](#dependencies)
- [Build Instructions (Visual Studio 2022 + C++20)](#build-instructions-visual-studio-2022--c20)
- [Usage / Examples](#usage--examples)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)


## 🧭 Overview

**ImagePro** is a modular C++ framework for:
- Real-time image/video processing
- AI integration (e.g., age/gender detection)
- OpenGL GPU rendering
- Interactive GUI overlays with ImGui


## 🔍 Use Cases
- Real-time age detection from webcam feed
- AI-enhanced filtering pipelines
- OpenGL shader visualization
- GUI testing of computer vision pipelines

## ✨ Features

- 📷 **Image Input**: Load from files or webcam using OpenCV
- 🧠 **AI Modules**: Age detection, face recognition (modular)
- 🎮 **GPU Rendering**: Real-time OpenGL texture visualization
- 🛠️ **Extensible**: Easily plug in new detection pipelines
- 🧪 **Diagnostics**: On-screen stats, overlay, and debug via ImGui
- 🔁 **Shader Integration**: Custom visual effects via GLSL

## 📁 Repository Structure

```
imagepro/
├── include/ # Header files (.h/.hpp)
├── src/ # Core C++ implementation
│ ├── AgeDetection.cpp
│ ├── ImageProcessor.cpp
│ ├── Main.cpp
├── shaders/ # GLSL shaders for rendering
├── demo/ # Sample/demo applications
├── assets/ # Images, models, and AI resources
└── README.md
```

## 🔍 Component Highlights

- `AgeDetection.cpp`: Singleton class for AI inference (e.g., age prediction)
- `ImageProcessor.cpp`: Handles preprocessing and filter chains
- `Main.cpp`: Initializes OpenGL context, ImGui UI, camera, and render loop


## 📦 Dependencies

Make sure to install the following libraries (preferably via vcpkg):

| Dependency | Purpose                          |
|------------|----------------------------------|
| OpenCV     | Image capture & processing       |
| GLEW       | OpenGL extension loading         |
| GLFW       | Window and context creation      |
| GLM        | Matrix/vector math for OpenGL    |
| stb_image  | Image loading (textures, etc.)   |
| ImGui      | UI overlays                      |

You can install all via vcpkg:

```bash
.\vcpkg install opencv4 glew glfw3 glm imgui stb
```


## 🛠️ Build Instructions (Visual Studio 2022 + C++20)
These steps will guide you to build ImagePro on Windows using Visual Studio 2022 and vcpkg.

✅ Step 1: Install Visual Studio 2022
Install the following workloads using Visual Studio Installer:
✔️ Desktop development with C++

✅ Step 2: Install Dependencies with vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install opencv4 glew glfw3 glm imgui stb
.\vcpkg integrate install  # Optional, but useful

✅ Step 3: Open and Build in Visual Studio
Open build/imagepro.sln
Select x64 and Release or Debug
Build the solution: Ctrl + Shift + B

✅ Step 5: Run the Executable
Copy required assets (e.g. deploy_age.prototxt, age_net.caffemodel) to the same folder as the executable
Run the .exe from Visual Studio or build/Release/


## 🚀 Usage / Examples
Basic Use Flow:
Launch the app
Select input (webcam or image)
Observe real-time render
Toggle ImGui overlays
Apply filters or enable age detection
You can customize pipeline and features directly via GUI or edit the processor source code.


## 📬 Contact
Author: felchs
Email: supportimagepro@gmail.com
GitHub: github.com/felchs/imagepro
