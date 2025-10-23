#!/bin/bash
SKETCHES=(
    "examples/AccessoryDecoderDummy"
    "examples/AccessoryDecoderNmra"
    "examples/CommandStation"
    "examples/FunctionDecoderDummy"
    "examples/LocomotiveDecoderDummy"
    "examples/LocomotiveDecoderNmra"
    "tests/DCCMessageTest"
    "tests/EncodingTest"
    "tests/InCircuitTest"
    "tests/InCircuitSelfTest"
    "tests/TxManagerTest"
    "tests/EndToEndTest"
)

for sketch in "${SKETCHES[@]}"; do
    echo "Compiling $sketch"
    ./bin/arduino-cli compile --fqbn rp2040:rp2040:rpipico --libraries="." "$sketch" src/RailcomLib/*.cpp
    if [ $? -ne 0 ]; then
        echo "Failed to compile $sketch"
        exit 1
    fi
done
