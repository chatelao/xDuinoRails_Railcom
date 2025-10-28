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

function decodeRawId(decoded6bitValues) {
  if (decoded6bitValues.length === 0) {
    return "No data to decode.";
  }

  let combinedValue = 0n;
  for (const val of decoded6bitValues) {
    combinedValue = (combinedValue << 6n) | BigInt(val);
  }

  const numBits = decoded6bitValues.length * 6;
  if (numBits < 4) {
    return "Error: Not enough bits for an ID.";
  }
  const id = Number((combinedValue >> BigInt(numBits - 4)) & 0b1111n);
  const payload = combinedValue & ((1n << BigInt(numBits - 4)) - 1n);

  const idStr = RailcomID[id] || `Unknown ID (${id})`;

  return `ID: ${idStr}\nPayload: ${payload.toString()}`;
}

const railcomMessageTypes = {
  1: { length: 2 }, // ADR_HIGH
  2: { length: 2 }, // ADR_LOW
};


function formatHex(hexString) {
    return hexString.replace(/(.{2})/g, '$1_').slice(0, -1);
}

function formatBinary(binaryString) {
    return binaryString.replace(/(.{4})/g, '$1_').slice(0, -1);
}

function decodeRawId(messageChunks) {
  if (messageChunks.length === 0) return "";
  let combinedValue = 0n;
  for (const val of messageChunks) {
    combinedValue = (combinedValue << 6n) | BigInt(val);
  }
  const numBits = messageChunks.length * 6;
  if (numBits < 4) return "Error: Not enough bits for an ID.";
  const id = Number((combinedValue >> BigInt(numBits - 4)) & 0b1111n);
  const payload = combinedValue & ((1n << BigInt(numBits - 4)) - 1n);
  const idStr = RailcomID[id] || `Unknown ID (${id})`;
  const payloadBits = numBits - 4;
  const payloadBytes = Math.ceil(payloadBits / 8);
  const payloadHex = payload.toString(16).toUpperCase().padStart(payloadBytes * 2, '0');
  return `ID: ${idStr}\nPayload: 0x${formatHex(payloadHex)}`;
}

function decodePayload(messageChunks) {
    if (messageChunks.length === 0) return "";
    let combinedValue = 0n;
    for (const val of messageChunks) {
        combinedValue = (combinedValue << 6n) | BigInt(val);
    }
    const numBits = messageChunks.length * 6;
    if (numBits < 4) return "Error: Not enough bits for an ID.";
    const id = Number((combinedValue >> BigInt(numBits - 4)) & 0b1111n);
    const payload = combinedValue & ((1n << BigInt(numBits - 4)) - 1n);
    const idStr = RailcomID[id] || `Unknown ID (${id})`;

    let interpretation = `ID: ${idStr}\n`;
    switch (id) {
        case 1: // ADR_HIGH
        case 2: // ADR_LOW
            interpretation += `Address part: ${payload.toString()}`;
            break;
        default:
            interpretation += `Payload: ${payload.toString()}`;
            break;
    }
    return interpretation;
}


document.addEventListener('DOMContentLoaded', () => {
  const input = document.getElementById('input');
  const decodeBtn = document.getElementById('decode');
  const output6bit = document.getElementById('output-6bit');
  const outputRawId = document.getElementById('output-raw-id');
  const outputPayload = document.getElementById('output-payload');

  decodeBtn.addEventListener('click', () => {
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
          const decoded = decode6of8(byte);
          const byteBin = byte.toString(2).padStart(8, '0');
          if (decoded !== undefined) {
            allDecoded6bitValues.push(decoded);
            output6bit.textContent += `0x${byteHex.toUpperCase()} (0b${formatBinary(byteBin)}) -> ${decoded.toString(10).padStart(2, '0')} (0b${formatBinary(decoded.toString(2).padStart(6, '0'))})\n`;
          } else {
            output6bit.textContent += `0x${byteHex.toUpperCase()} (0b${formatBinary(byteBin)}) -> Error: Invalid byte\n`;
          }
        }
      }
    }

    const messages = [];
    let buffer = [...allDecoded6bitValues];
    while (buffer.length >= 2) {
        const firstChunk = buffer[0];
        const id = firstChunk >> 2;
        const messageLength = railcomMessageTypes[id]?.length || 4;

        if (buffer.length < messageLength) break;

        const messageChunks = buffer.splice(0, messageLength);
        messages.push(messageChunks);
    }

    outputRawId.textContent = messages.map(decodeRawId).join('\n\n---\n\n');
    outputPayload.textContent = messages.map(decodePayload).join('\n\n---\n\n');
    if (buffer.length > 0) {
        const leftover = buffer.map(b => `0b${b.toString(2).padStart(6,'0')}`).join(' ');
        outputPayload.textContent += `\n\nWarning: ${buffer.length} leftover 6-bit chunk(s): ${leftover}`;
    }
  });
});
