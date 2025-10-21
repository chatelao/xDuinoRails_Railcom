#include "RailcomDebug.h"

void printRailcomMessage(const RailcomMessage* msg, Stream& stream) {
    if (msg == nullptr) {
        stream.println("Received: nullptr");
        return;
    }

    stream.print("Received Message ID: ");
    stream.print((int)msg->id);
    stream.print(" - ");

    switch (msg->id) {
        case RailcomID::POM: {
            auto m = static_cast<const PomMessage*>(msg);
            stream.print("POM Response, CV Value: ");
            stream.println(m->cvValue);
            break;
        }
        case RailcomID::ADR_HIGH: {
            auto m = static_cast<const AdrMessage*>(msg);
            stream.print("Address Broadcast (High), Addr: ");
            stream.println(m->address);
            break;
        }
        case RailcomID::ADR_LOW: {
            auto m = static_cast<const AdrMessage*>(msg);
            stream.print("Address Broadcast (Low), Addr: ");
            stream.println(m->address);
            break;
        }
        case RailcomID::DYN: {
            auto m = static_cast<const DynMessage*>(msg);
            stream.print("Dynamic Data, SubIndex: ");
            stream.print(m->subIndex);
            stream.print(", Value: ");
            stream.println(m->value);
            break;
        }
        // Add other cases here as needed
        default:
            stream.println("Unknown or Unhandled Message Type");
            break;
    }
}
