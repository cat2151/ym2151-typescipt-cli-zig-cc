import { execSync } from 'child_process';
import * as path from 'path';
import * as fs from 'fs';

/**
 * Generate a 440Hz tone WAV file using Nuked-OPM emulator
 * @param outputPath - Path to the output WAV file
 */
export function generate440HzWav(outputPath: string = 'output.wav'): void {
    console.log('Starting YM2151 (Nuked-OPM) 440Hz tone generator...');
    
    // The executable is in the source directory, not dist
    const executablePath = path.join(__dirname, '..', '..', 'src', 'phase2', 'opm_generator');
    
    // Check if executable exists
    if (!fs.existsSync(executablePath)) {
        throw new Error(`Executable not found: ${executablePath}. Please run 'npm run build' first.`);
    }
    
    try {
        // Execute the C program
        const output = execSync(`"${executablePath}" "${outputPath}"`, {
            encoding: 'utf-8',
            stdio: 'pipe'
        });
        
        console.log(output);
        console.log(`Successfully generated WAV file: ${outputPath}`);
    } catch (error) {
        console.error('Error generating WAV file:', error);
        throw error;
    }
}

// If run directly (not imported as module)
if (require.main === module) {
    const outputPath = process.argv[2] || 'output.wav';
    generate440HzWav(outputPath);
}
