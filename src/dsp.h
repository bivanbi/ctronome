#ifndef CTRONOME_DSP_H
#define CTRONOME_DSP_H

struct dsp_device {
    int handler;
    word number_of_channels;
    dword sample_rate;
    int dsp_format;
};

struct dsp_device dsp_init(char *, word, word, dword);

void dsp_close(byte);

void dsp_write(byte, byte *, dword);

#endif //CTRONOME_DSP_H
