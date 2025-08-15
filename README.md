# Audio_filter

Prints the dominant note of an audio file over time. The Fourier transform is
implemented by hand (radix-2 FFT, no DSP libraries): the audio is cut into
windows, each window is transformed, and the strongest frequency is converted
to a note.

```
g++ -O2 -o fourier Fourier.cpp
./fourier song.wav [window_size]
```

- Input: 16-bit PCM `.wav` (mono or stereo). For anything else: `ffmpeg -i song.mp3 song.wav`.
- `window_size`: samples per window, power of two, default `4096` (~93 ms at 44.1 kHz).
  Bigger = finer frequency (better for low notes), smaller = finer in time. Windows overlap 50%.

```
# 44100 Hz | window 4096 samples (92.9 ms) | bin width 10.77 Hz | hop 2048
# start(s)   freq(Hz)  note
     0.000     439.89   A4  (+0 cents)
     0.975     522.71   C5  (-2 cents)
     2.090   -          silence
```

The note is the *loudest frequency*, not necessarily the pitch you hear: in
sounds with strong harmonics (voice, some instruments) it can come out an
octave or so above the real note.
