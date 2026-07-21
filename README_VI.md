<p align="center">
  <img src="icons/tray_icon.png" alt="Remote Desktop Logo" width="80" />
</p>

<h1 align="center">Remote Desktop Connection</h1>

<p align="center">
  <strong>Ứng dụng điều khiển máy tính từ xa đa nền tảng, hiệu suất cao — hỗ trợ chia sẻ màn hình, truyền âm thanh và điều khiển chuột/bàn phím theo thời gian thực qua mạng LAN.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white" alt="C++17" />
  <img src="https://img.shields.io/badge/Qt-6.10+-41CD52?logo=qt&logoColor=white" alt="Qt6" />
  <img src="https://img.shields.io/badge/CMake-3.16+-064F8C?logo=cmake&logoColor=white" alt="CMake" />
  <img src="https://img.shields.io/badge/Nền_tảng-Windows%20|%20Linux-lightgrey?logo=windows&logoColor=white" alt="Nền tảng" />
</p>

<p align="center">
  <a href="README.md">en English</a> · <a href="README_VI.md">🇻🇳 Tiếng Việt</a>
</p>

---

## 📖 Tổng Quan

**Remote Desktop Connection** là một công cụ điều khiển máy tính từ xa gọn nhẹ, được xây dựng bằng **C++**, **Qt6 (QML)** và **CMake**. Ứng dụng cho phép điều khiển máy tính từ xa một cách an toàn, độ trễ thấp trong mạng nội bộ (LAN).

### Tính năng chính

| Khả năng                  | Mô tả                                                                                            |
| :------------------------- | :------------------------------------------------------------------------------------------------ |
| 🖥️ **Truyền màn hình**   | Chụp màn hình theo thời gian thực với phát hiện thay đổi khung hình, hỗ trợ 2 chế độ nén (JPEG / Zlib) |
| 🔊 **Âm thanh hệ thống** | Thu và phát âm thanh hệ thống qua WASAPI (Windows) hoặc PulseAudio (Linux)                        |
| 🖱️ **Mô phỏng đầu vào** | Chuyển tiếp toàn bộ chuột & bàn phím qua Win32 `SendInput` hoặc X11 XTest                         |
| 📋 **Đồng bộ clipboard**  | Đồng bộ clipboard văn bản hai chiều theo thời gian thực                                           |
| 🔐 **Xác thực**           | Mã PIN 6 chữ số ngẫu nhiên, tạo mới cho mỗi phiên host                                           |

---

## 📷 Ảnh Chụp Màn Hình

### Giao Diện Kết Nối
*Host chưa hoạt động — sẵn sàng bật host hoặc kết nối đến máy đối tác.*

![Giao diện kết nối](results/connect_page.png)

### Chế Độ Host Đang Hoạt Động
*Đang lắng nghe kết nối từ client, hiển thị mã PIN tự động sinh.*

![Host đang hoạt động](results/host_page.png)

### Phiên Điều Khiển Từ Xa
*Truyền màn hình trực tiếp với âm thanh, điều khiển đầu vào và chuyển đổi chất lượng.*

![Phiên điều khiển](results/control_page.png)

---

## ✨ Chi Tiết Tính Năng

### Chụp & Truyền Màn Hình

- **Phát hiện thay đổi khung hình** — So sánh nhị phân nhanh (`memcmp`) trên dữ liệu pixel thô; chỉ truyền những khung hình thay đổi, giúp giảm đáng kể băng thông.
- **Hai chế độ nén:**
  - **Tối ưu (mặc định):** JPEG chất lượng 90% — cân bằng tốt giữa độ rõ nét và băng thông.
  - **Không mất dữ liệu (Lossless):** RGB32 → Zlib (level 1, nhanh nhất) — hình ảnh chính xác từng pixel khi cần.
- **Chuyển đổi tại client** — Thay đổi chế độ nén ngay trong phiên kết nối.

### Âm Thanh Hệ Thống (Loopback)

| Nền tảng    | Công nghệ           | Chi tiết                                                                                                     |
| :---------- | :------------------- | :----------------------------------------------------------------------------------------------------------- |
| **Windows** | WASAPI Loopback      | Thu âm từ thiết bị xuất mặc định qua `AUDCLNT_STREAMFLAGS_LOOPBACK`. Tự động chuyển Float32 PCM → Int16 PCM. |
| **Linux**   | PulseAudio (`parec`) | Thu âm từ monitor sink mặc định. Dự phòng bằng `QAudioSource` với thiết bị `monitor` / `loopback`.           |

- Phát lại theo thời gian thực trên client qua `QAudioSink` với bộ đệm.
- Có nút Tắt / Bật tiếng trên giao diện điều khiển.

### Mô Phỏng Chuột & Bàn Phím

