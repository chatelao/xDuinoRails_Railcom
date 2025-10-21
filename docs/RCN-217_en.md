# RCN-217 RailCom – DCC feedback protocol

## Contents
- RCN-217
- 3.2 RailCom command type STAT ...................................................................................... 12
- 4.2.1 CV28 RailCom configuration.................................................................. 14
- 5.1.2 Byte Writing ............................................................................................. 18
- standard
- 4.3 DCC commands ......................................................................................................... 15
- Model Railway Products eV
- 3.1 RailCom command type MOB ....................................................................................... 10
- 5.1.3
- 1.1 Purpose of the standard................................................................................................ 3
- 5 Applications (Application/APP) Layer for Vehicle Decoders.......................................... 18
- 4.2 CVs............................................................................................................................. 14
- 1 General ............................................................................................................................ 3
- Additional function commands................................................................ 15
- 5.1 POM (ID 0)................................................................................................................. 18
- 3 Packet Layer.................................................................................................................... 9
- Bit Writing............................................................................................................ 19
- 4.1 System requirements ............................................................................................. 13
- 4.2.4 RailCom Block................................................................................................. 14
- 5.2 ADR (IDs 1 & 2) ........................................................................................... 19
- 2.1 General................................................................................................................ 4
- 4.3.1
- 2.5 Data backup ............................................................................................................ 8
- 5.1.1 Byte Reading ................................................................................................... 18
- 4.2.3 CV31, CV32 ........................................................................................... 14
- 2.4 Timing.................................................................................................................... 6
- the manufacturer of digital
- 4 RailCom CVs and DCC commands ...................................................................................... 13
- 1.3 Explanations............................................................................................................ 3
- 2 Physical Layer.................................................................................................................... 4
- 4.3.2
- CV29................................................................................................................ 14
- output
- DCC feedback protocol
- 4.2.2
- RailCommunity – Association
- 2.3 The RailCom Detector ........................................................................................... 6
- July 27, 2025
- Advanced programming commands ................................................................ 16
- 1.2 Requirements............................................................................................................ 3
- 2.2 RailCom – transmitter in the decoder................................................................................. 5
- 4.3.3 NOP for Accessory Decoders............................................................................ 17
- RailCom
- Association of Manufacturers of Digital Model Railway Products Page 1 of 41
- Machine Translated by Google
- RCN-217 RailCom – DCC feedback protocol July 27, 2025
- Association of Manufacturers of Digital Model Railway Products Page 2 of 41
- Info1 (ID 3).................................................................................................. 21
- 6 Application for accessory decoders (switches, etc.)............................................................. 29
- 5.2.2
- 6.6 ERROR (ID 6) ....................................................................................................... 34
- 6.8.2 Byte Writing ............................................................................................. 38
- 6.8.3 Bit Writing............................................................................................................ 38
- 5.6.1 XPOM Byte Reading...................................................................................... 28
- Filling............................................................................................................ 23
- 5.3.1 Sending location information........................................................................ 22
- 6.5 TIME (ID 5)............................................................................................................. 33
- 5.7 CV car (ID 12)................................................................................................................ 28
- Appendix A: References to other standards............................................................................ 39
- 6.8.1 XPOM Byte Read...................................................................................... 38
- 5.2.3 Rerailing search (ID 1, 2 and 14).................................................................. 21
- 5.3.2
- 5.4 Current driving information (ID 4) ............................................................................. 23
- Bit Writing............................................................................................................ 28
- A.2 Informative references ............................................................................................. 40
- 6.2.3
- 6.2.1 Byte Reading ................................................................................................... 31
- 6.3 STAT1 (ID 4) ........................................................................................................... 32
- 6.9 STAT2 (ID 8) ........................................................................................................... 39
- 5.2.4 Decoder registration via programming address 0000
- 6.2 POM (ID 0).............................................................................................................. 30
- .............................
- 6.8 XPOM (ID 8 to ID 11)................................................................................................. 37
- A.1 Normative references ................................................................................................ 39
- 5.6.3
- 6.4 STAT4 (ID 3) ........................................................................................................... 33
- 22
- 5.6.2 Byte Writing ............................................................................................. 28
- 5.2.1 Dynamic Channel 1 Usage...................................................................... 20
- Bit Writing............................................................................................................ 31
- 6.2.2 Byte Writing ............................................................................................. 31
- 5.3 EXT (ID 3)............................................................................................................... 22
- Appendix B: History ................................................................................................................ 40
- 5.5 DYN (ID 7)................................................................................................................. 24
- 5.6 XPOM (ID 8 to ID 11)................................................................................................. 26
- 6.7 DYN (ID 7)................................................................................................................. 35
- 6.1 SRQ - Service Request for Accessory Decoders ........................................................... 29
- Machine Translated by Google

