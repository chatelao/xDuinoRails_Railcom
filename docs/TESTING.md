# Testing

This document outlines the testing strategy for the RP2040 RailCom Library, covering both the automated CI/CD process and in-circuit testing procedures. A robust testing methodology is crucial for ensuring the library's reliability and compliance with the RCN-217 standard.

## Automated Testing (CI/CD)

The project uses GitHub Actions for Continuous Integration and Continuous Deployment (CI/CD). The CI workflow, defined in `.github/workflows/ci.yml`, automates a series of checks on every push and pull request to ensure that new changes do not introduce regressions.

The key stages of the CI pipeline include:

1.  **Environment Setup**: The workflow begins by setting up a clean Ubuntu environment and installing the `arduino-cli`.
2.  **Platform Installation**: It installs the required `rp2040:rp2040` core from the official Earle Philhower Arduino-Pico package. This ensures that the code is compiled against the correct and consistent toolchain.
3.  **Dependency Installation**: The `AUnit` and `NmraDcc` libraries are installed to ensure that all dependencies are available for compilation.
4.  **Sketch Compilation**: The `arduino/compile-sketches` action is used to compile all example sketches and test suites. This step verifies that the code is syntactically correct and that all header files are correctly included. A failure at this stage indicates a critical issue that would prevent users from compiling the code in their own environments.

This automated process guarantees that the library remains in a buildable state at all times, providing a solid foundation for further testing.

## Unit and In-Circuit Testing

The library employs a combination of unit tests and in-circuit tests to verify its functionality. The tests are located in the `tests/` directory and are written using the AUnit framework.

### Unit Tests

Unit tests are designed to validate individual components of the library in isolation. They are structured to cover specific functionalities, such as:

*   **`DCCMessageTest`**: Verifies the correct creation and parsing of DCC messages.
*   **`EncodingTest`**: Ensures that RailCom messages are encoded correctly according to the RCN-217 specification.
*   **`TxManagerTest`**: Tests the non-blocking, ISR-driven message queue of the `RailcomTx` class.

These tests use mock objects and predefined data to simulate various scenarios and edge cases, allowing for thorough validation of the library's logic without requiring physical hardware.

### End-to-End Test

The `EndToEndTest` is a unique test suite that uses a single RP2040 to simulate both a Command Station and a Decoder. It leverages two hardware UARTs on the RP2040 to create a closed-loop system where the transmitter's output is directly fed into the receiver's input.

This setup allows for the verification of the entire communication chain, from message creation and transmission to reception and decoding. It is a powerful tool for identifying issues that may only arise during the interaction between different components of the library.

### In-Circuit Testing

While the automated tests provide a high level of confidence, in-circuit testing is essential for validating the library's performance in a real-world environment. The `InCircuitTest` sketch is designed to be run on an actual hardware setup, consisting of:

*   **A Command Station**: A device capable of sending DCC signals (e.g., a DCC++ EX command station).
*   **A Decoder**: An RP2040 running the `InCircuitTest` sketch, connected to the DCC bus.
*   **A RailCom Detector**: A device capable of receiving and displaying RailCom messages.

This setup allows for the verification of the library's timing-critical operations, such as the PIO-based cutout generation and the ISR-driven message transmission. It also provides an opportunity to test the library's interoperability with other DCC and RailCom devices.

By combining automated CI/CD checks, comprehensive unit tests, and real-world in-circuit testing, the RP2040 RailCom Library ensures a high level of quality and reliability.
