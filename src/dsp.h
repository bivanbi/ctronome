#ifndef CTRONOME_DSP_H
#define CTRONOME_DSP_H

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

struct DspDevice {
    int handler;
    word number_of_channels;
    dword sample_rate;
    int dsp_format;
};

struct DspDevice dsp_init(char *, word, word, dword);

void dsp_close(byte);

void dsp_write(byte, byte *, dword);

void parse_dsp_bits_per_sample(struct DspDevice *dsp_device, word bits_per_sample);
void parse_dsp_number_of_channels(struct DspDevice *dsp_device, word number_of_channels);
void parse_dsp_sample_rate(struct DspDevice *dsp_device, dword sample_rate);

void open_dsp_device(struct DspDevice *, char *);
void set_dsp_format(struct DspDevice *, char *);
void set_dsp_number_of_channels(struct DspDevice *, char *);
void set_dsp_sample_rate(struct DspDevice *, char *);

void verify_dsp_device(struct DspDevice *, word, word, dword);

#endif //CTRONOME_DSP_H
