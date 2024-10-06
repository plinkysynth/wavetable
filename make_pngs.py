import os
import wave
import struct
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft

# Configuration
folder_path = "."  # Change this to the root folder you want to start from
img_width, img_height = 768, 384

# Utility functions
def load_wav_file(filepath):
    try:
        with wave.open(filepath, 'rb') as wav_file:
            channels = wav_file.getnchannels()
            sampwidth = wav_file.getsampwidth()
            framerate = wav_file.getframerate()
            nframes = wav_file.getnframes()
            # Read and unpack the data based on sample width
            raw_data = wav_file.readframes(nframes)
            if sampwidth == 2:  # 16-bit
                dtype = np.int16
                data = np.frombuffer(raw_data, dtype=dtype).reshape(-1, channels)
            elif sampwidth == 3:  # 24-bit
                # Convert 3-byte data to int32, ensuring correct byte alignment
                raw_data = np.frombuffer(raw_data, dtype=np.uint8).reshape(-1, 3 * channels)
                data = np.zeros((raw_data.shape[0], channels), dtype=np.int32)
                for i in range(channels):
                    # Combine 3 bytes into a single int32
                    data[:, i] = (raw_data[:, 3 * i + 2] << 16) | (raw_data[:, 3 * i + 1] << 8) | raw_data[:, 3 * i]
                    data[:, i] = np.where(data[:, i] & 0x800000, data[:, i] - 0x1000000, data[:, i])  # Sign extension
            else:
                return None  # Skip unsupported bit depths

            # Select the first channel and normalize to -1 to 1 range
            first_channel = data[:, 0]
            max_val = 2**(sampwidth * 8 - 1)
            normalized_samples = first_channel / max_val
            return normalized_samples

    except Exception as e:
        print(f"Failed to process {filepath}: {e}")
        return None


def plot_waveform_and_harmonics(normalized_samples, output_image_path):
    fig, ax = plt.subplots(figsize=(img_width/100, img_height/100), dpi=100)
    ax.set_xlim(-32, img_width+32)
    ax.set_ylim(-32, img_height+32)
    border_size = 0.1
    fig.patch.set_facecolor('black')
    ax.set_facecolor('black')
    ax.axis('off')

    # Normalize the samples to range [-1, 1]
    sample_count = len(normalized_samples)

    # Draw the top half (waveform)
    time_x = np.linspace(0, img_width, sample_count)
    y_waveform = (3 - normalized_samples) * (img_height / 4 - 1)  # Scale y to fit top half
    ax.plot(time_x, y_waveform, color='white', linewidth=1, antialiased=True)

    # Draw the bottom half (harmonics)
    harmonic_count = 16
    fft_values = (np.abs(fft(normalized_samples)[1:harmonic_count+1]) + 1e-6)
    peak = np.max(fft_values)
    fft_values = np.clip(fft_values, 0, peak)
    for i, value in enumerate(fft_values):
        x_pos = img_width * (i / (harmonic_count-1))
        y_pos = (value)/peak * (img_height / 2.2)  # Scale to bottom half height
        ax.plot([x_pos, x_pos], [0, y_pos], color='white', linewidth=1)
        ax.scatter(x_pos, y_pos, color='white', s=10)

    # Save the image
    fig.savefig(output_image_path, facecolor='black', bbox_inches='tight', pad_inches=0)
    plt.close(fig)

def process_folder(folder_path):
    for root, _, files in os.walk(folder_path):
        for filename in files:
            if filename.lower().endswith(".wav"):
                wav_path = os.path.join(root, filename)
                output_image_path = os.path.splitext(wav_path)[0] + ".png"
                samples = load_wav_file(wav_path)
                if samples is not None:
                    print(f"Processing {wav_path}")
                    plot_waveform_and_harmonics(samples, output_image_path)
                else:
                    print(f"Skipping {wav_path}")

# Run the script
process_folder(folder_path)
