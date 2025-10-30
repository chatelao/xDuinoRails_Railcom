const ENCODE_TABLE = [
  // Official RCN-217 4-of-8 encoding table
  // 0x00 - 0x0F
  0xAC, 0xAA, 0xA9, 0xA5, 0xA3, 0xA6, 0x9C, 0x9A, 0x99, 0x95, 0x93, 0x96, 0x8E, 0x8D, 0x8B, 0xB1,
  // 0x10 - 0x1F
  0xB2, 0xB4, 0xB8, 0x74, 0x72, 0x6C, 0x6A, 0x69, 0x65, 0x63, 0x66, 0x5C, 0x5A, 0x59, 0x55, 0x53,
  // 0x20 - 0x2F
  0x56, 0x4E, 0x4D, 0x4B, 0x47, 0x71, 0xE8, 0xE4, 0xE2, 0xD1, 0xC9, 0xC5, 0xD8, 0xD4, 0xD2, 0xCA,
  // 0x30 - 0x3F
  0xC6, 0xCC, 0x78, 0x17, 0x1B, 0x1D, 0x1E, 0x2E, 0x36, 0x3A, 0x27, 0x2B, 0x2D, 0x35, 0x39, 0x33
];

const DECODE_TABLE = new Map();
for (let i = 0; i < ENCODE_TABLE.length; i++) {
  DECODE_TABLE.set(ENCODE_TABLE[i], i);
}

const RailcomID = {
  0: "POM", 1: "ADR_HIGH", 2: "ADR_LOW", 3: "EXT/STAT4",
  4: "INFO/STAT1", 5: "TIME", 6: "ERROR", 7: "DYN",
  8: "XPOM_0/STAT2", 9: "XPOM_1", 10: "XPOM_2", 11: "XPOM_3",
  12: "CV_AUTO", 13: "DECODER_STATE", 14: "RERAIL", 15: "DECODER_UNIQUE"
};

function decode6of8(byte) {
  return DECODE_TABLE.get(byte);
}

const railcomMessageTypes = {
  0: { length: 4 }, // POM
  1: { length: 2 }, // ADR_HIGH
  2: { length: 2 }, // ADR_LOW
  3: { length: 2 }, // EXT/STAT4
  4: { length: 2 }, // INFO/STAT1
  5: { length: 4 }, // TIME
  6: { length: 2 }, // ERROR
  7: { length: 3 }, // DYN
  8: { length: 6 }, // XPOM_0/STAT2
  9: { length: 6 }, // XPOM_1
  10: { length: 6 }, // XPOM_2
  11: { length: 6 }, // XPOM_3
  12: { length: 4 }, // CV_AUTO
  13: { length: 4 }, // DECODER_STATE
  14: { length: 2 }, // RERAIL
  15: { length: 6 }, // DECODER_UNIQUE
};


function formatHex(hexString) {
    return hexString.replace(/(.{2})/g, '$1_').slice(0, -1);
}

function formatBinary(binaryString) {
    let result = '';
    for (let i = 0; i < binaryString.length; i += 4) {
        result += binaryString.substring(i, i + 4) + '_';
    }
    return result.slice(0, -1);
}

function decodeRawId(messageChunks) {
  if (messageChunks.length === 1 && messageChunks[0] === 'ACK') {
    return 'ACK';
  }
  if (messageChunks.length === 0) return "";
  let combinedValue = 0n;
  for (const val of messageChunks) {
    combinedValue = (combinedValue << 6n) | BigInt(val);
  }
  const numBits = messageChunks.length * 6;
  if (numBits < 4) return "Error: Not enough bits for an ID.";
  const id = Number((combinedValue >> BigInt(numBits - 4)) & 0b1111n);
  const payload = combinedValue & ((1n << BigInt(numBits - 4)) - 1n);
  const idStr = RailcomID[id] || `Unknown ID`;
  const payloadBits = numBits - 4;
  const payloadBytes = Math.ceil(payloadBits / 8);
  const payloadHex = payload.toString(16).toUpperCase().padStart(payloadBytes * 2, '0');
  return `ID: ${idStr} (${id})\nPayload: 0x${formatHex(payloadHex)}`;
}

