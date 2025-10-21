#include "Railcom.h"
#include <cstring>

DCCMessage::DCCMessage() : _len(0) {}

DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len > sizeof(_data) ? sizeof(_data) : len) {
    if (data && len > 0) {
        memcpy(_data, data, _len);
    }
}

const uint8_t* DCCMessage::getData() const {
    return _data;
}

size_t DCCMessage::getLength() const {
    return _len;
}
