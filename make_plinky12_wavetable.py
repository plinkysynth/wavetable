# this little script converts all the OG plinky wavetables into
# wav files with 128 interpolated shapes, suitable for (eg) plinky12....
import sys
import os
import numpy as np
import scipy.io.wavfile
import glob
def process_wavetable(wavetable_path, output_path):
    data = np.fromfile(wavetable_path, dtype='<i2').reshape(-1, 256)[:, 16:144].ravel()
    data = data[:17*1031].reshape(17,1031) # this is the full og plinky wavetable 
    data = data[:, :512] # takes the first mip (512 samples) 
    positions = np.linspace(0, 16, 129)[1:]
    idx = positions.astype(int)
    frac = positions - idx
    next_idx = np.clip(idx+1, 0, 16)

    interp = (1 - frac[:, None]) * data[idx] + frac[:, None] * data[next_idx]
    interp = interp.astype('<i2').ravel()

    # 33511 as a sample rate makes it a C!
    scipy.io.wavfile.write(output_path, 33511, interp)
os.makedirs("p12", exist_ok=True)
for wavetable_path in glob.glob("*/*.uf2"):
    nm = os.path.dirname(wavetable_path)
    nm = nm[:8]  # Clip nm to max 8 letters
    output_path = "p12/" + nm + ".wav"
    print(wavetable_path, '->', output_path)
    process_wavetable(wavetable_path, output_path)
