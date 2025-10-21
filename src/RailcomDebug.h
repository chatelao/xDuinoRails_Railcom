#ifndef RAILCOM_DEBUG_H
#define RAILCOM_DEBUG_H

#include <Arduino.h>
#include "Railcom.h"

// Prints a human-readable description of a RailcomMessage to the given Stream.
void printRailcomMessage(const RailcomMessage* msg, Stream& stream);

#endif // RAILCOM_DEBUG_H
