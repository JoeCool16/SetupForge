![SetupForge](https://raw.githubusercontent.com/JoeCool16/SetupForge/refs/heads/master/app_icon.ico)
# SetupForge

**SetupForge** is a Windows desktop application built with C++ and wxWidgets that automates software installation and system configuration tasks. It allows users to create, edit, and execute custom installation scripts to perform operations such as running executables, moving files, creating directories, managing environment variables, editing the registry, and mapping network drives.

---

## 📂 Features

- **Run Executable Files**: Automate the execution of `.exe` files.
- **File Management**: Move files and create folders.
- **Environment Variables**: Add or edit user/system environment variables.
- **Windows Registry**: Modify registry values for system customization.
- **Drive Mapping**: Connect to network drives easily.
- **Script Resumption**: Automatically resumes after system restarts.
- **User-Friendly Interface**: Simple GUI for script creation and execution.
- **Error Handling & Logging**: Tracks issues and logs progress for troubleshooting.

---

## 🚀 Getting Started

### Prerequisites

Ensure you have the following dependencies installed on your system:

- **Windows OS** (Windows 10/11 recommended)
- **Visual Studio** (for compiling the source code)
- **wxWidgets** (latest version)
- **CMake** (for build management)
- **Git** (for source code management)

---

### Installation

#### 1. Download SetupForge

You can download the latest release from the [GitHub Releases](https://github.com/JoeCool16/SetupForge/releases).

#### 2. Run the Script Creator

1. Download `SetupForge.exe` from the link above.
2. Double-click the `.exe` file to run the application.
   
---

### Build from Source (Optional)

If you want to build SetupForge from the source code, follow these steps:

```bash
# Clone the repository
git clone https://github.com/chrisapton/SetupForge.git

# Navigate to the project directory
cd SetupForge

# Create a build directory
mkdir build && cd build

# Run CMake
cmake ..

# Build the project
cmake --build .