- **Ánh xạ tọa độ** — `ScreenRenderer` ánh xạ chính xác tọa độ cửa sổ QML → độ phân giải gốc của màn hình host.
- **Windows:** Win32 `SendInput` API — hỗ trợ di chuyển, nhấn trái/phải/giữa, cuộn chuột, nhấn/thả phím.
- **Linux:** X11 XTest extension — `XTestFakeMotionEvent`, `XTestFakeButtonEvent`, `XTestFakeKeyEvent`.
- **Giới hạn tần suất chuột** — Sự kiện di chuyển chuột được giới hạn bằng `QTimer` để tránh tràn mạng.

### Đồng Bộ Clipboard

- Đồng bộ văn bản hai chiều, theo thời gian thực giữa host và client.
- Phát hiện trùng lặp để tránh vòng lặp clipboard vô hạn.

### Bảo Mật & Trải Nghiệm Người Dùng

- **Mã PIN dùng một lần** — PIN 6 chữ số ngẫu nhiên, sinh mới cho mỗi phiên host.
- **Chống mở nhiều cửa sổ** — `QSharedMemory` đảm bảo chỉ một phiên bản ứng dụng chạy tại một thời điểm.
- **Thu nhỏ vào khay hệ thống** — Cửa sổ host tự động ẩn vào system tray khi client kết nối; khôi phục qua menu chuột phải trên biểu tượng tray.
- **Giao diện tối hiện đại** — Thiết kế cao cấp tông Slate 900 / Navy Blue, biểu tượng FontAwesome, hiệu ứng chuyển động mượt mà và thông báo toast.

---

## 🏗️ Kiến Trúc

```
┌──────────────────────────────────────────────────────────┐
│                    Giao diện QML (Frontend)               │
│  Home.qml (Trang chủ)  ·  Control.qml (Phiên điều khiển)│
│                     Toast.qml (Thông báo)                │
└───────────────────────────┬──────────────────────────────┘
                            │ Q_INVOKABLE / Q_PROPERTY
┌───────────────────────────▼──────────────────────────────┐
│                   Tầng Core (C++)                        │
│  ConnectionManager  ·  ScreenRenderer  ·  SystemTray     │
│  ClipboardManager   ·  AudioLoopbackCapture              │
│  NetworkManager                                          │
└───────────────────────────┬──────────────────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────┐
│                   Tầng Mạng (TCP)                        │
│  RemoteServer ←→ PacketStream ←→ RemoteClient            │
│               Packet · Protocol                          │
│  Cổng: 5000  ·  Magic: 0x52444B54 ("RDKT")              │
└──────────────────────────────────────────────────────────┘
```

### Các Loại Gói Tin (Protocol)

```
Xác thực:    AuthRequest → AuthSuccess / AuthFailed
Màn hình:    ScreenFrame (nén JPEG hoặc Zlib)
Đầu vào:     MouseMove · MousePress · MouseRelease · MouseWheel
             KeyPress · KeyRelease
Clipboard:   Clipboard (dữ liệu văn bản)
Âm thanh:    AudioFrame (mẫu PCM Int16)
Kiểm tra:    Ping ↔ Pong
Cài đặt:     SettingsUpdate (chuyển đổi chế độ nén)
```

---

## 📁 Cấu Trúc Dự Án

```text
REMOTE_PC/
├── main.cpp                             # Điểm khởi chạy ứng dụng
├── CMakeLists.txt                       # Cấu hình build CMake
│
├── backend/
│   ├── core/                            # Các module hệ thống cốt lõi
│   │   ├── ConnectionManager.h/cpp      # Máy trạng thái trung tâm & cầu nối QML
│   │   ├── ScreenRenderer.h/cpp         # Vẽ khung hình nhận được lên canvas QML
│   │   ├── AudioLoopbackCapture.h/cpp   # Thu âm thanh hệ thống (WASAPI / parec)
│   │   ├── ClipboardManager.h/cpp       # Đồng bộ clipboard hai chiều
│   │   ├── NetworkManager.h/cpp         # Phát hiện địa chỉ IP nội bộ
│   │   └── SystemTray.h/cpp             # Biểu tượng tray, thông báo & menu ngữ cảnh
│   │
│   └── network/                         # Tầng giao tiếp TCP
│       ├── Protocol.h                   # Kiểu gói tin, magic number, enum nén
│       ├── Packet.h/cpp                 # Cấu trúc dữ liệu gói tin
│       ├── PacketStream.h/cpp           # Tuần tự hóa / giải tuần tự luồng TCP
│       ├── RemoteServer.h/cpp           # Phía host: chụp, phát sóng, mô phỏng đầu vào
│       └── RemoteClient.h/cpp           # Phía client: kết nối, hiển thị, chuyển tiếp đầu vào
│
├── qml/                                 # Giao diện QML
│   ├── Home.qml                         # Trang chủ (chọn chế độ Host / Client)
│   ├── Control.qml                      # Cửa sổ phiên điều khiển từ xa
│   └── components/
│       └── Toast.qml                    # Component thông báo toast
│
├── fonts/
│   └── as7.otf                          # Font biểu tượng FontAwesome 7
│
├── icons/
│   └── tray_icon.png                    # Biểu tượng khay hệ thống
│
└── results/                             # Ảnh chụp màn hình cho tài liệu
    ├── connect_page.png
    ├── host_page.png
    └── control_page.png
```

