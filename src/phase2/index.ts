import * as path from 'path';
import * as fs from 'fs';

// Import the native addon using a flexible path resolution
const nativeAddonPath = path.resolve(__dirname, '../../build/Release/nuked_opm_native.node');
if (!fs.existsSync(nativeAddonPath)) {
    throw new Error(`Native addon not found at ${nativeAddonPath}`);
}
const addon = require(nativeAddonPath);

// WAV file header structure
interface WavHeader {
    riff: string;           // "RIFF"
    fileSize: number;       // File size - 8
    wave: string;           // "WAVE"
    fmt: string;            // "fmt "
    fmtSize: number;        // Format chunk size (16 for PCM)
    audioFormat: number;    // Audio format (1 for PCM)
    numChannels: number;    // Number of channels
    sampleRate: number;     // Sample rate
    byteRate: number;       // Byte rate
    blockAlign: number;     // Block align
    bitsPerSample: number;  // Bits per sample
    data: string;           // "data"
    dataSize: number;       // Data size
}

function createWavHeader(sampleRate: number, numChannels: number, bitsPerSample: number, numSamples: number): Buffer {
    const byteRate = sampleRate * numChannels * bitsPerSample / 8;
    const dataSize = numSamples * numChannels * bitsPerSample / 8;
    const fileSize = dataSize + 36;
    
    const buffer = Buffer.alloc(44);
    
    // RIFF header
    buffer.write('RIFF', 0);
    buffer.writeUInt32LE(fileSize, 4);
    buffer.write('WAVE', 8);
    
    // fmt chunk
    buffer.write('fmt ', 12);
    buffer.writeUInt32LE(16, 16);  // fmt chunk size
    buffer.writeUInt16LE(1, 20);   // audio format (PCM)
    buffer.writeUInt16LE(numChannels, 22);
    buffer.writeUInt32LE(sampleRate, 24);
    buffer.writeUInt32LE(byteRate, 28);
    buffer.writeUInt16LE(numChannels * bitsPerSample / 8, 32);  // block align
    buffer.writeUInt16LE(bitsPerSample, 34);
    
    // data chunk
    buffer.write('data', 36);
    buffer.writeUInt32LE(dataSize, 40);
    
    return buffer;
}

function initYM2151For440Hz(): void {
    // Channel 0 setup for 440Hz
    // Based on YM2151 formula: Frequency = (Clock / (64 * 144)) * 2^(block-1) * (FNUM / 2048)
    // For 440Hz with 3.579545MHz clock and block=4:
    // FNUM ≈ 290 (0x122)
    
    const channel = 0;
    const block = 4;
    const fnum = 290;
    
    // Set up operators for channel 0 (simple sine wave tone)
    // Operator 0 (M1) settings
    const slot = channel * 4; // Slot 0 for channel 0
    
    // DT1/MUL (0x40-0x5F) - Multiplier = 1
    addon.writeRegister(0x40 + slot, 0x01);
    
    // TL (Total Level) (0x60-0x7F) - Volume (0 = max, 127 = silent)
    addon.writeRegister(0x60 + slot, 0x00);
    
    // KS/AR (0x80-0x9F) - Attack Rate
    addon.writeRegister(0x80 + slot, 0x1F); // Fast attack
    
    // AMS-EN/D1R (0xA0-0xBF) - Decay 1 Rate
    addon.writeRegister(0xA0 + slot, 0x00);
    
    // DT2/D2R (0xC0-0xDF) - Decay 2 Rate
    addon.writeRegister(0xC0 + slot, 0x00);
    
    // D1L/RR (0xE0-0xFF) - Release Rate
    addon.writeRegister(0xE0 + slot, 0x0F);
    
    // Set connection/feedback for channel (0x20-0x27)
    // Connection = 0 (simple M1->C1->C2->out), Feedback = 0
    addon.writeRegister(0x20 + channel, 0x00);
    
    // Set Key Code and Fraction (0x28-0x2F and 0x30-0x37)
    // Key Code register (0x28-0x2F)
    const kc = (block << 4) | 0x0A; // block=4, note code for A
    addon.writeRegister(0x28 + channel, kc);
    
    // Key Fraction (0x30-0x37)
    const kf = 0x00;
    addon.writeRegister(0x30 + channel, kf);
    
    // Key On (0x08) - Turn on all operators for channel 0
    addon.writeRegister(0x08, 0x78 | channel); // 0x78 = all 4 operators on
}

/**
 * Generate a 440Hz tone WAV file using Nuked-OPM emulator
 * @param outputPath - Path to the output WAV file
 */
export function generate440HzWav(outputPath: string = 'output.wav'): void {
    console.log('Starting YM2151 (Nuked-OPM) 440Hz tone generator...');
    console.log('Generating 440Hz tone for 3 seconds using Nuked-OPM...');
    
    // YM2151 runs at 3.579545 MHz
    // Sample rate: 55930 Hz (YM2151's internal sample rate)
    const SAMPLE_RATE = 55930;
    const DURATION_SECONDS = 3;
    const NUM_SAMPLES = SAMPLE_RATE * DURATION_SECONDS;
    const NUM_CHANNELS = 2; // Stereo
    const BITS_PER_SAMPLE = 16;
    
    // Initialize OPM chip
    addon.initChip();
    addon.resetChip();
    
    // Set up for 440Hz tone
    initYM2151For440Hz();
    
    // Create WAV header
    const wavHeader = createWavHeader(SAMPLE_RATE, NUM_CHANNELS, BITS_PER_SAMPLE, NUM_SAMPLES);
    
    // Generate samples
    console.log(`Generating ${NUM_SAMPLES} samples at ${SAMPLE_RATE} Hz...`);
    
    const sampleBuffer = Buffer.alloc(NUM_SAMPLES * NUM_CHANNELS * 2); // 2 bytes per sample
    let bufferIndex = 0;
    
    for (let i = 0; i < NUM_SAMPLES; i++) {
        // Clock the chip to generate one sample
        const output = addon.clockChip();
        
        // Convert to 16-bit samples (with scaling)
        const leftSample = Math.max(-32768, Math.min(32767, Math.floor(output[0] / 2)));
        const rightSample = Math.max(-32768, Math.min(32767, Math.floor(output[1] / 2)));
        
        sampleBuffer.writeInt16LE(leftSample, bufferIndex);
        bufferIndex += 2;
        sampleBuffer.writeInt16LE(rightSample, bufferIndex);
        bufferIndex += 2;
        
        // Progress indicator
        if (i % Math.floor(SAMPLE_RATE / 10) === 0) {
            console.log(`Progress: ${Math.floor(i * 100 / NUM_SAMPLES)}%`);
        }
    }
    
    // Write to file
    const finalBuffer = Buffer.concat([wavHeader, sampleBuffer]);
    fs.writeFileSync(outputPath, finalBuffer);
    
    // Cleanup
    addon.cleanupChip();
    
    console.log(`Done! WAV file saved to ${outputPath}`);
    console.log(`Successfully generated WAV file: ${outputPath}`);
}

// If run directly (not imported as module)
if (require.main === module) {
    const outputPath = process.argv[2] || 'output.wav';
    generate440HzWav(outputPath);
}
