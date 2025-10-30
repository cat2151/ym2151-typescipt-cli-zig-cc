#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "opm.h"

// WAV file header structure
typedef struct {
    char riff[4];           // "RIFF"
    uint32_t file_size;     // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmt_size;      // Format chunk size (16 for PCM)
    uint16_t audio_format;  // Audio format (1 for PCM)
    uint16_t num_channels;  // Number of channels
    uint32_t sample_rate;   // Sample rate
    uint32_t byte_rate;     // Byte rate
    uint16_t block_align;   // Block align
    uint16_t bits_per_sample; // Bits per sample
    char data[4];           // "data"
    uint32_t data_size;     // Data size
} wav_header_t;

// Sleep function (cross-platform)
#ifdef _WIN32
#include <windows.h>
void sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}
#else
#include <unistd.h>
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

void write_wav_header(FILE *file, uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t num_samples) {
    wav_header_t header;
    uint32_t byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    uint32_t data_size = num_samples * num_channels * bits_per_sample / 8;
    
    memcpy(header.riff, "RIFF", 4);
    header.file_size = data_size + 36;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmt_size = 16;
    header.audio_format = 1;
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.byte_rate = byte_rate;
    header.block_align = num_channels * bits_per_sample / 8;
    header.bits_per_sample = bits_per_sample;
    memcpy(header.data, "data", 4);
    header.data_size = data_size;
    
    fwrite(&header, sizeof(wav_header_t), 1, file);
}

void write_register(opm_t *chip, uint8_t address, uint8_t data) {
    // Write address
    OPM_Write(chip, 0, address);
    
    // Write data
    OPM_Write(chip, 1, data);
    
    // Wait 10ms after register write (as required)
    sleep_ms(10);
}

void init_ym2151_for_440hz(opm_t *chip) {
    // Reset chip
    OPM_Reset(chip);
    sleep_ms(10);
    
    // Channel 0 setup for 440Hz
    // Based on YM2151 formula: Frequency = (Clock / (64 * 144)) * 2^(block-1) * (FNUM / 2048)
    // For 440Hz with 3.579545MHz clock and block=4:
    // FNUM ≈ 290 (0x122)
    
    uint8_t channel = 0;
    uint8_t block = 4;
    uint16_t fnum = 290;
    
    // Set up operators for channel 0 (simple sine wave tone)
    // Operator 0 (M1) settings
    uint8_t slot = channel * 4; // Slot 0 for channel 0
    
    // DT1/MUL (0x40-0x5F) - Multiplier = 1
    write_register(chip, 0x40 + slot, 0x01);
    
    // TL (Total Level) (0x60-0x7F) - Volume (0 = max, 127 = silent)
    write_register(chip, 0x60 + slot, 0x00);
    
    // KS/AR (0x80-0x9F) - Attack Rate
    write_register(chip, 0x80 + slot, 0x1F); // Fast attack
    
    // AMS-EN/D1R (0xA0-0xBF) - Decay 1 Rate
    write_register(chip, 0xA0 + slot, 0x00);
    
    // DT2/D2R (0xC0-0xDF) - Decay 2 Rate
    write_register(chip, 0xC0 + slot, 0x00);
    
    // D1L/RR (0xE0-0xFF) - Release Rate
    write_register(chip, 0xE0 + slot, 0x0F);
    
    // Set connection/feedback for channel (0x20-0x27)
    // Connection = 0 (simple M1->C1->C2->out), Feedback = 0
    write_register(chip, 0x20 + channel, 0x00);
    
    // Set Key Code and Fraction (0x28-0x2F and 0x30-0x37)
    // Key Code register (0x28-0x2F)
    uint8_t kc = (block << 4) | 0x0A; // block=4, note code for A
    write_register(chip, 0x28 + channel, kc);
    
    // Key Fraction (0x30-0x37)
    uint8_t kf = 0x00;
    write_register(chip, 0x30 + channel, kf);
    
    // Key On (0x08) - Turn on all operators for channel 0
    write_register(chip, 0x08, 0x78 | channel); // 0x78 = all 4 operators on
}

int main(int argc, char *argv[]) {
    const char *output_filename = "output.wav";
    if (argc > 1) {
        output_filename = argv[1];
    }
    
    printf("Generating 440Hz tone for 3 seconds using Nuked-OPM...\n");
    
    // YM2151 runs at 3.579545 MHz
    // Sample rate: 55930 Hz (YM2151's internal sample rate)
    const uint32_t SAMPLE_RATE = 55930;
    const uint32_t DURATION_SECONDS = 3;
    const uint32_t NUM_SAMPLES = SAMPLE_RATE * DURATION_SECONDS;
    const uint16_t NUM_CHANNELS = 2; // Stereo
    const uint16_t BITS_PER_SAMPLE = 16;
    
    // Initialize OPM chip
    opm_t chip;
    memset(&chip, 0, sizeof(opm_t));
    OPM_Reset(&chip);
    
    // Set up for 440Hz tone
    init_ym2151_for_440hz(&chip);
    
    // Open output file
    FILE *wav_file = fopen(output_filename, "wb");
    if (!wav_file) {
        fprintf(stderr, "Error: Could not open output file %s\n", output_filename);
        return 1;
    }
    
    // Write WAV header
    write_wav_header(wav_file, SAMPLE_RATE, NUM_CHANNELS, BITS_PER_SAMPLE, NUM_SAMPLES);
    
    // Generate samples
    printf("Generating %u samples at %u Hz...\n", NUM_SAMPLES, SAMPLE_RATE);
    
    int16_t *sample_buffer = (int16_t *)malloc(NUM_SAMPLES * NUM_CHANNELS * sizeof(int16_t));
    if (!sample_buffer) {
        fprintf(stderr, "Error: Could not allocate sample buffer\n");
        fclose(wav_file);
        return 1;
    }
    
    for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
        int32_t output[2];
        uint8_t sh1, sh2, so;
        
        // Clock the chip to generate one sample
        OPM_Clock(&chip, output, &sh1, &sh2, &so);
        
        // Convert to 16-bit samples (with scaling)
        sample_buffer[i * 2] = (int16_t)(output[0] / 2); // Left channel
        sample_buffer[i * 2 + 1] = (int16_t)(output[1] / 2); // Right channel
        
        // Progress indicator
        if (i % (SAMPLE_RATE / 10) == 0) {
            printf("Progress: %d%%\n", (int)(i * 100 / NUM_SAMPLES));
        }
    }
    
    // Write samples to file
    fwrite(sample_buffer, sizeof(int16_t), NUM_SAMPLES * NUM_CHANNELS, wav_file);
    
    // Clean up
    free(sample_buffer);
    fclose(wav_file);
    
    printf("Done! WAV file saved to %s\n", output_filename);
    
    return 0;
}
