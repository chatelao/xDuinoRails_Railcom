# RP2040 RailCom-Bibliothek

[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/11454/badge)](https://www.bestpractices.dev/projects/11454)

Eine Arduino-Bibliothek zum Kodieren und Dekodieren von RailCom-Nachrichten auf dem RP2040, konform zur RCN-217-Spezifikation.

## Architektur und Abgrenzung

Dieses Projekt stellt eine Bibliothek für die RailCom®-Kommunikation für digitale Modellbahnen zur Verfügung. Die Bibliothek ist in zwei Hauptkomponenten mit klar definierten Rollen unterteilt:

### RailcomRx (Empfänger)

-   **Anwendungsbereich:** Die `RailcomRx`-Klasse ist für den Einsatz in einer **Digitalzentrale** oder einem dedizierten **RailCom-Detektor** vorgesehen.
-   **Aufgaben:** Ihre Hauptaufgabe ist der Empfang und das Parsen von RailCom-Nachrichten, die von Decodern auf dem Gleis gesendet werden.
-   **Abgrenzung:** `RailcomRx` ist **nicht** für die Erzeugung des DCC-Signals oder des "Cutouts" (der stromlosen Lücke im DCC-Signal), die für die RailCom-Übertragung erforderlich ist, verantwortlich. Diese Aufgaben müssen von der übergeordneten Anwendung (z.B. der Firmware der Zentrale) übernommen werden. `RailcomRx` erwartet, die entsprechende DCC-Nachricht im Format der `NmraDcc`-Bibliothek zu erhalten, um den Kontext der empfangenen RailCom-Nachricht zu verstehen.

### RailcomTx (Sender)

-   **Anwendungsbereich:** Die `RailcomTx`-Klasse ist für den Einsatz in einem **Fahrzeug- oder Zubehördecoder** konzipiert.
-   **Aufgaben:** Ihre alleinige Aufgabe ist es, RailCom-Antwortnachrichten vorzubereiten und zu senden.
-   **Abgrenzung:** `RailcomTx` ist **nicht** für die Dekodierung von DCC-Signalen, die Motorsteuerung oder die Ansteuerung von Funktionen und anderen Decodermerkmalen zuständig. Die Anwendungslogik des Decoders muss `RailcomTx` die zuletzt empfangene DCC-Nachricht und den genauen Startzeitpunkt des Cutouts mitteilen. Die Bibliothek sendet dann die vorbereiteten RailCom-Nachrichten.

```
DCC-Zentrale (Mikrocontroller)        Decoder (Mikrocontroller)
+----------------------+                 +--------------------+
|                      |                 |                    |
|    DCC_TX_PIN ------->----------------->------ DCC_RX_PIN   |
|         (DCC-Signal) |                 |    (DCC-Signal)    |
|                      |                 |                    |
| RC_CUTOUT_PIN ------->-------          |                    |
| RC_RX_PIN ===========<=================<====== RC_TX_PIN    |
|    (RailCom-Signal)  |                 | (RailCom-Signal)   |
|                      |                 |                    |
+----------------------+                 +--------------------+
```

## Merkmale

-   **High-Level-API:** Die Klassen `RailcomTx` (für Decoder) und `RailcomRx` (für Detektoren/Zentralen) vereinfachen das Erstellen, Senden und Parsen von RCN-217-Nachrichten.
-   **Asynchrones Senden:** Verwendet eine Nachrichten-Warteschlange, um Nachrichten vorzubereiten und sie zum richtigen Zeitpunkt (während des Cutouts) zu senden.
-   **Decoder-Zustandsmaschine:** Enthält eine `DecoderStateMachine`-Klasse, um eine realistische Antwortlogik für Decoder zu demonstrieren.
-   **Umfassende Beispiele:** Enthält `Dummy`- und `NmraDcc`-basierte Beispiele für verschiedene Decodertypen.
-   **Web-basiertes Werkzeug:** Ein [web-basiertes Werkzeug](https://chatelao.github.io/xDuinoRails_Railcom/tools/index.html) zum Dekodieren von RailCom-Nachrichten.

## Anwendungsfälle in der Praxis

Die bidirektionale Kommunikation von RailCom eröffnet viele Möglichkeiten für eine realistische und automatisierte Anlagensteuerung. Hier sind einige der häufigsten Anwendungen:

1.  **Automatische Lok-Erkennung:** Eine Zentrale kann eine Lokomotive sofort identifizieren, wenn Sie sie auf das Gleis stellen, sodass die manuelle Eingabe ihrer Adresse entfällt.
2.  **Programmierung auf dem Hauptgleis:** Lesen und Schreiben von Decoder-CVs auf dem Hauptgleis, ohne einen separaten Programmiergleis zu benötigen, was die Feinabstimmung der Leistung erheblich beschleunigt.
3.  **Zubehör-Rückmeldung:** Weichendecoder können ihre tatsächliche Position zurückmelden, sodass Ihr Stellpult immer mit der physischen Anlage synchronisiert ist.

Eine umfassendere Liste dessen, was Sie mit RailCom tun können, finden Sie in der **[vollständigen Dokumentation der Anwendungsfälle](docs/USE_CASES.md)**.

## Neopixel-Beispiel

Das Beispiel `LocomotiveDecoderNeopixel` erfordert die Installation der [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)-Bibliothek. Sie können sie über den Arduino-Bibliotheksmanager installieren.

## Installation
1.  Gehen Sie in der Arduino-IDE zu `Sketch > Bibliothek einbinden > Bibliotheken verwalten...`
2.  Suchen Sie nach "RP2040 Railcom" und installieren Sie es.
3.  Installieren Sie auch die erforderlichen Abhängigkeiten: `AUnit` und `NmraDcc`.

## Erste Schritte

Für den Einstieg finden Sie in den detaillierten "Howto"-Anleitungen für die Verkabelung und minimale Software-Beispiele:

-   **[Howto: RailCom-Decoder (Sender)](docs/HOWTO_TX.md)**
-   **[Howto: RailCom-Detektor (Empfänger)](docs/HOWTO_RX.md)**

## API-Referenz
Eine detaillierte API-Referenz finden Sie in der **[API-Dokumentation](docs/API_REFERENCE.md)**.

## Testen
Einen detaillierten Überblick über die Teststrategie finden Sie in der **[Testdokumentation](docs/TESTING.md)**.

## Werkzeuge

### Railcom Bit-Decoder

Ein einfaches web-basiertes Werkzeug zum Dekodieren von RailCom-Nachrichten. Sie können hexadezimale oder binäre Werte einfügen und die dekodierten 6-Bit-Werte sowie die vollständige Nutzlast anzeigen lassen.

**[Probieren Sie es hier aus!](https://chatelao.github.io/xDuinoRails_Railcom/tools/index.html)**

## Mitwirken
Beiträge sind willkommen! Bitte eröffnen Sie ein Issue oder senden Sie einen Pull Request im [GitHub-Repository](https://github.com/chatelao/xDuinoRails_Railcom).
