# Pocket Striker

![Build Status](https://github.com/YOUR_USERNAME/PocketStriker/workflows/Build%20Verification%20(Self-Hosted)/badge.svg)

A C++ gameplay prototype built in Unreal Engine 5 demonstrating core gameplay engineering skills through networked player control, motion matching animation, opponent AI, and performance optimization.

## Project Structure

```
Source/PocketStriker/
├── Gameplay/          # Player controller, movement, actions, state machine
├── Animation/         # Motion matching system and animation database
├── AI/                # AI perception, behavior trees, utility-based decisions
├── Network/           # Client prediction, server reconciliation, interpolation
└── Tools/             # Debug HUD, visualizers, parameter editors
```

## Requirements

### Software Prerequisites

- **Unreal Engine 5.3 or later** (tested with UE 5.3-5.6)
  - Download from [Epic Games Launcher](https://www.unrealengine.com/download)
  - Ensure C++ support is installed during UE installation
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Required workloads:
    - "Game development with C++"
    - ".NET desktop development"
  - Required individual components:
    - Windows 10 SDK (10.0.18362.0 or later)
    - MSVC v143 - VS 2022 C++ x64/x86 build tools
- **Windows 10/11** (64-bit)
  - Minimum: Windows 10 version 1909 or later

### Hardware Recommendations

- CPU: Quad-core processor (Intel i5/AMD Ryzen 5 or better)
- RAM: 16 GB minimum, 32 GB recommended
- GPU: DirectX 11/12 compatible with 4GB VRAM
- Storage: 10 GB free space for project and intermediate files

## Build Instructions

### Initial Setup

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd PocketStriker
   ```

2. **Generate Visual Studio project files**
   - Right-click `PocketStriker.uproject` in Windows Explorer
   - Select "Generate Visual Studio project files"
   - Wait for generation to complete (may take 1-2 minutes)
   - This creates `PocketStriker.sln` and project files

3. **Open the solution**
   - Open `PocketStriker.sln` in Visual Studio 2022
   - Wait for IntelliSense to finish indexing

### Build Configurations

The project supports three build configurations:

#### Development Editor (Recommended for Development)
- Includes editor functionality and debugging symbols
- Optimized for iteration speed
- **Build steps:**
  1. Set configuration to "Development Editor" and platform to "Win64"
  2. Build > Build Solution (Ctrl+Shift+B)
  3. Press F5 to launch the Unreal Editor with debugger attached

#### Development (Standalone Game)
- Standalone game build with debugging symbols
- No editor overhead
- **Build steps:**
  1. Set configuration to "Development" and platform to "Win64"
  2. Build > Build Solution
  3. Run from `Binaries/Win64/PocketStriker.exe`

#### Shipping (Distribution)
- Fully optimized for performance
- No debugging symbols or development features
- **Build steps:**
  1. Set configuration to "Shipping" and platform to "Win64"
  2. Build > Build Solution
  3. Package through Unreal Editor for distribution (see Packaging section)

### Verification

After building, verify the compilation succeeded:
- Check Output window for "Build succeeded" message
- Ensure 0 errors (warnings are acceptable)
- Launch the editor to confirm functionality

## Troubleshooting

### Common Build Issues

**Problem: "Cannot find UnrealBuildTool.exe"**
- Solution: Ensure Unreal Engine is properly installed with C++ support
- Verify UE installation path in Epic Games Launcher
- Regenerate project files by right-clicking .uproject

**Problem: "Windows SDK not found"**
- Solution: Install Windows 10 SDK through Visual Studio Installer
- Go to Tools > Get Tools and Features > Individual Components
- Select "Windows 10 SDK (10.0.18362.0)" or later

**Problem: "MSB3073: command exited with code 6"**
- Solution: This indicates a compilation error in C++ code
- Check Error List window for specific C++ errors
- Common causes: missing includes, syntax errors, undefined symbols

**Problem: "Out of memory" during compilation**
- Solution: Close other applications to free RAM
- Reduce parallel compilation: Tools > Options > Projects and Solutions > Build and Run
- Set "maximum number of parallel project builds" to 1 or 2

**Problem: IntelliSense errors but build succeeds**
- Solution: IntelliSense may be out of sync
- Right-click solution > Rescan Solution
- Close and reopen Visual Studio
- Delete `.vs` folder and reopen solution

**Problem: "Module 'PocketStriker' could not be loaded"**
- Solution: Ensure all source files compiled successfully
- Check that .dll files exist in `Binaries/Win64/`
- Rebuild the entire solution (Build > Rebuild Solution)

### Performance Issues

**Slow compilation times:**
- Enable Incredibuild or FastBuild if available
- Use "Live Coding" in Unreal Editor for faster iteration
- Consider upgrading to SSD storage for faster I/O

**Editor crashes on launch:**
- Verify GPU drivers are up to date
- Check Windows Event Viewer for crash details
- Try launching with `-dx11` or `-dx12` command line argument
- Delete `Saved` and `Intermediate` folders and rebuild

### Getting Help

If you encounter issues not covered here:
1. Check the Error List window in Visual Studio for specific errors
2. Review the Output window for detailed build logs
3. Consult [Unreal Engine Documentation](https://docs.unrealengine.com/)
4. Check project issues on GitHub

## Packaging for Distribution

To create a standalone executable for distribution:

1. **Open the project in Unreal Editor**
2. **Configure packaging settings:**
   - File > Package Project > Packaging Settings
   - Set "Build Configuration" to Shipping (or Development for testing)
   - Ensure target platform is set to Windows
   - Configure maps to include in package
3. **Package the project:**
   - File > Package Project > Windows > Windows (64-bit)
   - Select output directory
   - Wait for packaging to complete (10-30 minutes)
4. **Test the packaged build:**
   - Navigate to output directory
   - Run `WindowsNoEditor/PocketStriker.exe`
   - Verify all features work without Unreal Engine installed
   - Test on a clean machine without development tools

**For detailed packaging instructions, troubleshooting, and distribution options, see [PACKAGING.md](PACKAGING.md)**

### Quick Packaging Commands

**Development Build (with debugging):**
```powershell
# Through Editor: File > Package Project > Windows (64-bit)
# Set Build Configuration to "Development" in Project Settings
```

**Shipping Build (optimized for distribution):**
```powershell
# Through Editor: File > Package Project > Windows (64-bit)
# Set Build Configuration to "Shipping" in Project Settings
```

**Command Line Packaging:**
```powershell
$UEPath = "C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles"
& "$UEPath\RunUAT.bat" BuildCookRun -project="PocketStriker.uproject" -platform=Win64 -clientconfig=Shipping -cook -build -stage -pak -archive
```

## Module Architecture

The project is organized into distinct modules for maintainability:

- **Gameplay Module**: Core player systems and data-driven tuning
- **Animation Module**: Motion matching with blendspace fallback
- **AI Module**: Perception, utility-based decisions, and steering
- **Network Module**: Authoritative server with client-side prediction
- **Tools Module**: Debug visualization and runtime parameter editing

## Continuous Integration

This project includes GitHub Actions workflows for automated build verification:

- **build-simple.yml**: Validates project structure and JSON syntax on every push/PR
- **build.yml**: Full compilation workflow (requires self-hosted runner with UE installed)

### Setting Up CI for Your Fork

**Option 1: Validation Only (GitHub-hosted runners)**
- The validation workflow runs automatically on GitHub-hosted runners
- Checks project structure, JSON syntax, and common issues
- No additional setup required

**Option 2: Full Build (Self-hosted runner)**
1. Set up a Windows machine with:
   - Unreal Engine 5.3+ installed
   - Visual Studio 2022 with C++ workloads
   - GitHub Actions runner installed
2. Register the runner with your repository
3. Edit `.github/workflows/build-simple.yml` and set `if: false` to `if: true` in the build job
4. Push to trigger the full build workflow

**Note:** GitHub-hosted runners do not include Unreal Engine due to licensing and size constraints. For full CI/CD, a self-hosted runner or cloud-based UE build service is recommended.

## Status

Project structure and core module architecture established. Implementation in progress.

## License

Copyright Epic Games, Inc. All Rights Reserved.
