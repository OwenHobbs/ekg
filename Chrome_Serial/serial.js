let currentEKG = 0;
function processEKG(adcValue) {
    const ekgValue = ((3.3 / 1024) * adcValue).toFixed(2);
    console.log("ADC: " + adcValue + ", EKG: " + ekgValue + "V");

    document.getElementById("ekg-value").innerText = ekgValue;

    currentEKG = ekgValue;
}

async function updatePeriod(newPeriod, updateDesiredFreq = true) {
    newPeriod = newPeriod & 0xffff;
    const newFreq = (1.0 / ((newPeriod + 1) * 4e-6)).toFixed(3);
    document.getElementById("actual-freq").innerText = newFreq;
    if (updateDesiredFreq)
         document.getElementById("desired-freq").value = newFreq;

    const newPeriodLSB = newPeriod & 0xff;
    const newPeriodMSB = (newPeriod >> 8) & 0xff;
    console.log("Updating period to " + newPeriod);
    
    const data = new Uint8Array([0b01010101, newPeriodLSB, newPeriodMSB]);
    await writer.write(data);
}

async function toggleLCD() {
    const data = new Uint8Array([0b00001100]);
    await writer.write(data);
}

async function updateSamplingFreq(newSamplingFreq) {
    const newPeriod = Math.round((1.0 / (4e-6 * newSamplingFreq)) - 1);
    document.getElementById("period").value = newPeriod;
    updatePeriod(newPeriod, false);
} 

let port, writer, reader;
async function openSerialPort() {
    console.log("Open Port button pressed");

    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });

    reader = port.readable
        .pipeThrough(new TransformStream(new ByteLengthTransformer(2)))
        .getReader();
    writer = port.writable.getWriter();

    // Listen to data coming from the serial device.
    closeRequested = false;
    while (!closeRequested) {
        const { value, done } = await reader.read();

        // TODO: add sync byte?
        // Cast value to 10-bit adc value
        // value[0] is LSB, value[1] is most significant 2 bits
        const adcValue = (value[1] << 8) | value[0];

        processEKG(adcValue);
    }

    await reader.cancel();
    await writer.close();
    await port.close();
}

let closeRequested = false;
function closeSerialPort() {
    console.log("Close Serial button pressed");
    closeRequested = true;
}

class ByteLengthTransformer {
    constructor(desiredLength) {
        this.desiredLength = desiredLength;
        this.buffer = [];
    }

    transform(chunk, controller) {
        // Chunk is type Uint8Array
        for (const byte of chunk) {
            // Add bytes one at a time
            this.buffer.push(byte);

            // Check if buffer has the required number of bytes
            if (this.buffer.length == this.desiredLength) {
                // Output the buffer
                controller.enqueue(new Uint8Array(this.buffer));
                // Reset buffer
                this.buffer = [];
            }
        }
    }

    flush(controller) {
        controller.enqueue(this.buffer);
    }
}