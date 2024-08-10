#ifndef CTRONOME_DSP_ADAPTER_H
#define CTRONOME_DSP_ADAPTER_H

#include <stdint.h>

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

struct DspDevice {
    char *path;
    int handle;
    word number_of_channels;
    dword sample_rate;
    int dsp_format;
};

int dsp_init(struct DspDevice *, word, word, dword);

int dsp_write(struct DspDevice *, byte *, dword);

int dsp_close(struct DspDevice *);

int parse_dsp_bits_per_sample(struct DspDevice *dsp_device, word bits_per_sample);

int parse_dsp_number_of_channels(struct DspDevice *dsp_device, word number_of_channels);

int parse_dsp_sample_rate(struct DspDevice *dsp_device, dword sample_rate);

int dsp_open(struct DspDevice *dsp_device);

int set_dsp_format(struct DspDevice *);

int set_dsp_number_of_channels(struct DspDevice *);

int set_dsp_sample_rate(struct DspDevice *);

int verify_dsp_device(struct DspDevice *, word, word, dword);

#endif //CTRONOME_DSP_ADAPTER_H