## 1 General

### 1.1 Purpose of the standard
This standard describes the transmission of data from the decoder over the track to a receiver, i.e., in the opposite direction to the control protocols. The transmission method described here, together with the protocol used, is called RailCom.

### 1.2 Requirements
To comply with this standard, all technical values and protocols defined in this standard must be observed. This specification applies exclusively to the use of RailCom within the DCC data format (protocol). The use of RailCom within other data formats is not permitted. Tables 5 to 7 define which messages a decoder must support at a minimum.

### 1.3 Explanations
"RailCom" is a German trademark registered in the name of Lenz Elektronik for Class 9 "Electronic Controls" under number 301 16 303, and a trademark registered in the USA for Classes 21, 23, 26, 36, and 38 "Electronic Controls for Model Railways" under Reg. No. 2,746,080. European Patent 1 380 326 B1 has been revoked. RailCom is therefore freely usable, subject to the trademarks.

• The following symbols are used to indicate the meaning of a bit:
  - 0 Bit value 0
  - 1 bit value 1
  - A address bit
  - D data bit
  - G speed
  - L Load
  - P Location information (position)
  - R direction bit
  - S sequence number
  - T Type of location information
  - X Subindex

• Bytes are groups of eight bits each.
• Each bit in the byte has a value that depends on its position; the first bit sent, the leftmost bit in the diagram, has the highest value and is called the MSB (most significant bit). The bits of a byte are numbered from left to right, starting with 7 and descending to 0. The last bit sent, the rightmost bit in the diagram, is called the LSB (least significant bit).
• A DCC data packet is a defined sequence of bits that is used as a track signal in [RCN-210] are described.
• “XF#” refers to the binary state control instructions according to [RCN-212] Section 2.3.5.
• The characters used in the DCC commands are not listed again here.
• (ÿ) indicates the RailCom data sent from the decoder to the central unit.
• The commands from the command station to the decoder (ÿ) are written without the addressing data. Addressing is done according to the DCC standard.
• The bit combinations for DCC shown in the boxes in this standard are purely informative and are not normative. Only the specified RCNs apply.
• Unless otherwise specified, values always refer to an 8-bit field. Binary values are marked according to the list above. Hexadecimal values are marked with a leading 0x.

## 2 Physical Layer

### 2.1 General
This chapter describes the physical layer of RailCom.
The flow of information in the DCC system normally occurs from the central unit (booster) via the track to the decoders. For the reverse transmission direction, this power and data flow must be interrupted. This is achieved by the boosters, which create a so-called RailCom cutout at the end of each DCC packet by disconnecting and short-circuiting the two track lines from the power supply. This functional group within the booster is called a "cutout device." Such a cutout device could also be implemented as a separate unit outside the booster.
The actual data transmission takes place via a current loop. The decoder must provide the necessary current from its internal buffer. Figure 1 shows the arrangement of the booster, detector, and decoder during the RailCom cutout.
The voltage drop across the cutout device must not exceed 10mV at a maximum of 34mA during cutout.

### 2.2 RailCom – transmitter in the decoder
To transmit a '0', the decoder must supply a current I of 30 +4/-6 mA, with a voltage drop on the track of up to 2.2 V. If high-current RailCom is enabled in CV28, the current is 60 +8/-12 mA, also with a voltage drop on the track of up to 2.2 V. For a '1', the current I may not exceed ± 0.1 mA. The current source The decoder's power supply must be protected against unexpected external voltage on the track during cutout. Figure 2 shows a possible hardware implementation.
It is recommended that the power supply be connected so that the positive pole is on the right rail, forward in the direction of travel. This allows the vehicle's direction on the track to be detected. However, there are situations where this recommendation cannot be followed. Therefore, ID 3 has been added to channel 1 (section 5.2.2).
Due to its principle, direction detection only works in a two-rail, two-rail system. In a three-rail, two-rail system ("Märklin") or a three-rail, three-rail system ("Trix-Express"), the rerailing direction cannot be detected via the polarity of the RailCom or track signal. Therefore, neither the recommendation here nor ID 3 in channel 1 can be used.

### 2.3 The RailCom Detector
A detector must interpret a current greater than 10 mA during the middle 50% of the bit time as '0', and a current less than 6 mA during the middle 50% of the bit time as '1'.
The voltage drop across the detector must not exceed 200 mV at a maximum current of 34 mA during cutout.
A maximum of two detectors (including the global detector) may be used in series.
The local detector should include a connection for external evaluation of track occupancy. If this is not the case, externally used occupancy detectors must be specified for RailCom.

