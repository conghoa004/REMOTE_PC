# Remote Desktop Connection (LR_02)

Một ứng dụng điều khiển máy tính từ xa (Remote Desktop) đa nền tảng, hiệu năng cao và có giao diện hiện đại, được phát triển bằng ngôn ngữ **C++**, khung làm việc **Qt6 (QML)** và công nghệ **CMake**.

Ứng dụng hỗ trợ truyền hình ảnh màn hình, âm thanh hệ thống (audio loopback), đồng bộ hóa Clipboard (khay nhớ tạm) và mô phỏng các thao tác chuột, bàn phím trên cả hai hệ điều hành **Windows** và **Linux (X11)**.

---

## 🌟 Các Tính Năng Nổi Bật

### 1. Truyền và Kết nối Màn hình
* **Độ trễ thấp:** Tự động phát hiện thay đổi khung hình (frame diff) bằng cách so sánh nhị phân nhanh (`memcmp`) dữ liệu điểm ảnh để chỉ gửi đi những khung hình có sự thay đổi, giảm thiểu băng thông mạng và tải CPU.
* **Chất lượng hình ảnh linh hoạt:**
  * **Optimized (Mặc định):** Nén khung hình dưới dạng JPEG với chất lượng 90%, cân bằng hoàn hảo giữa chất lượng hiển thị và băng thông mạng.
  * **Lossless (Không hao hao):** Chuyển đổi sang định dạng RGB32 và nén bằng giải thuật Zlib (mức nén 1 - nhanh nhất), hiển thị sắc nét đến từng điểm ảnh.

### 2. Truyền Âm thanh Hệ thống (Audio Loopback)
* **Windows (WASAPI):** Sử dụng API WASAPI Loopback (`AUDCLNT_STREAMFLAGS_LOOPBACK`) để thu âm trực tiếp âm thanh phát ra từ thiết bị đầu ra mặc định. Tự động chuyển đổi định dạng âm thanh từ Float 32-bit PCM sang Int 16-bit PCM giúp tiết kiệm tối đa băng thông và đảm bảo khả năng tương thích khi phát.
* **Linux (PulseAudio / parec):** Thu âm thanh từ thiết bị giám sát mặc định thông qua công cụ `parec` hoặc tự động tìm kiếm các thiết bị thu âm dạng `monitor` / `loopback` bằng `QAudioSource`.
* **Phát âm thanh thời gian thực:** Đồng bộ âm thanh trực tiếp sang máy khách (client) bằng `QAudioSink` với hỗ trợ tắt tiếng (Mute/Unmute).

### 3. Mô phỏng Chuột và Bàn phím
* **Hỗ trợ tọa độ chính xác:** Máy khách sử dụng component `ScreenRenderer` để tự động ánh xạ tọa độ hiển thị cục bộ sang độ phân giải màn hình gốc của máy chủ điều khiển (Host).
* **Mô phỏng sự kiện trên Windows:** Sử dụng API Win32 `SendInput` để giả lập chuột (di chuyển, nhấn giữ, nhả chuột, cuộn bánh xe) và bàn phím (nhấn giữ, nhả phím).
* **Mô phỏng sự kiện trên Linux:** Sử dụng thư viện hệ thống X11 và phần mở rộng **XTest** (`XTestFakeMotionEvent`, `XTestFakeButtonEvent`, `XTestFakeKeyEvent`).

### 4. Đồng bộ khay nhớ tạm (Clipboard Sync)
* Đồng bộ hóa hai chiều (2-way synchronization) văn bản trong Clipboard giữa máy chủ (Host) và máy khách (Client) trong suốt thời gian kết nối.

### 5. Cơ chế bảo mật và Trải nghiệm người dùng
* **Mã PIN Kết nối:** Tạo mã PIN ngẫu nhiên dùng một lần (One-time Password) trên Host để xác thực Client kết nối.
* **Ngăn chặn chạy nhiều phiên bản (Single Instance protection):** Sử dụng `QSharedMemory` đảm bảo chỉ có duy nhất một cửa sổ ứng dụng hoạt động tại một thời điểm trên máy.
* **Ẩn về khay hệ thống (System Tray Icon):** Khi có Client kết nối thành công, cửa sổ chính của Host sẽ ẩn đi vì lý do bảo mật. Người dùng có thể hiển thị lại giao diện hoặc tắt ứng dụng thông qua menu chuột phải ở System Tray.
* **Giao diện hiện đại (Dark Mode):** Được thiết kế theo phong cách tối giản, sang trọng (Slate 900 / Navy Blue), tích hợp FontAwesome (`as7.otf`) để vẽ icon sắc nét, các hiệu ứng chuyển động mượt mà (smooth animations) và Toast thông báo tinh tế.

---

## 📁 Cấu trúc Thư mục Dự án

