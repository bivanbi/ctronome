#ifndef CTRONOME_DSP_H
#define CTRONOME_DSP_H

struct dsp_device {
    int handler;
    word number_of_channels;
    dword sample_rate;
    int dsp_format;
};

struct dsp_device dsp_init(byte *,word,word,dword);

#endif //CTRONOME_DSP_H
