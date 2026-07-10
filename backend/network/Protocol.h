#pragma once

#include <QtGlobal>

namespace Protocol
{

constexpr quint16 DefaultPort = 5000;

constexpr quint32 MagicNumber = 0x52444B54;   // "RDKT"

enum class PacketType : quint8
{
    Invalid = 0,

    // ===== Authentication =====
    AuthRequest,
    AuthSuccess,
    AuthFailed,

    // ===== Screen =====
    ScreenFrame,

    // ===== Mouse =====
    MouseMove,
    MousePress,
    MouseRelease,
    MouseWheel,

    // ===== Keyboard =====
    KeyPress,
    KeyRelease,

    // ===== Clipboard =====
    Clipboard,

    // ===== Audio =====
    AudioFrame,

    // ===== Ping =====
    Ping,
    Pong,

    // ===== Settings =====
    SettingsUpdate
};

enum class CompressionType : quint8
{
    Lossless = 0,
    HighQuality = 1
};

}