```text
├── backend/
│   ├── core/                        # Các module xử lý logic lõi
│   │   ├── AudioLoopbackCapture.h/cpp # Thu âm thanh hệ thống (WASAPI/parec)
│   │   ├── ClipboardManager.h/cpp   # Đồng bộ và thao tác Clipboard
│   │   ├── ConnectionManager.h/cpp  # Quản lý trạng thái và luồng dữ liệu kết nối
│   │   ├── NetworkManager.h/cpp     # Lấy thông tin IP mạng nội bộ
│   │   ├── ScreenRenderer.h/cpp     # Vẽ màn hình nhận được từ client lên QML
│   │   └── SystemTray.h/cpp         # Quản lý khay hệ thống và thông báo hệ thống
│   └── network/                     # Tầng kết nối mạng Socket
│       ├── Packet.h/cpp             # Định nghĩa cấu trúc gói tin
│       ├── PacketStream.h/cpp       # Tuần tự hóa / giải tuần tự gói dữ liệu TCP
│       ├── Protocol.h               # Định nghĩa các loại gói tin và chuẩn nén
│       ├── RemoteClient.h/cpp       # Xử lý kết nối phía máy khách (Client)
│       └── RemoteServer.h/cpp       # Xử lý máy chủ và mô phỏng thiết bị (Host)
├── fonts/
│   └── as7.otf                      # Font chứa các icon biểu tượng (FontAwesome)
├── icons/
│   └── tray_icon.png                # Icon hiển thị dưới khay hệ thống (System Tray)
├── importedcontent/                 # Thư mục chứa tài nguyên UI tự động tạo (nếu có)
├── qml/                             # Giao diện người dùng viết bằng QML
│   ├── components/
│   │   └── Toast.qml                # Component hiển thị thông báo popup nhỏ
│   ├── Control.qml                  # Cửa sổ hiển thị và tương tác màn hình từ xa (Client)
│   └── Home.qml                     # Cửa sổ chính lựa chọn chế độ Host / Client
├── CMakeLists.txt                   # Cấu hình biên dịch bằng CMake
└── main.cpp                         # Điểm khởi đầu của ứng dụng
```

---

## 🛠️ Yêu cầu Hệ thống & Biên dịch

### 1. Yêu cầu môi trường
* **Qt6 SDK** (Yêu cầu Qt 6.10 trở lên). Cần các thành phần:
  * `Gui`, `Qml`, `Quick`, `QuickControls2`, `Network`, `QuickEffects`, `Multimedia`, `Widgets`.
* **C++ Compiler** hỗ trợ chuẩn C++17 trở lên.
* **CMake** bản 3.16 trở lên.
* **Hệ điều hành:**
  * **Windows:** Yêu cầu các thư viện hệ thống sẵn có (COM, WASAPI).
  * **Linux:** Yêu cầu cài đặt thư viện phát triển X11 (`libX11`, `libXtst`) và máy chủ âm thanh PulseAudio (cần công cụ `pactl` và `parec` để thu âm thanh).
    * Lệnh cài đặt trên Ubuntu/Debian:
      ```bash
      sudo apt-get install libx11-dev libxtst-dev pulseaudio-utils
      ```

### 2. Các bước Biên dịch (CMake)

Mở Terminal tại thư mục gốc của dự án và chạy các lệnh sau:

```bash
# Tạo thư mục build và cấu hình dự án
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Biên dịch ứng dụng
cmake --build build --config Release
```

Sau khi biên dịch xong:
* Trên Windows: Bạn sẽ nhận được file thực thi `appLR_02.exe` trong thư mục `build`.
* Trên Linux: Bạn sẽ nhận được file thực thi `appLR_02` trong thư mục `build`.

---

## 💡 Cách Sử dụng

1. **Cho phép điều khiển máy này (Host):**
   * Mở ứng dụng, tại mục **This Computer (Host)**, bật công tắc (Switch) ở góc trên bên phải.
   * Địa chỉ IP mạng nội bộ của bạn và **Mật khẩu dùng một lần (One-time Password)** gồm 6 chữ số sẽ hiển thị.
   * Gửi thông tin IP và Mật khẩu này cho người kết nối.
   * Khi người khác kết nối thành công, giao diện ứng dụng sẽ tự động ẩn xuống Khay hệ thống (System Tray).

2. **Điều khiển máy khác (Client):**
   * Mở ứng dụng, tại mục **Control Remote Computer**, điền chính xác **Partner IP Address** và **Partner Password** của máy chủ.
   * Nhấn nút **Connect to Partner**.
   * Cửa sổ hiển thị màn hình từ xa sẽ hiện lên. Bạn có thể sử dụng chuột/bàn phím để điều khiển trực tiếp, điều chỉnh chế độ chất lượng hình ảnh (Optimized / Lossless), Bật/Tắt âm thanh (Mute/Unmute) hoặc kết thúc kết nối bằng nút **Disconnect**.

---

## 🛡️ Bản quyền & Đóng góp
Dự án được xây dựng phục vụ cho mục đích kết nối và điều khiển máy tính cá nhân từ xa an toàn trong mạng nội bộ. Mọi đóng góp chỉnh sửa hoặc báo lỗi vui lòng tạo Pull Request hoặc Issue trên kho lưu trữ.
