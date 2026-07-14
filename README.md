# Remote Desktop Connection (REMOTE_PC)

A modern, cross-platform remote desktop application built with Qt6 (C++ + QML). This application enables secure remote control of computers over a network with real-time screen sharing, audio streaming, keyboard/mouse input, and clipboard synchronization.

## ✨ Features

- **🖥️ Screen Sharing**: Real-time screen capture and transmission with adjustable quality (Lossless/Optimized compression)
- **🎙️ Audio Streaming**: Capture and stream audio from the remote computer
- **⌨️ Input Control**: Full keyboard and mouse control support (including scroll wheel)
- **📋 Clipboard Sync**: Bidirectional clipboard sharing between local and remote machines
- **🔐 Secure Authentication**: Password-protected connections with automatic password generation
- **🖱️ Dual Mode**: 
  - **Host Mode**: Share your computer for others to control
  - **Client Mode**: Control a remote computer
- **🎨 Modern UI**: Clean, dark-themed interface built with Qt Quick Controls
- **🔔 System Tray**: Minimize to system tray with notifications
- **💻 Cross-Platform**: Support for Windows, Linux (with X11), and macOS

## 🏗️ Architecture

```
.
├── backend/
│   ├── core/                          # Core functionality modules
│   │   ├── NetworkManager.h/cpp       # Network utilities (IP detection)
│   │   ├── ConnectionManager.h/cpp    # Main connection orchestrator (both client/server)
│   │   ├── ScreenRenderer.h/cpp       # Qt Quick custom painter for remote frames
│   │   ├── ClipboardManager.h/cpp     # Clipboard sync handler
│   │   ├── AudioLoopbackCapture.h/cpp # Audio capture from system
│   │   └── SystemTray.h/cpp           # System tray icon management
│   │
│   └── network/                       # Network protocol & streaming
│       ├── Protocol.h                 # Packet type definitions & compression enums
│       ├── Packet.h/cpp               # Packet structure & serialization
│       ├── PacketStream.h/cpp         # Packet stream reader/writer
│       ├── RemoteServer.h/cpp         # TCP server (Host mode)
│       └── RemoteClient.h/cpp         # TCP client (Client mode)
│
├── qml/                               # Qt Quick UI layer
│   ├── Home.qml                       # Main connection interface
│   ├── Control.qml                    # Remote control viewer window
│   └── components/
│       ├── Toast.qml                  # Toast notification component
│
├── fonts/
│   └── as7.otf                        # Font Awesome icons
│
├── icons/
│   └── tray_icon.png                  # System tray icon
│
├── main.cpp                           # Application entry point
├── CMakeLists.txt                     # Build configuration
└── importedcontent/                   # Figma design imports (if any)
```

## 🔄 How It Works

### Connection Flow

```
Client                                  Server
  |                                       |
  |--- TCP Connection on Port 5000 ----->|
  |                                       |
  |<-- Authentication Challenge ---------|
  |                                       |
  |--- Password Auth Packet ------------>|
  |                                       |
  |<-- Auth Success / Screen Frame ------|
  |                                       |
  |<-- Continuous Screen Frames ---------|
  |<-- Audio Frames (optional) ---------|
  |<-- Clipboard Updates (optional) -----|
  |                                       |
  |--- Mouse/Keyboard Events ----------->|
  |--- Clipboard Updates (optional) ---->|
  |                                       |
```

### Protocol Overview

The application uses a custom binary protocol with the following packet types:

- **Authentication**: `AuthRequest`, `AuthSuccess`, `AuthFailed`
- **Screen**: `ScreenFrame` (compressed with JPEG or PNG)
- **Input**: `MouseMove`, `MousePress`, `MouseRelease`, `MouseWheel`, `KeyPress`, `KeyRelease`
- **Clipboard**: `Clipboard` (text sync)
- **Audio**: `AudioFrame` (raw audio data)
- **Control**: `Ping`, `Pong`, `SettingsUpdate`

