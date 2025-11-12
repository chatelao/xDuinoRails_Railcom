/**
 * @file DecoderStateMachine.cpp
 * @brief Implementation of the DecoderStateMachine class.
 */
#include "DecoderStateMachine.h"

#include <Arduino.h>

/**
 * @brief Constructs the DecoderStateMachine.
 * @details Initializes member variables, sets up dummy data for testing purposes
 *          (CVs and Data Spaces), and registers the callbacks for the DCC parser.
 * @param txManager Reference to the RailcomTx object.
 * @param type The type of the decoder.
 * @param address The primary address.
 * @param cv28 Configuration Variable 28.
 * @param cv29 Configuration Variable 29.
 * @param manufacturerId Manufacturer ID for RCN-218.
 * @param productId Product ID for RCN-218.
 */
DecoderStateMachine::DecoderStateMachine(RailcomTx& txManager, DecoderType type, uint16_t address, uint8_t cv28, uint8_t cv29, uint16_t manufacturerId, uint32_t productId)
    : _txManager(txManager), _type(type), _address(address), _cv28(cv28), _cv29(cv29),
      _manufacturerId(manufacturerId), _productId(productId), _logonState(LogonState::IDLE), _accessory_state(0), _channel1_broadcast_enabled(true),
      _backoff_counter(0), _backoff_value(1), _cv_auto_broadcast_active(false) {

    // Populate the dummy CV list for testing
    _cvs[1] = 10;
    _cvs[8] = 155;
    _cvs[29] = 34;
    _cv_auto_iterator = _cvs.begin();

    // --- RCN-218 Data Space Initialization ---
    // Use Case #7: Displaying Locomotive's Long Name (Data Space 5)
    _data_spaces[5] = { 'D', 'B', ' ', 'C', 'l', 'a', 's', 's', ' ', '2', '1', '8' };

    // Use Case #8: Automatic Function Icon Mapping (Data Space 4)
    // Format: [F-Num], [Icon-Num], [F-Num], [Icon-Num], ...
    _data_spaces[4] = { 1, 10, 2, 20 }; // F1=Horn, F2=Bell

    // Use Case #21: Displaying Manufacturer Info (Data Space 6)
    // Format: [Manuf. ID High], [Manuf. ID Low], [Article Num ASCII...]
    _data_spaces[6] = {
        (uint8_t)((_manufacturerId >> 8) & 0xFF),
        (uint8_t)(_manufacturerId & 0xFF),
        '1', '2', '3', '4', '5'
    };

    setupCallbacks();
}

/**
 * @brief Processes an incoming DCC packet.
 * @details First, it checks CV29 to ensure RailCom is enabled. Then, it passes
 *          the message to the internal DCC parser, which triggers the appropriate
 *          callbacks. It also manages the Channel 1 address broadcast, disabling
 *          it once the decoder is directly addressed to reduce network congestion.
 * @param msg The DCCMessage to process.
 */
void DecoderStateMachine::handleDccPacket(const DCCMessage& msg) {
    // According to NMRA S-9.2.2, CV29, Bit 3 enables/disables RailCom.
    // If RailCom is not enabled, do not process any packets.
    if ((_cv29 & 0b00001000) == 0) {
        return;
    }

    bool response_sent = false;
    _dccParser.parse(msg, &response_sent);

    // Also send address as a default response for locomotives
    const uint8_t* data = msg.getData();
    size_t len = msg.getLength();
    bool is_addressed_to_me = (len >= 2 && ((data[0] << 8) | data[1]) == _address);

    if (is_addressed_to_me) {
        // Once we are addressed directly, we should stop broadcasting on Ch1
        // to reduce channel congestion.
        _channel1_broadcast_enabled = false;
    }

    if (!response_sent && _type == DecoderType::LOCOMOTIVE && _channel1_broadcast_enabled) {
        _txManager.sendAddress(_address);
    }
}

/**
 * @brief Handles periodic background tasks.
 * @details This function is responsible for the automatic CV broadcast feature.
 *          When active, it iterates through a map of CVs and sends one
 *          CV_AUTO message per call.
 */
