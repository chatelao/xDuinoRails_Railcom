# RP2040 RailCom Library

[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/11454/badge)](https://www.bestpractices.dev/projects/11454)

An Arduino library for encoding and decoding RailCom messages on the RP2040, compliant with the RCN-217 specification.

## Architektur und Abgrenzungen

Dieses Projekt stellt eine Bibliothek zur Verfügung, die die RailCom®-Kommunikation für Digitale Modellbahnen ermöglicht. Die Bibliothek ist in zwei Hauptkomponenten unterteilt, deren Rollen klar voneinander abgegrenzt sind:

### RailcomRx (Empfänger)

-   **Anwendungsbereich:** Die `RailcomRx`-Klasse ist für den Einsatz in einer **Zentrale** oder einem dedizierten **RailCom-Detektor** vorgesehen.
-   **Aufgaben:** Ihre Hauptaufgabe ist das Empfangen und Parsen von RailCom-Nachrichten, die von Decodern auf dem Gleis gesendet werden.
-   **Abgrenzung:** `RailcomRx` ist **nicht** für die Erzeugung des DCC-Signals oder des für die RailCom-Übertragung notwendigen "Cutouts" (der stromlosen Lücke im DCC-Signal) verantwortlich. Diese Aufgaben müssen von der übergeordneten Anwendung (z.B. der Firmware der Zentrale) übernommen werden. `RailcomRx` erwartet, die zugehörige DCC-Mitteilung im Format der `NmraDcc`-Bibliothek zu erhalten, um den Kontext der empfangenen RailCom-Nachricht zu verstehen.

### RailcomTx (Sender)

-   **Anwendungsbereich:** Die `RailcomTx`-Klasse ist für den Einsatz in einem **Fahrzeug- oder Zubehördecoder** konzipiert.
-   **Aufgaben:** Ihre einzige Aufgabe ist das Vorbereiten und Versenden von RailCom-Antwortnachrichten.
-   **Abgrenzung:** `RailcomTx` ist **nicht** für die Decodierung von DCC-Signalen, die Motorsteuerung oder die Ansteuerung von Funktionen und anderer Decoderfunktionen zuständig. Die Anwendungslogik des Decoders muss `RailcomTx` die letzte empfangene DCC-Mitteilung sowie den exakten Startzeitpunkt des Cutouts mitteilen. Daraufhin versendet die Bibliothek die vorbereiteten RailCom-Nachrichten.

```
DCC Zentrale (Mikrocontroller)        Decoder (Mikrocontroller)
+----------------------+                 +--------------------+
|                      |                 |                    |
|    DCC_TX_PIN ------->----------------->------ DCC_RX_PIN   |
|         (DCC Signal) |                 |    (DCC Signal)    |
|                      |                 |                    |
| RC_CUTOUT_PIN ------->-------          |                    |
| RC_RX_PIN ===========<=================<====== RC_TX_PIN    |
|    (  RailCom Signal)|                 | (RailCom Signal)   |
|                      |                 |                    |
+----------------------+                 +--------------------+
```

## Features

-   **High-Level API:** `RailcomTx` (für Decoder) und `RailcomRx` (für Detektoren/Zentralen) Klassen vereinfachen das Erstellen, Senden und Parsen von RCN-217 Nachrichten.
-   **Asynchrones Senden:** Verwendet eine nachrichten-warteschlange, um Nachrichten vorzubereiten und sie zum exakt richtigen Zeitpunkt (während des Cutouts) zu senden.
-   **Decoder State Machine:** Beinhaltet eine `DecoderStateMachine`-Klasse, um eine realitätsnahe Antwortlogik für Decoder zu demonstrieren.
-   **Umfassende Beispiele:** Beinhaltet `Dummy`- und `NmraDcc`-basierte Beispiele für verschiedene Decoder-Typen.
-   **Web-basiertes Werkzeug:** Ein [web-basiertes Werkzeug](https://chatelao.github.io/xDuinoRails_Railcom/index.html) zum Dekodieren von RailCom-Nachrichten.

## Real-World Use Cases

RailCom's bidirectional communication opens up many possibilities for realistic and automated layout control. Here are some of the most common applications:

1.  **Automatic Locomotive Recognition:** A command station can instantly identify a locomotive when you place it on the track, eliminating the need to manually enter its address.
2.  **On-the-Main Programming:** Read and write decoder CVs on the main track without needing a separate programming track, making it much faster to fine-tune performance.
3.  **Accessory Feedback:** Turnout decoders can report their actual position back, ensuring your control panel is always synchronized with the physical layout.

For a more comprehensive list of what you can do with RailCom, see the **[full Use Cases documentation](docs/USE_CASES.md)**.

## Neopixel Example

The `LocomotiveDecoderNeopixel` example requires the [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) library to be installed. You can install it through the Arduino Library Manager.

## Installation
1.  In the Arduino IDE, go to `Sketch > Include Library > Manage Libraries...`
2.  Search for "RP2040 Railcom" and install.
3.  Also install the required dependencies: `AUnit` and `NmraDcc`.

## Getting Started

To get started, please see the detailed "howto" guides for wiring and minimal software examples:

-   **[Howto: RailCom Decoder (Transmitter)](docs/HOWTO_TX.md)**
-   **[Howto: RailCom Detector (Receiver)](docs/HOWTO_RX.md)**

## API Reference
For a detailed API reference, please see the **[API Documentation](docs/API_REFERENCE.md)**.

## Testing
For a detailed overview of the testing strategy, please see the **[Testing Documentation](docs/TESTING.md)**.

## Tools

### Railcom Bit Decoder

A simple web-based tool for decoding RailCom messages. You can paste hexadecimal or binary values and see the decoded 6-bit values and the full payload.

**[Try it here!](https://chatelao.github.io/xDuinoRails_Railcom/tools/index.html)**