### 2.4 Timing
The RailCom cutout is divided into two channels. Channel 1 can transmit 2 bytes, and channel 2 can transmit up to 6 bytes. Figure 4 shows the timing diagram. All times are referenced to the zero crossing of the last edge of the packet end bit.
Up to 8 bytes of data can be transmitted in a cutout. Each transmitted byte begins with a start bit ('0') followed by the 8 data bits (least significant bit first) and ends with a stop bit ('1'). The transmission rate is 250 kbit/s ±2%. The rise time (10% ÿ 90%) and fall time (90% ÿ 10%) must not exceed 0.5 µs.

### 2.5 Data backup
Data transmission is secured using 4 out of 8 coding, meaning each transmitted byte contains 4 '1' and 4 '0' bits (Hamming weight 4). If this ratio is violated, a transmission error occurs.
There are 70 different bit combinations within a byte, each with a Hamming weight of 4. Of these, 64 are used to transmit six payload bits; of the remaining six, three are used for short special messages: two ACKs and two NACKs. The remaining three combinations are currently unused.
A net 12 bit payload can be transmitted in channel 1 and up to 36 bit payload in channel 2.

## 3 Packet Layer
This chapter describes the structure of RailCom packages.
RailCom packets (hereinafter referred to as datagrams) have a length of 6, 12, 18, 24, or 36 payload bits. Only a 12-bit datagram can be transmitted on channel 1. Any combination of datagrams with a maximum total length of 36 bits can be transmitted on channel 2.
Datagrams (except ACK and NACK) begin, unless otherwise stated, with a 4-bit Identifier, followed by 8, 14, 20 or 32 bit payload.
The length of the datagram is determined by the identifier (see chapters 3.1 and 3.2).
Optionally, the data channel can be filled to 36 bits with ACK.
Mobile decoders (vehicle decoders) and stationary decoders (accessory decoders) have different feedback requirements. Accordingly, the channels are used differently for both decoder types. The meaning of the datagrams therefore depends on the address of the preceding DCC packet. Accordingly, the following RailCom command types, MOB (mobile) and STAT (stationary), are differentiated based on the DCC address ranges as defined in [RCN-211] Section 3.
For automatic decoder registration, the two data channels can be combined (channel bundling). The timing of the two channels remains unchanged, but only one data block with a total of 48 bits – including an identifier depending on the context – is transmitted. The structure of this data block is defined in [RCN-218].

### 3.1 RailCom command type MOB
Channel 1 is used by mobile vehicle decoders for rapid localization on the layout (see app:adr). To do this, they must transmit their DCC address after each DCC packet addressed to a vehicle decoder (labeled MOB in Table 4), which is then received by local detectors on the layout. Programming mode packets are excluded from this rule from the moment the decoder detects programming mode. Transmission after each DCC packet addressed to a vehicle decoder (short address 1..127, long address 1..10239) is required to prevent parts of different addresses from being mistakenly combined into an incorrect, nonexistent address when multiple transmitters are in the section.
Decoders may not send feedback to the idle packet or service mode packets. Only the decoder addressed by the DCC packet may send on channel 2. To enable feedback on channel 1 as long as no DCC locomotive address is addressed, it is recommended to address DCC locomotive address 10239 alternately with the idle packet.
Channel 2 may only be used by the addressed decoder and is used to transmit decoder information. An addressed decoder must always send an acknowledgement (ACK if necessary) on channel 2 to confirm error-free reception of the DCC packet.
A feedback signal on channel 2 indicates that the decoder has received the command without errors, but not that the decoder has accepted and executed the command.
The following identifiers (datagrams) are defined for vehicle decoders, whereby all IDs not listed are to be considered reserved.

### 3.2 RailCom command type STAT
Accessory decoders use channel 1 to report service requests (see app:srq). They can transmit their identity (12-bit address) (12-bit value without identifier) after each DCC packet addressed to an accessory decoder (labeled STAT in Table 4) (not when addressing via decoder ID). If multiple decoders report simultaneously, a search must be initiated.
Channel 2 may only be used by the addressed decoder and is used to transmit decoder information. An addressed decoder must always send an acknowledgement (ACK if necessary) on channel 2 to confirm error-free reception of the DCC packet.
A feedback signal on channel 2 indicates that the decoder has received the command without errors, but not that the decoder has accepted and executed the command.
The following identifiers (datagrams) in channel 2 are defined for accessory decoders, whereby all IDs not listed are to be considered reserved.

## 4 RailCom CVs and DCC commands

### 4.1 System requirements
This RailCom specification is designed to be backward compatible, meaning that decoders that are not RailCom capable can continue to be operated and command stations that are not RailCom capable can continue to control decoders that are RailCom capable.
The decoder must respond to each vehicle or turnout address in channel 2 of the blanking interval (see applications). The decoder must not respond if it has not been addressed or if the command was sent to broadcast address 0.