function decodePayload(messageChunks) {
    if (messageChunks.length === 1 && messageChunks[0] === 'ACK') {
        return 'ACK';
    }
    if (messageChunks.length === 0) return "";
    let combinedValue = 0n;
    for (const val of messageChunks) {
        combinedValue = (combinedValue << 6n) | BigInt(val);
    }
    const numBits = BigInt(messageChunks.length * 6);
    if (numBits < 4) return "Error: Not enough bits for an ID.";

    const id = Number((combinedValue >> (numBits - 4n)) & 0b1111n);
    const payload = combinedValue & ((1n << (numBits - 4n)) - 1n);
    const payloadBits = Number(numBits - 4n);
    const payloadBytes = Math.ceil(payloadBits / 8);
    const payloadHex = payload.toString(16).toUpperCase().padStart(payloadBytes * 2, '0');
    const idStr = RailcomID[id] || `Unknown ID`;

    let interpretation = `ID: ${idStr} (${id})\n`;
    switch (id) {
        case 0: // POM (Programming on the Main)
            {
                const cv = Number((payload >> 8n) & 0xFFFn);
                const value = Number(payload & 0xFFn);
                interpretation += `CV: ${cv} (0x${cv.toString(16).toUpperCase()})\nValue: ${value}`;
            }
            break;
        case 1: // ADR_HIGH (Address High Byte)
        case 2: // ADR_LOW (Address Low Byte)
            {
                const part = Number(payload);
                interpretation += `Address part: ${part} (0b${part.toString(2).padStart(8, '0')})`;
            }
            break;
        case 3: // EXT/STAT4 (Extended info / Status of 4 ports)
            {
                interpretation += `Turnout Status:\n`;
                for (let i = 3; i >= 0; i--) {
                    const turnoutNum = i + 1;
                    const greenBit = (Number(payload) >> (i * 2 + 1)) & 1;
                    const redBit = (Number(payload) >> (i * 2)) & 1;
                    let status = 'Unknown';
                    if (greenBit === 1) {
                        status = 'Green (straight/right/go)';
                    } else if (redBit === 1) {
                        status = 'Red (turn/left/stop)';
                    } else {
                        status = 'Off';
                    }
                    interpretation += `  Pair ${turnoutNum}: ${status}\n`;
                }
            }
            break;
        case 4: // INFO/STAT1 (Information / Status of 1 port)
            {
                interpretation += `Status: ${payload.toString()}\n(Note: Meaning is application-specific)`;
            }
            break;
        case 5: // TIME
            {
                interpretation += `Time: ${payload.toString()} ms`;
            }
            break;
        case 6: // ERROR
            {
                interpretation += `Error code: ${payload.toString()}\n(Note: 1=DCC, 2=Motor, 3=Function, etc.)`;
            }
            break;
        case 7: // DYN (Dynamic Variable)
            {
                const dv = Number((payload >> 6n) & 0xFFn);
                const subindex = Number(payload & 0x3Fn);
                interpretation += `Value: ${dv} (0x${dv.toString(16).toUpperCase()})\nSubindex: ${subindex}`;
                switch (subindex) {
                    case 0: interpretation += " (True speed, part 1)"; break;
                    case 1: interpretation += " (True speed, part 2)"; break;
                    case 2:
                        const last = dv & 0x7F;
                        interpretation += ` (SUSI load: ${last} / Speed: ${dv >> 7} GGGGGGG)`;
                        break;
                    case 3:
                        const major = (dv >> 4) & 0x0F;
                        const minor = dv & 0x0F;
                        interpretation += ` (RailCom Version ${major}.${minor})`;
                        break;
                    case 4: interpretation += " (Change Flags from RCN-218)"; break;
                    case 5: interpretation += " (Flag Register)"; break;
                    case 6: interpretation += " (Input Register)"; break;
                    case 7: interpretation += ` (Reception statistics: ${dv} % bad packets)`; break;
                    case 8: interpretation += ` (Container 1 level: ${dv} %)`; break;
                    case 9: interpretation += ` (Container 2 level: ${dv} %)`; break;
                    case 10: interpretation += ` (Container 3 level: ${dv} %)`; break;
                    case 11: interpretation += ` (Container 4 level: ${dv} %)`; break;
                    case 12: interpretation += ` (Container 5 level: ${dv} %)`; break;
                    case 13: interpretation += ` (Container 6 level: ${dv} %)`; break;
                    case 14: interpretation += ` (Container 7 level: ${dv} %)`; break;
                    case 15: interpretation += ` (Container 8 level: ${dv} %)`; break;
                    case 16: interpretation += ` (Container 9 level: ${dv} %)`; break;
                    case 17: interpretation += ` (Container 10 level: ${dv} %)`; break;
                    case 18: interpretation += ` (Container 11 level: ${dv} %)`; break;
                    case 19: interpretation += ` (Container 12 level: ${dv} %)`; break;
                    case 20: interpretation += " (Location address)"; break;
                    case 21: interpretation += " (Warning and alarm messages)"; break;
                    case 22: interpretation += " (Odometer)"; break;
                    case 23: interpretation += " (Maintenance interval)"; break;
                    case 24: case 25: interpretation += " (Reserved)"; break;
                    case 26:
                        const temp = -50 + dv;
                        interpretation += ` (Temperature: ${temp}°C)`;
                        break;
                    case 27: interpretation += " (Direction status byte for East-West control)"; break;
                    case 46:
                        const voltage = 5 + dv * 0.1;
                        interpretation += ` (Track voltage: ${voltage.toFixed(1)} V)`;
                        break;
                    case 47:
                        const distance = dv * 4;
                        interpretation += ` (Calculated stopping distance: ${distance} m)`;
                        break;
                    case 28: case 29: case 30: case 31: case 32: case 33: case 34: case 35:
                    case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43:
                    case 44: case 45: case 48: case 49: case 50: case 51: case 52: case 53:
                    case 54: case 55: case 56: case 57: case 58: case 59: case 60: case 61:
                    case 62: case 63:
                        interpretation += " (Reserved)";
                        break;
                    default:
                        interpretation += " (Unknown)";
                        break;
                }
            }
            break;
        case 8: // XPOM_0/STAT2
        case 9: // XPOM_1
        case 10: // XPOM_2
        case 11: // XPOM_3
            {
                const seq = Number((payload >> 24n) & 0b11n);
                const cv_xp = Number((payload >> 8n) & 0xFFFFn);
                const val_xp = Number(payload & 0xFFn);
                interpretation += `Sequence: ${seq} of 3\nCV: ${cv_xp}\nValue: ${val_xp}`;
            }
            break;
        case 12: // CV_AUTO (Automatic CV Reporting)
            {
                const cv_auto = Number((payload >> 8n) & 0xFFFn);
                const val_auto = Number(payload & 0xFFn);
                interpretation += `CV: ${cv_auto}\nValue: ${val_auto}`;
            }
            break;
        case 13: // DECODER_STATE
            {
                interpretation += `State: ${payload.toString()}\n(Note: Meaning is application-specific, e.g., motor status, function state)`;
            }
            break;
        case 14: // RERAIL
            {
                interpretation += `Rerail counter: ${payload.toString()}`;
            }
            break;
        case 15: // DECODER_UNIQUE
            {
                interpretation += `Unique ID part: ${payload.toString()}`;
            }
            break;
        default:
            interpretation += `Payload: ${payload.toString()}`;
            break;
    }
    const editUrl = `encoder.html?id=${id}&payload=${payloadHex}`;
    interpretation += `\n<a href="${editUrl}" target="_blank">Edit</a>`;
    return interpretation;
}


