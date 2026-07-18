# Remote Desktop Connection (LR_02)

A cross-platform, high-performance, and modern Remote Desktop application built using **C++**, **Qt6 (QML)**, and **CMake**.

The application supports real-time screen streaming, system audio loopback capture, clipboard synchronization, and mouse/keyboard input simulation on both **Windows** and **Linux (X11)**.

---

## 🌟 Key Features

### 1. Screen Capturing & Streaming
* **Low Latency:** Detects frame differences using fast binary memory comparison (`memcmp`) of raw pixel data to only transmit updated regions of the screen, minimizing network bandwidth and CPU usage.
* **Flexible Image Quality:**
  * **Optimized (Default):** Compresses frames as JPEG with 90% quality, balancing display clarity and network consumption.
  * **Lossless:** Converts frames to RGB32 format and compresses them using Zlib (compression level 1 - fastest) to deliver pixel-perfect clarity.

### 2. System Audio Loopback Capture
* **Windows (WASAPI):** Uses the WASAPI Loopback API (`AUDCLNT_STREAMFLAGS_LOOPBACK`) to record system audio directly from the default output device. It automatically converts 32-bit Float PCM to 16-bit Int PCM to reduce bandwidth and ensure cross-platform playback compatibility.
* **Linux (PulseAudio / parec):** Captures audio from the default monitor sink using the `parec` tool, with a fallback to `QAudioSource` querying `monitor` or `loopback` devices.
* **Real-time Playback:** Synchronizes audio streams directly to the client using `QAudioSink`, with support for Mute/Unmute controls.

### 3. Mouse and Keyboard Input Simulation
* **Precise Coordinate Mapping:** The client leverages the `ScreenRenderer` component to accurately map local QML window coordinates to the host's native screen resolution.
* **Windows Simulation:** Utilizes the Win32 `SendInput` API to simulate mouse actions (movement, left/right/middle click, wheel scroll) and keyboard presses/releases.
* **Linux Simulation:** Utilizes X11 system libraries and the **XTest** extension (`XTestFakeMotionEvent`, `XTestFakeButtonEvent`, `XTestFakeKeyEvent`).

### 4. Clipboard Synchronization
* Dual-way real-time text synchronization between host and client clipboards during an active remote session.

### 5. Security & User Experience
* **One-time Passcode:** Generates a random 6-digit connection PIN on the host to authenticate incoming client connections.
* **Single Instance Protection:** Uses `QSharedMemory` to ensure only one instance of the application runs on the system at any given time.
* **System Tray Minimization:** For safety, the host window automatically hides to the system tray when a client connects. Users can interact with or shut down the application via the system tray context menu.
* **Modern Dark Theme:** Designed with a premium dark-themed interface (Slate 900 / Navy Blue), integrating FontAwesome (`as7.otf`) for crisp icon rendering, smooth micro-animations, and clean toast notifications.

---

## 📁 Project Directory Structure

```text
├── backend/
│   ├── core/                        # Core system logic modules
│   │   ├── AudioLoopbackCapture.h/cpp # System audio capture (WASAPI/parec)
│   │   ├── ClipboardManager.h/cpp   # Clipboard synchronization and management
│   │   ├── ConnectionManager.h/cpp  # State management and network connection controller
│   │   ├── NetworkManager.h/cpp     # Local IP address retrieval
│   │   ├── ScreenRenderer.h/cpp     # Renders incoming screen frames onto QML Canvas
│   │   └── SystemTray.h/cpp         # Manages system tray icons and notifications
│   └── network/                     # Network socket communication layer
│       ├── Packet.h/cpp             # Defines network packet structures
│       ├── PacketStream.h/cpp       # Handles serialization/deserialization of TCP packet streams
│       ├── Protocol.h               # Defines network packet types and compression schemas
│       ├── RemoteClient.h/cpp       # Handles client-side remote connection logic
│       └── RemoteServer.h/cpp       # Handles server-side hosting and input simulation
├── fonts/
│   └── as7.otf                      # Font containing FontAwesome icons
├── icons/
│   └── tray_icon.png                # System tray icon
├── importedcontent/                 # Directory for auto-generated UI assets (if any)
├── qml/                             # QML-based frontend user interface
│   ├── components/
│   │   └── Toast.qml                # Small popup toast notification component
│   ├── Control.qml                  # Client window for displaying and controlling the remote desktop
│   └── Home.qml                     # Main startup window to choose Host or Client mode
├── CMakeLists.txt                   # CMake build configuration
└── main.cpp                         # Application entry point
```

---

## 🛠️ Requirements & Build Guide

### 1. Requirements
* **Qt6 SDK** (Qt 6.10 or higher recommended, which can be downloaded from [Qt Official Open Source Download Page](https://www.qt.io/development/download-qt-installer-oss)). Required modules:
  * `Gui`, `Qml`, `Quick`, `QuickControls2`, `Network`, `QuickEffects`, `Multimedia`, `Widgets`.
* **C++ Compiler** supporting standard C++17 or higher.
* **CMake** version 3.16 or higher.
* **Operating Systems:**
  * **Windows:** Standard build with Win32 APIs (COM, WASAPI).
  * **Linux:** Requires X11 development headers (`libX11`, `libXtst`) and PulseAudio utility tools (`pactl` and `parec` for loopback capture).
    * Install on Ubuntu/Debian:
      ```bash
      sudo apt-get install libx11-dev libxtst-dev pulseaudio-utils
      ```

### 2. Build Steps (CMake)

Open a terminal in the root directory of the project and execute the following commands:

```bash
# Configure the build directory
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build the application
cmake --build build --config Release
```

After building:
* **Windows:** The executable file `appLR_02.exe` will be located under the `build` folder.
* **Linux:** The executable file `appLR_02` will be located under the `build` folder.

---

## 💡 How to Use

1. **Host (Allow Remote Control):**
   * Run the application and toggle the switch on the right side of the **This Computer (Host)** panel.
   * Your local IP address and a **one-time 6-digit passcode** will be generated and displayed.
   * Share the IP and Passcode with the client who wants to connect to your computer.
   * Once a connection is established, the application window will automatically hide to the system tray for safety.

2. **Client (Control a Remote PC):**
   * Run the application and focus on the **Control Remote Computer** panel.
   * Enter the **Partner IP Address** and **Partner Passcode** of the host.
   * Click **Connect to Partner**.
   * The remote desktop window will open, allowing you to use your mouse and keyboard to control the remote PC, toggle image quality between Optimized and Lossless, Mute/Unmute audio, or stop the session via the **Disconnect** button.

---

## 🛡️ License & Contributions
This project is designed for secure, low-latency remote computer control in local area networks (LAN). Feel free to submit pull requests or raise issues for enhancements and bug fixes.