---

## 🛠️ Yêu Cầu Hệ Thống

### Phụ thuộc

| Phụ thuộc       | Phiên bản | Ghi chú                                                                                           |
| :-------------- | :-------- | :------------------------------------------------------------------------------------------------- |
| **Qt6 SDK**     | 6.10+     | Modules: `Gui`, `Qml`, `Quick`, `QuickControls2`, `Network`, `QuickEffects`, `Multimedia`, `Widgets` |
| **Trình biên dịch C++** | C++17+ | MSVC, GCC hoặc Clang                                                                         |
| **CMake**       | 3.16+     | Hệ thống build                                                                                    |

> **Tải Qt:** [Qt Open Source Installer](https://www.qt.io/development/download-qt-installer-oss)

### Theo Nền Tảng

<details>
<summary><strong>🪟 Windows</strong></summary>

Không cần cài thêm gì. Ứng dụng sử dụng các API có sẵn của Win32:
- **COM / WASAPI** để thu âm thanh hệ thống
- **SendInput** để mô phỏng đầu vào

</details>

<details>
<summary><strong>🐧 Linux (X11)</strong></summary>

Cài đặt các gói phát triển X11 và PulseAudio:

```bash
# Ubuntu / Debian
sudo apt-get install libx11-dev libxtst-dev pulseaudio-utils

# Fedora
sudo dnf install libX11-devel libXtst-devel pulseaudio-utils

# Arch Linux
sudo pacman -S libx11 libxtst pulseaudio
```

</details>

---

## 🚀 Build & Chạy

### Build với CMake

```bash
# Cấu hình
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

### Chạy ứng dụng

```bash
# Windows
.\build\Release\appLR_02.exe

# Linux
./build/appLR_02
```

> **Mẹo:** Bạn cũng có thể mở dự án trực tiếp trong **Qt Creator** bằng cách load file `CMakeLists.txt`.

---

## 💡 Hướng Dẫn Sử Dụng

### 1. Host — Cho Phép Điều Khiển Từ Xa

1. Khởi chạy ứng dụng.
2. Trong bảng **This Computer (Host)**, bật công tắc sang **ON**.
3. **Địa chỉ IP nội bộ** và **mã PIN 6 chữ số** sẽ được hiển thị.
4. Chia sẻ thông tin này cho người cần kết nối.
5. Khi client kết nối thành công, cửa sổ tự động ẩn vào **khay hệ thống (system tray)**.
6. Nhấn chuột phải vào biểu tượng tray để khôi phục cửa sổ hoặc tắt ứng dụng.

### 2. Client — Điều Khiển Máy Tính Từ Xa

1. Khởi chạy ứng dụng.
2. Trong bảng **Control Remote Computer**, nhập **địa chỉ IP** và **mã PIN** của host.
3. Nhấn **Connect to Partner**.
4. Cửa sổ điều khiển từ xa mở ra với các chức năng:

| Chức năng              | Hành động                                                          |
| :--------------------- | :----------------------------------------------------------------- |
| **Chuột & Bàn phím**  | Tương tác bình thường — toàn bộ đầu vào được chuyển tiếp đến host |
| **Chuyển đổi chất lượng** | Chuyển giữa *Tối ưu* (JPEG) và *Không mất dữ liệu* (Zlib)     |
| **Bật/Tắt âm thanh**  | Tắt / Bật tiếng âm thanh hệ thống của host                       |
| **Ngắt kết nối**      | Kết thúc phiên điều khiển từ xa                                    |

---

## 🔧 Cấu Hình

| Tham số                | Mặc định         | Vị trí                                  |
| :--------------------- | :---------------- | :--------------------------------------- |
| Cổng TCP               | `5000`            | `Protocol.h` → `DefaultPort`            |
| Chất lượng JPEG        | `90%`             | `RemoteServer.cpp`                       |
| Mức nén Zlib           | `1` (nhanh nhất)  | `RemoteServer.cpp`                       |
| Chu kỳ chụp màn hình  | Dựa trên Timer    | `RemoteServer` → `m_captureTimer`        |

---

## 🛡️ Giấy Phép & Đóng Góp

Dự án được thiết kế để **điều khiển máy tính từ xa an toàn, độ trễ thấp qua mạng LAN**. Mọi đóng góp đều được hoan nghênh — hãy gửi pull request hoặc mở issue để cải thiện và sửa lỗi.