void DecoderStateMachine::task() {
    if (!_cv_auto_broadcast_active) {
        return;
    }

    // If the iterator is at the end, reset it to the beginning.
    if (_cv_auto_iterator == _cvs.end()) {
        _cv_auto_iterator = _cvs.begin();
    }

    // Send the CV-Auto message for the current CV.
    _txManager.sendCvAuto(_cv_auto_iterator->first, _cv_auto_iterator->second);

    // Move to the next CV for the next call.
    _cv_auto_iterator++;
}

/**
 * @brief Sets up all the lambda function callbacks for the RailcomDccParser.
 * @details This is where the core logic of the decoder is defined. Each callback
 *          corresponds to a specific DCC command and defines what RailCom message(s)
 *          should be sent in response.
 */
void DecoderStateMachine::setupCallbacks() {
    /**
     * @brief Callback for RCN-218 Logon Enable command.
     * @details Initiates the logon process and sends the decoder's unique ID.
     *          Implements a simple collision avoidance backoff mechanism.
     * @see RCN-218, Chapter 3
     */
    _dccParser.onLogonEnable = [this](uint8_t group, uint16_t zid, uint8_t sessionId) {
        if (_logonState == LogonState::IDLE) {
            _logonState = LogonState::WAITING_FOR_LOGON;
        }

        if (_logonState == LogonState::WAITING_FOR_LOGON) {
            if (_backoff_counter > 0) {
                _backoff_counter--;
                return;
            }
            _txManager.sendDecoderUnique(_manufacturerId, _productId);
            _logonState = LogonState::IN_SINGULATION;
            // Progressively increase backoff time to avoid collisions
            _backoff_counter = _backoff_value;
            _backoff_value++;
        }
    };

    /**
     * @brief Callback for RCN-218 Select command.
     * @details Responds to a selection command during the logon process.
     * @see RCN-218, Chapter 3
     */
    _dccParser.onSelect = [this](uint16_t manufacturerId, uint32_t productId, uint8_t subCmd, const uint8_t* data, size_t len) {
        if (manufacturerId == _manufacturerId && productId == _productId) {
            if (_logonState == LogonState::IN_SINGULATION) {
                _logonState = LogonState::ANNOUNCED;
                // Handle sub-commands like ReadShortInfo, ReadBlock, etc.
                _txManager.sendAck();
            }
        }
    };

    /**
     * @brief Callback for RCN-218 Logon Assign command.
     * @details Finalizes the logon process by accepting the new address and sending the decoder state.
     * @see RCN-218, Chapter 3
     */
    _dccParser.onLogonAssign = [this](uint16_t manufacturerId, uint32_t productId, uint16_t address) {
        if (manufacturerId == _manufacturerId && productId == _productId) {
            if (_logonState == LogonState::ANNOUNCED) {
                _address = address;
                _logonState = LogonState::REGISTERED;
                _backoff_counter = 0;
                _backoff_value = 1;
                _txManager.sendDecoderState(0, 0, 0); // Dummy values
            }
        }
    };

    // --- RCN-217 Callbacks ---

    /**
     * @brief Handles a POM Read CV command.
     * @details Sends a POM response with the (dummy) value of the requested CV.
     * @see RCN-217, 5.1.1 & 5.2.4
     */
    _dccParser.onPomReadCv = [this](uint16_t cv, uint16_t address) {
        // RCN-217, 5.2.4: Handle decoder registration via programming address 0.
        if (address == 0 && cv == 29) {
            // Only respond if bit 4 of CV28 is set.
            if ((_cv28 & 0b00010000) != 0) {
                // The response is the content of CV29.
                _txManager.sendPomResponse(_cv29);
            }
            return; // Explicitly return to avoid processing as a standard POM.
        }

        if (address == _address) {
            uint8_t value = 42; // Dummy value for the requested CV
            _txManager.sendPomResponse(value);
        }
    };

    /**
     * @brief Handles a POM Write CV command.
     * @details Sends a POM response echoing the value that was written.
     * @see RCN-217, 5.1.2
     */
    _dccParser.onPomWriteCv = [this](uint16_t cv, uint8_t value, uint16_t address) {
        if (address == _address) {
            // Here you would typically write the CV value to memory
            _txManager.sendPomResponse(value);
        }
    };

    /**
     * @brief Handles a POM Write Bit command.
     * @details Sends a POM response with the new, modified value of the CV.
     * @see RCN-217, 5.1.3
     */
    _dccParser.onPomWriteBit = [this](uint16_t cv, uint8_t bit, uint8_t value, uint16_t address) {
        if (address == _address) {
            // Here you would typically write the CV bit to memory
            uint8_t currentValue = 42; // Dummy value
            if (value) {
                currentValue |= (1 << bit);
            } else {
                currentValue &= ~(1 << bit);
            }
            _txManager.sendPomResponse(currentValue);
        }
    };

    /**
     * @brief Handles an accessory decoder command.
     * @details Updates the internal state of the accessory and sends the appropriate
     *          status message (STAT1 or STAT4) based on the decoder type.
     * @see RCN-217, 6.3 & 6.4
     */
    _dccParser.onAccessory = [this](uint16_t address, bool activate, uint8_t output) {
        if (address == _address) {
            if (activate) {
                _accessory_state |= (1 << output);
            } else {
                _accessory_state &= ~(1 << output);
            }

            if (_type == DecoderType::ACCESSORY_EXTENDED) {
                _txManager.sendStatus1(_accessory_state);
            } else {
                _txManager.sendStatus4(_accessory_state);
            }
        }
    };

    /**
     * @brief Handles a standard function command.
     * @details Sends a simple ACK in response.
     * @see RCN-212, 2.3.5
     */
    _dccParser.onFunction = [this](uint16_t address, uint8_t function, bool state) {
         if (address == _address) {
            // Here you would typically handle the function command
            _txManager.sendAck();
        }
    };

    /**
     * @brief Handles an extended function (XF) command.
     * @details Implements logic for Rerailing Search (XF2), Request for Location (XF1),
     *          and toggling the CV-Auto broadcast (XF3).
     * @see RCN-217, 4.3.1, 5.2.3, 5.3.1, 5.7
     */
    _dccParser.onExtendedFunction = [this](uint16_t address, uint8_t command) {
        // XF2 (Rerailing Search) is a broadcast command sent to address 0.
        // See RCN-217 Section 5.2.3
        if (command == 0x02 && address == 0) {
            // Respond with address and time since power-on.
            // In a real application, `millis()` would be replaced with a timer
            // that is reset on power-on.
            _txManager.handleRerailingSearch(_address, millis() / 1000);
            return; // Explicitly return to avoid falling through to addressed commands
        }

        if (address == _address) {
            // XF1 is the command for "request for location information"
            // See RCN-217 Section 5.3.1
            if (command == 0x01) {
                // Send a dummy EXT message with type 0 and position 0.
                // In a real application, these values would come from sensors.
                _txManager.sendExt(0, 0);
            }
            // XF3 toggles the CV-Auto broadcast.
            // See RCN-217 Section 5.7
            else if (command == 0x03) {
                _cv_auto_broadcast_active = !_cv_auto_broadcast_active;
                // Reset iterator to the beginning when starting the broadcast
                if (_cv_auto_broadcast_active) {
                    _cv_auto_iterator = _cvs.begin();
                }
            }
        }
    };

    /**
     * @brief Handles a Data Space Read command.
     * @details Retrieves the requested data from the internal data space map
     *          and sends it in a Data Space response message.
     * @see RCN-218, 4.3
     */
    _dccParser.onDataSpaceRead = [this](uint16_t address, uint8_t dataSpaceNum, uint8_t startAddr) {
        if (address == _address) {
            // Check if the requested data space exists in our map.
            if (_data_spaces.count(dataSpaceNum)) {
                const auto& full_data = _data_spaces.at(dataSpaceNum);

                // Ensure the start address is within the bounds of the data.
                if (startAddr < full_data.size()) {
                    // Create a sub-vector starting from the startAddr.
                    // The spec implies the central knows the max length, so we send the rest.
                    std::vector<uint8_t> partial_data(full_data.begin() + startAddr, full_data.end());
                    _txManager.sendDataSpace(partial_data.data(), partial_data.size(), dataSpaceNum);
                }
            }
        }
    };
}