### 4.2 CVs
All CVs presented here are for information purposes only. The specifications in [RCN-225] are binding.

#### 4.2.1 CV28 RailCom configuration
Bit function:
- 0: Channel 1 for address broadcast (section 5.2) -> 1=enabled, 0=disabled
- 1: Channel 2 for data and acknowledge -> 1=enabled, 0=disabled
- 2: Channel 1 automatically switch off (section 5.2.1) -> 1=enabled, 0=disabled
- 3: Transmit ID 3 in channel 1 (section 5.2.2) -> 1=enabled, 0=disabled
- 6: Release of high-current RailCom (Section 2.2) -> 1=enabled, 0=disabled
- 7: Release automatic login (Section 5.2.4) -> 1=enabled, 0=disabled

#### 4.2.2 CV29
Use according to [RCN-225].

#### 4.2.3 CV31, CV32
Use as pointer according to [RCN-225].

#### 4.2.4 RailCom Block
CV31 = 0 and CV32 = 255 address a block of 256 CVs that are used for RailCom Applications.

### 4.3 DCC commands
The expanded functionality provided by RailCom also requires additional DCC commands. These include:

#### 4.3.1 Additional function commands
RailCom provides additional functionality such as search functions, rerail search, etc. (see also section "Applications (Application/APP) Layer for Vehicle decoder"). For this purpose, the short form binary state control commands according to [RCN-212] section 2.3.5 are used.

#### 4.3.2 Advanced programming commands
The [RCN-214] also defines the configuration variable access command - short form for programming CVs in Section 3.

#### 4.3.3 NOP for accessory decoders
The NOP command is defined in [RCN-213] Section 2.5. This command is sent regularly but initially does nothing except allow accessory decoders to send an SRQ. A RailCom-capable command station must send a NOP command at specific intervals to query the accessory decoders. An interval of approximately 0.5 seconds is recommended.

## 5 Applications (Application/APP) Layer for Vehicle Decoders
The following sections describe the commands for the RailCom functionality for vehicle decoders with the address ranges marked as MOB in Table 4.

### 5.1 POM (ID 0)
POM stands for "Programming On the Main," meaning programming on the track for operation. It is used to read and write configuration variables in operating mode according to [RCN-214] Section 2. These commands are responded to on channel 2 with a 12-bit datagram with ID 0 = 0000 and 8 data bits. The data bits contain the value of the CV.

### 5.2 ADR (IDs 1 & 2)
Vehicle decoders use channel 1 as a broadcast channel for their own address. They transmit their active address (basic, extended, multiple unit, or possibly additional) in the cutout after each DCC packet addressed to a vehicle decoder.

### 5.3 EXT (ID 3)
This feedback is used to transmit location information. This allows the location of the vehicle decoder at a given address to be determined and, if necessary, to trigger a refill of supplies.

### 5.4 Current travel information (ID 4)
This feedback is used to transmit dynamic information from vehicle decoders. Dynamic Information refers to Dynamic Variables (DVs).

### 5.5 DYN (ID 7)
Dynamic variables (DV) (e.g., speed, container, etc.) are transmitted in an 18-bit datagram (ID 7), which contains the 8-bit DV value (D) and a 6-bit subindex (X) that selects one of 64 possible DVs.

### 5.6 XPOM (ID 8 to ID 11)
XPOM is an extended format compared to POM for writing and reading up to four CVs in operating mode according to [RCN-214] Section 4.

### 5.7 CV-Car (ID 12)
This feedback is used to transmit CVs from vehicle decoders in the background.

## 6 Application for accessory decoders (switches etc.)

### 6.1 SRQ - Service Request for Accessory Decoders
Channel 1 of the cutout is used by accessory decoders to request communication from the central unit. This request is referred to below as an SRQ (Service Request).

### 6.2 POM (ID 0)
Programming on the main for accessory decoders.

### 6.3 STAT1 (ID 4)
This feedback is used to transmit status messages from accessory decoders.

### 6.4 STAT4 (ID 3)
This feedback is used to transmit the status of all four output pairs of simple Accessory decoders.

### 6.5 TIME (ID 5)
This feedback is used to transmit the forecasted orbital time of the decoder.

### 6.6 ERROR (ID 6)
This feedback is used to transmit error information.

### 6.7 DYN (ID 7)
This feedback is used to transmit dynamic information from accessory decoders.

### 6.8 XPOM (ID 8 to ID 11)
Extended programming on the main for accessory decoders.

### 6.9 STAT2 (ID 8)
This feedback is used to transmit status messages from accessory decoders Part 2 Specially tailored for mechanical control operations.