document.addEventListener('DOMContentLoaded', () => {
  const input = document.getElementById('input');
  const output6bit = document.getElementById('output-6bit');
  const outputRawId = document.getElementById('output-raw-id');
  const outputPayload = document.getElementById('output-payload');
  const exampleSelect = document.getElementById('example-select');

  const examples = {
    "Short Address": "93 66",
    "Long Address": "8E B1 93 53",
    "POM Command": "AC A3 A6 9C",
    "STAT4 Turnout Status": "8B B4",
    "Message with ACK": "93 66 0F 93 53",
    "Long Address (Manual)": "8E B1 93 53 0F 8E B1 93 53",
    "Accessory Address": "8E A3 96 63 0F 8D B2 95 56",
  };

  for (const name in examples) {
    const option = document.createElement('option');
    option.value = examples[name];
    option.textContent = name;
    exampleSelect.appendChild(option);
  }

  exampleSelect.addEventListener('change', () => {
    input.value = exampleSelect.value;
    decodeInput();
  });

  function decodeInput() {
    const lines = input.value.split('\n').filter(line => line.trim() !== '');
    const allDecoded6bitValues = [];
    output6bit.textContent = '';
    outputPayload.textContent = '';
    outputRawId.textContent = '';

    for (const line of lines) {
      const hexString = line.replace(/0x/g, '').replace(/0b/g, '').replace(/\s/g, '');
      for (let i = 0; i < hexString.length; i += 2) {
        const byteHex = hexString.substring(i, i + 2);
        if (byteHex.length === 0) continue;
        const byte = parseInt(byteHex, 16);

        if (!isNaN(byte)) {
          const byteBin = byte.toString(2).padStart(8, '0');
          if (byte === 0x0F || byte === 0xF0) { // ACK bytes
            allDecoded6bitValues.push('ACK');
            output6bit.textContent += `0x${byteHex.toUpperCase()} (0b${formatBinary(byteBin)}) -> ACK\n`;
          } else {
            const decoded = decode6of8(byte);
            if (decoded !== undefined) {
              allDecoded6bitValues.push(decoded);
              const decodedBin = decoded.toString(2).padStart(6, '0');
              output6bit.textContent += `0x${byteHex.toUpperCase()} (0b${formatBinary(byteBin)}) -> ${decoded.toString(10).padStart(2, '0')}\n`;
              output6bit.textContent += `    (0b${formatBinary(decodedBin)})\n`;
            } else {
              output6bit.textContent += `0x${byteHex.toUpperCase()} (0b${formatBinary(byteBin)}) -> Error: Invalid byte\n`;
            }
          }
        }
      }
    }

    const messageGroups = [];
    let currentGroup = [];
    for (const value of allDecoded6bitValues) {
      if (value === 'ACK') {
        if (currentGroup.length > 0) {
          messageGroups.push(currentGroup);
          currentGroup = [];
        }
        messageGroups.push(['ACK']);
      } else {
        currentGroup.push(value);
      }
    }
    if (currentGroup.length > 0) {
      messageGroups.push(currentGroup);
    }

    const messages = [];
    let leftoverBuffer = [];
    for (const group of messageGroups) {
      if (group.length === 1 && group[0] === 'ACK') {
        messages.push(group);
        continue;
      }

      let subBuffer = group;
      while (subBuffer.length >= 2) {
        const firstChunk = subBuffer[0];
        if (typeof firstChunk !== 'number') {
          break;
        }
        const id = firstChunk >> 2;
        const messageLength = railcomMessageTypes[id]?.length;

        if (!messageLength || subBuffer.length < messageLength) {
          break;
        }

        const messageChunks = subBuffer.splice(0, messageLength);
        messages.push(messageChunks);
      }
      if (subBuffer.length > 0) {
        leftoverBuffer.push(...subBuffer);
      }
    }

    outputRawId.innerHTML = messages.map(decodeRawId).join('<hr>');
    outputPayload.innerHTML = messages.map(decodePayload).join('<hr>');
    if (leftoverBuffer.length > 0) {
      const leftover = leftoverBuffer.map(b => `0b${b.toString(2).padStart(6, '0')}`).join(' ');
      outputPayload.innerHTML += `<br><br>Warning: ${leftoverBuffer.length} leftover 6-bit chunk(s): ${leftover}`;
    }
  }

  input.addEventListener('input', decodeInput);
});
