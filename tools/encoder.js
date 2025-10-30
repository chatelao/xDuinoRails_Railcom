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

const RailcomID = {
  0: "POM", 1: "ADR_HIGH", 2: "ADR_LOW", 3: "EXT/STAT4",
  4: "INFO/STAT1", 5: "TIME", 6: "ERROR", 7: "DYN",
  8: "XPOM_0/STAT2", 9: "XPOM_1", 10: "XPOM_2", 11: "XPOM_3",
  12: "CV_AUTO", 13: "DECODER_STATE", 14: "RERAIL", 15: "DECODER_UNIQUE"
};

const messagePayloads = {
    0: [{ name: "CV", bits: 12 }, { name: "Value", bits: 8 }],
    1: [{ name: "Address part", bits: 8 }],
    2: [{ name: "Address part", bits: 8 }],
    3: [{ name: "Port 1", bits: 2 }, { name: "Port 2", bits: 2 }, { name: "Port 3", bits: 2 }, { name: "Port 4", bits: 2 }],
    4: [{ name: "Status", bits: 8 }],
    5: [{ name: "Time", bits: 16, type: 'datepicker' }],
    6: [{ name: "Error code", bits: 8 }],
    7: [{ name: "Value", bits: 8, type: 'slider' }, { name: "Subindex", bits: 6 }],
    8: [{ name: "Sequence", bits: 2 }, { name: "CV", bits: 16 }, { name: "Value", bits: 8 }],
    9: [{ name: "Sequence", bits: 2 }, { name: "CV", bits: 16 }, { name: "Value", bits: 8 }],
    10: [{ name: "Sequence", bits: 2 }, { name: "CV", bits: 16 }, { name: "Value", bits: 8 }],
    11: [{ name: "Sequence", bits: 2 }, { name: "CV", bits: 16 }, { name: "Value", bits: 8 }],
    12: [{ name: "CV", bits: 12 }, { name: "Value", bits: 8 }],
    13: [{ name: "State", bits: 16 }],
    14: [{ name: "Rerail counter", bits: 8 }],
    15: [{ name: "Unique ID part", bits: 32 }],
};


document.addEventListener('DOMContentLoaded', () => {
  const form = document.getElementById('encoder-form');
  const messageIdSelect = document.getElementById('message-id');
  const payloadFieldsDiv = document.getElementById('payload-fields');
  const rawPayloadInput = document.getElementById('raw-payload');
  const outputEncoded = document.getElementById('output-encoded');

  for (const id in RailcomID) {
    const option = document.createElement('option');
    option.value = id;
    option.textContent = `${RailcomID[id]} (${id})`;
    messageIdSelect.appendChild(option);
  }

  function updatePayloadFields() {
    payloadFieldsDiv.innerHTML = '';
    const messageId = messageIdSelect.value;
    const fields = messagePayloads[messageId];

    if (fields) {
      fields.forEach(field => {
        const label = document.createElement('label');
        label.textContent = `${field.name} (bits: ${field.bits}):`;
        let input;

        switch (field.type) {
            case 'slider':
                input = document.createElement('input');
                input.type = 'range';
                input.min = 0;
                input.max = (1 << field.bits) - 1;
                break;
            case 'datepicker':
                input = document.createElement('input');
                input.type = 'date';
                break;
            default:
                input = document.createElement('input');
                input.type = 'number';
                input.min = 0;
                input.max = (1 << field.bits) - 1;
        }

        input.name = field.name;
        payloadFieldsDiv.appendChild(label);
        payloadFieldsDiv.appendChild(input);
      });
    }
    encode();
  }

  function encode() {
    const messageId = parseInt(messageIdSelect.value);
    let payload = 0n;

    if (rawPayloadInput.value) {
        payload = BigInt('0x' + rawPayloadInput.value);
    } else {
        const fields = messagePayloads[messageId];
        if (fields) {
            let currentOffset = 0;
            for (let i = fields.length - 1; i >= 0; i--) {
                const field = fields[i];
                const input = payloadFieldsDiv.querySelector(`[name="${field.name}"]`);
                if (input) {
                    let value;
                    switch (field.type) {
                        case 'datepicker':
                            if (input.value) {
                                const date = new Date(input.value);
                                value = BigInt(date.getHours() * 60 + date.getMinutes());
                            } else {
                                value = 0n;
                            }
                            break;
                        default:
                            value = BigInt(input.value || 0);
                    }
                    payload |= value << BigInt(currentOffset);
                }
                currentOffset += field.bits;
            }
        }
    }

    const totalBits = 4 + (messagePayloads[messageId] ? messagePayloads[messageId].reduce((acc, field) => acc + field.bits, 0) : 0);
    const combinedValue = (BigInt(messageId) << BigInt(totalBits - 4)) | payload;

    let bitString = combinedValue.toString(2).padStart(totalBits, '0');

    let sixBitChunks = [];
    for (let i = 0; i < bitString.length; i += 6) {
        sixBitChunks.push(parseInt(bitString.substring(i, i + 6), 2));
    }

    const encodedBytes = sixBitChunks.map(chunk => ENCODE_TABLE[chunk]);

    outputEncoded.textContent = '0x' + encodedBytes.map(byte => byte.toString(16).toUpperCase().padStart(2, '0')).join('');
  }


  messageIdSelect.addEventListener('change', updatePayloadFields);
  form.addEventListener('input', encode);

  const urlParams = new URLSearchParams(window.location.search);
  const messageIdFromUrl = urlParams.get('id');
  const payloadFromUrl = urlParams.get('payload');

  if (messageIdFromUrl) {
      messageIdSelect.value = messageIdFromUrl;
  }

  updatePayloadFields();

  if (payloadFromUrl) {
      rawPayloadInput.value = payloadFromUrl;
      const payload = BigInt('0x' + payloadFromUrl);
      const fields = messagePayloads[messageIdSelect.value];
      if (fields) {
          let currentOffset = 0;
          for (let i = fields.length - 1; i >= 0; i--) {
              const field = fields[i];
              const mask = (1n << BigInt(field.bits)) - 1n;
              const value = (payload >> BigInt(currentOffset)) & mask;
              const input = payloadFieldsDiv.querySelector(`[name="${field.name}"]`);
              if (input) {
                  input.value = value;
              }
              currentOffset += field.bits;
          }
      }
      encode();
  }
});
