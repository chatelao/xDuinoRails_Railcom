#include "Railcom.h"
#include <cstring>
#include <vector>

// --- DCCMessage Implementation ---
DCCMessage::DCCMessage() : _len(0) {}
DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len > sizeof(_data) ? sizeof(_data) : len) {
    memcpy(_data, data, _len);
}
const uint8_t* DCCMessage::getData() const { return _data; }
size_t DCCMessage::getLength() const { return _len; }
