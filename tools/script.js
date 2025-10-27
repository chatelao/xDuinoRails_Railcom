const ENCODE_TABLE = [
  0x0F, 0x17, 0x1B, 0x1D, 0x1E, 0x27, 0x2B, 0x2D, 0x2E, 0x33, 0x35, 0x36,
  0x39, 0x3A, 0x3C, 0x47, 0x4B, 0x4D, 0x4E, 0x53, 0x55, 0x56, 0x59, 0x5A,
  0x5C, 0x63, 0x65, 0x66, 0x69, 0x6A, 0x6C, 0x71, 0x72, 0x74, 0x78, 0x87,
  0x8B, 0x8D, 0x8E, 0x93, 0x95, 0x96, 0x99, 0x9A, 0x9C, 0xA3, 0xA5, 0xA6,
  0xA9, 0xAA, 0xAC, 0xB1, 0xB2, 0xB4, 0xB8, 0xC3, 0xC5, 0xC6, 0xC9, 0xCA,
  0xCC, 0xD1, 0xD2, 0xD4, 0xD8, 0xE1, 0xE2, 0xE4, 0xE8, 0xF0
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

function decode4of8(byte) {
  return DECODE_TABLE.get(byte);
}

function decodePayload(decoded6bitValues) {
  if (decoded6bitValues.length === 0) {
    return "No data to decode.";
  }

  let combinedValue = 0n;
  for (const val of decoded6bitValues) {
    combinedValue = (combinedValue << 6n) | BigInt(val);
  }

  const numBits = decoded6bitValues.length * 6;
  const id = Number((combinedValue >> BigInt(numBits - 4)) & 0b1111n);
  const payload = combinedValue & ((1n << BigInt(numBits - 4)) - 1n);

  const idStr = RailcomID[id] || `Unknown ID (${id})`;

  return `ID: ${idStr}\nPayload: ${payload.toString()}`;
}

document.addEventListener('DOMContentLoaded', () => {
  const input = document.getElementById('input');
  const decodeBtn = document.getElementById('decode');
  const output6bit = document.getElementById('output-6bit');
  const outputPayload = document.getElementById('output-payload');

  decodeBtn.addEventListener('click', () => {
    const lines = input.value.split('\n').filter(line => line.trim() !== '');
    const decoded6bitValues = [];
    output6bit.textContent = '';
    outputPayload.textContent = '';

    for (const line of lines) {
      let byte;
      if (line.startsWith('0x')) {
        byte = parseInt(line.slice(2), 16);
      } else if (line.startsWith('0b')) {
        byte = parseInt(line.slice(2), 2);
      } else {
        byte = parseInt(line, 16);
      }

      if (!isNaN(byte)) {
        const decoded = decode4of8(byte);
        const byteHex = byte.toString(16).toUpperCase().padStart(2, '0');
        const byteBin = byte.toString(2).padStart(8, '0');
        if (decoded !== undefined) {
          decoded6bitValues.push(decoded);
          output6bit.textContent += `0x${byteHex} (0b${byteBin}) -> ${decoded.toString(10).padStart(2, '0')} (0b${decoded.toString(2).padStart(6, '0')})\n`;
        } else {
          output6bit.textContent += `0x${byteHex} (0b${byteBin}) -> Error: Invalid 4of8 byte\n`;
        }
      }
    }

    outputPayload.textContent = decodePayload(decoded6bitValues);

  });
});