### Key Components

#### ConnectionManager
Central hub for all connection logic. Manages:
- Starting/stopping the TCP server (host mode)
- Connecting/disconnecting from remote servers (client mode)
- Delegating network events to RemoteServer/RemoteClient
- Broadcasting frames to the QML UI
- Managing audio playback and clipboard sync

#### RemoteServer
TCP server that:
- Listens on port 5000 (configurable)
- Authenticates connecting clients with a shared password
- Captures screen frames at regular intervals (configurable FPS)
- Captures system audio
- Broadcasts frames/audio to all connected clients
- Receives and simulates input events (mouse, keyboard) from clients

#### RemoteClient
TCP client that:
- Connects to a RemoteServer
- Authenticates with a password
- Receives and decodes screen frames
- Receives and plays back audio
- Sends local input events (mouse, keyboard, clipboard) to the server

#### ScreenRenderer
Custom QQuickPaintedItem that:
- Renders received QImage frames to the QML window
- Handles coordinate mapping for mouse input (local window → remote screen)

## 🛠️ Building & Running

### Prerequisites

- **Qt 6.10+** (Gui, Qml, Quick, QuickControls2, Network, QuickEffects, Multimedia, Widgets)
- **CMake 3.16+**
- **C++17 compatible compiler** (MSVC, GCC, or Clang)
- **Platform-specific requirements**:
  - **Linux**: X11 development libraries (`libx11-dev`, `libxtst-dev`)
  - **Windows**: Windows SDK (usually bundled with Visual Studio)
  - **macOS**: Xcode Command Line Tools

### Build Steps

```bash
# Clone the repository
git clone https://github.com/conghoa004/REMOTE_PC.git
cd REMOTE_PC

# Create build directory
mkdir build
cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run
./appLR_02  # Linux/macOS
appLR_02.exe  # Windows
```

### Configuration

Edit the following in the source code to customize:

- **Default Port**: Change `DefaultPort` in `backend/network/Protocol.h` (currently 5000)
- **Password Length**: Modify `maximumLength` in `qml/Home.qml` (currently 6 digits)
- **Screen Capture FPS**: Adjust `m_captureTimer` interval in `backend/network/RemoteServer.cpp`
- **Compression Settings**: Modify compression parameters in `RemoteServer::compressFrame()`

## 📱 Usage

### Host Mode (Share Your Computer)

1. Launch the application
2. In the "This Computer (Host)" panel, toggle **ON** to enable hosting
3. Share your **IP Address** and **One-time Password** with the remote user
4. Click refresh to generate a new password
5. The app will hide to the system tray when someone connects
6. Click the tray icon to unhide and stop hosting

### Client Mode (Control a Remote Computer)

1. Launch the application
2. In the "Control Remote Computer" panel, enter the remote **IP Address** and **Password**
3. Click **"Connect to Partner"**
4. Once connected, a new window opens with the remote desktop
5. **Use your mouse and keyboard** to control the remote computer
6. Toggle **Lossless** for higher quality (requires more bandwidth)
7. Toggle **Mute** to silence remote audio

## 🔐 Security Considerations

- **Password Authentication**: All connections require a shared password
- **No Encryption**: Currently uses plain TCP (consider adding TLS/SSL in production)
- **Single Instance**: Application prevents multiple instances to avoid conflicts
- **Local Network Only**: Best used on trusted networks

⚠️ **Production Warning**: For production use, implement:
- TLS/SSL encryption for network traffic
- Enhanced password mechanisms (e.g., time-limited tokens)
- Rate limiting on authentication attempts
- User session logging and audit trails

## 📋 Platform-Specific Notes

### Linux
- Requires **X11** (X Window System)
- Set to use XIM input method to avoid IME (Vietnamese input) issues
- Mouse/keyboard input simulation via XTest extension
- Screen capture via X11 API

### Windows
- Uses Windows API for screen capture
- SendInput API for input simulation
- Built-in audio capture via DirectSound/WASAPI

