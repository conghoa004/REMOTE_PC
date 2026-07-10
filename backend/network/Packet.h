#pragma once

#include <QByteArray>
#include "Protocol.h"

struct Packet
{
    Protocol::PacketType type;
    QByteArray payload;
};