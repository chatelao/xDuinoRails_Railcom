/**
 * @file CommandStationRx.cpp
 * @brief Implements the high-level CommandStationRx class.
 */
#include "CommandStationRx.h"

CommandStationRx::CommandStationRx(RailcomRx* rx) :
    _rx(rx),
    _last_known_address(0),
    _pending_adr_high(0) {
}

void CommandStationRx::begin() {
    // Initialization, if any, would go here.
    // For now, we mainly rely on the underlying RailcomRx being initialized.
}

void CommandStationRx::task() {
    RailcomMessage* msg = _rx->read();
    if (msg != nullptr) {
        processMessage(msg);
    }
}

void CommandStationRx::onLocoIdentified(LocoIdentifiedCallback callback) {
    _locoIdentifiedCallback = callback;
}

void CommandStationRx::onPomResponse(PomResponseCallback callback) {
    _pomResponseCallback = callback;
}

void CommandStationRx::onAccessoryStatus(AccessoryStatusCallback callback) {
    _accessoryStatusCallback = callback;
}

void CommandStationRx::processMessage(RailcomMessage* msg) {
    switch (msg->id) {
        case RailcomID::ADR_LOW: {
            uint16_t address_low = static_cast<AdrMessage*>(msg)->address;
            // Only form a full address if we have a pending high part.
            if (_pending_adr_high != 0) {
                 uint16_t full_address = (_pending_adr_high << 8) | address_low;
                _last_known_address = full_address;
                if (_locoIdentifiedCallback) {
                    _locoIdentifiedCallback(full_address);
                }
            }
            _pending_adr_high = 0; // Reset pending high part
            break;
        }

        case RailcomID::ADR_HIGH: {
            // A new address sequence is starting. Store the high part and invalidate
            // the last known address to prevent misattribution of subsequent messages.
            _pending_adr_high = static_cast<AdrMessage*>(msg)->address;
            _last_known_address = 0;
            break;
        }

        case RailcomID::POM: {
            // A POM response is only meaningful if it follows an address message.
            if (_pomResponseCallback && _last_known_address != 0) {
                uint8_t cvValue = static_cast<PomMessage*>(msg)->cvValue;
                _pomResponseCallback(_last_known_address, cvValue);
            }
            // Invalidate the address after use to ensure the next message can't reuse it.
            _last_known_address = 0;
            break;
        }

        case RailcomID::STAT1:
        case RailcomID::STAT2:
        case RailcomID::STAT4: {
             // An accessory status is only meaningful if it follows an address message.
             if (_accessoryStatusCallback && _last_known_address != 0) {
                uint8_t status = 0;
                if (msg->id == RailcomID::STAT1) status = static_cast<Stat1Message*>(msg)->status;
                if (msg->id == RailcomID::STAT2) status = static_cast<Stat2Message*>(msg)->status;
                if (msg->id == RailcomID::STAT4) status = static_cast<Stat4Message*>(msg)->status;

                _accessoryStatusCallback(_last_known_address, status);
            }
            // Invalidate the address after use.
            _last_known_address = 0;
            break;
        }

        default:
            // For any other message, invalidate the last known address to be safe.
            _last_known_address = 0;
            break;
    }
}
