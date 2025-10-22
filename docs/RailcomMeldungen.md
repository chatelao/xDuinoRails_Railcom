# RailCom Meldungstypen und ihre Auslöser (gemäß RCN-217)

Diese Tabelle fasst die verschiedenen RailCom-Meldungen, die in der RCN-217-Spezifikation definiert sind, und die Bedingungen, unter denen sie gesendet werden, zusammen.

## Fahrzeugdekoder (MOB - Mobile)

| Meldungstyp (ID) | DCC-Befehl (Trigger) | Andere Auslöser (Trigger) | Referenz (RCN-217) |
| :--- | :--- | :--- | :--- |
| **ADR (1, 2)** | Nach jedem DCC-Paket, das an einen beliebigen Fahrzeugdekoder adressiert ist. | - | Sektion 3.1, 5.2 |
| **Info1 (3)** | - | Zyklisch zusammen mit ADR (1, 2), wenn in CV28 Bit 3 aktiviert. | Sektion 5.2.2 |
| **POM (0)** | Als Antwort auf einen DCC "POM Read" Befehl, der an die Adresse dieses Dekoders gerichtet ist. | - | Sektion 5.1 |
| **EXT (3)** | Als Antwort auf den DCC-Funktionsbefehl "XF1 off". | Wird zur Übertragung von Standortinformationen verwendet. | Sektion 5.3 |
| **DYN (7)** | - | Spontan vom Dekoder gesendet, um dynamische Zustandsänderungen zu melden (z.B. Geschwindigkeit, Füllstand). | Sektion 5.5 |
| **XPOM (8-11)** | Als Antwort auf einen DCC "XPOM Read" Befehl. | Die Sequenznummer (SS) in der Antwort entspricht der des Befehls. | Sektion 5.6 |
| **CV-Car (12)** | Als Antwort auf den DCC-Funktionsbefehl "XF3 on", um alle CVs im Hintergrund zu übertragen. | Kann auch spontan gesendet werden. | Sektion 5.7 |
| **Rerailing (1, 2, 14)** | Als Antwort auf den DCC-Funktionsbefehl "XF2 off" (an Broadcast-Adresse 0). | Nachdem der Dekoder Strom erhalten hat (z.B. nach dem Wiederaufgleisen). | Sektion 5.2.3 |

## Zubehör-/Weichendekoder (STAT - Stationary)

| Meldungstyp (ID) | DCC-Befehl (Trigger) | Andere Auslöser (Trigger) | Referenz (RCN-217) |
| :--- | :--- | :--- | :--- |
| **SRQ (keine ID)** | Nach einem beliebigen Zubehördekoder-DCC-Paket oder einem NOP-Befehl. | Wenn eine Dienstanforderung (z.B. durch manuelle Betätigung) ansteht. Wird auf Kanal 1 gesendet. | Sektion 6.1 |
| **POM (0)** | Als Antwort auf einen DCC "POM Read" Befehl, der an die Adresse dieses Dekoders gerichtet ist. | - | Sektion 6.2 |
| **STAT1 (4)** | Als Bestätigung/Antwort nach einem Schaltbefehl für erweiterte Zubehördekoder. | - | Sektion 6.3 |
| **STAT4 (3)** | Als Bestätigung/Antwort nach einem Schaltbefehl für einfache Zubehördekoder. | Meldet den Zustand von 4 Weichenpaaren. | Sektion 6.4 |
| **TIME (5)** | Als Bestätigung/Antwort nach einem Schaltbefehl. | Um die voraussichtliche Schaltdauer zu melden. | Sektion 6.5 |
| **ERROR (6)** | Nach einem beliebigen Befehl (einschließlich NOP). | Wenn ein Fehlerzustand im Dekoder vorliegt. | Sektion 6.6 |
| **DYN (7)** | - | Spontan vom Dekoder gesendet, um dynamische Zustandsänderungen zu melden. | Sektion 6.7 |
| **XPOM (8-11)** | Als Antwort auf einen DCC "XPOM Read" Befehl. | - | Sektion 6.8 |
| **STAT2 (8)** | Als Bestätigung/Antwort nach einem Schaltbefehl. | Speziell für mechanische Antriebe. | Sektion 6.9 |