### macOS
- Requires appropriate permissions for screen capture and input control
- May need to build from source due to code signing requirements

## 🐛 Known Limitations

- Performance degrades on slow/high-latency networks
- Audio may have slight sync issues depending on network conditions
- No support for multi-monitor setups in current version
- No built-in encryption (use VPN for secure connections)
- Password is 6-digit numeric only (by design)

## 🔮 Future Enhancements

- [ ] TLS/SSL encryption for secure connections
- [ ] Multi-monitor support
- [ ] File transfer capabilities
- [ ] Session recording
- [ ] User authentication with account system
- [ ] Bandwidth throttling
- [ ] Connection history and favorites
- [ ] Mobile app companion
- [ ] Better error handling and reconnection logic

## 📝 Project Structure Details

### Backend Core (C++)

**NetworkManager**: Detects and exposes the local IP address for sharing
- Refreshable to update IP when network changes
- Exposed to QML as a property

**ConnectionManager**: The main orchestrator
- Manages RemoteServer and RemoteClient instances
- Bridges C++ backend with QML frontend
- Emits signals for UI updates (frames, state changes)
- Handles password generation
- Manages audio playback

**ScreenRenderer**: Custom QQuickPaintedItem
- Receives QImage from the remote source
- Paints to QML canvas
- Maps mouse coordinates from local screen to remote screen resolution

**ClipboardManager**: Synchronizes system clipboard
- Monitors clipboard changes
- Sends updates to remote client
- Updates local clipboard from remote

**AudioLoopbackCapture**: Dedicated thread for audio capture
- Platform-specific implementations
- Emits audio frames with metadata (sample rate, channels, etc.)

**SystemTray**: System tray icon for window control
- Show/hide window
- Quit application
- Display notifications

### Backend Network (C++)

**Protocol**: Namespace containing protocol definitions
- Magic number: `0x52444B54` ("RDKT")
- Packet types enumeration
- Compression types (Lossless, HighQuality)

**Packet**: Serializable packet container
- Header with magic number, type, and size
- Payload with compression info
- Methods for packing/unpacking

**PacketStream**: Handles packet I/O over TCP
- Buffering and frame detection
- Automatic packet assembly
- Signal-based delivery

**RemoteServer**: TCP server implementation
- Accepts incoming client connections
- Authenticates via password matching
- Captures screen frames using platform APIs
- Broadcasts frames/audio to all connected clients
- Receives and simulates input events
- Manages clipboard sync

**RemoteClient**: TCP client implementation
- Connects to a RemoteServer
- Handles authentication handshake
- Receives screen frames and audio
- Sends input events (mouse, keyboard, clipboard)
- Mouse move throttling to reduce network traffic

### Frontend (QML)

**Home.qml**: Main connection interface
- Toggle between Host and Client modes
- Display local IP and password (host mode)
- Input fields for remote IP and password (client mode)
- Connection status and system status bar
- Toast notifications

**Control.qml**: Remote desktop control window
- ScreenRenderer for remote display
- Mouse/keyboard input capture via MouseArea and Keys
- Quality toggle (Lossless/Optimized)
- Mute audio toggle
- Disconnect button
- Connection status indicator

**Toast.qml**: Reusable toast notification component
- Used for error messages and status updates

## 🤝 Contributing

Feel free to submit issues and enhancement requests!

## 📄 License

This project is currently private. Please contact the repository owner for licensing information.

## 📧 Author

**conghoa004** - https://github.com/conghoa004

---

**Last Updated**: July 2026

---

## 🚀 Quick Start Checklist

- [ ] Install Qt 6.10+ and CMake 3.16+
- [ ] Install platform-specific dependencies (X11 for Linux)
- [ ] Clone the repository
- [ ] Build using CMake
- [ ] Run the application
- [ ] Test Host Mode on one machine
- [ ] Test Client Mode from another machine
- [ ] Verify mouse, keyboard, and audio work
- [ ] Check clipboard synchronization
