#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/soundcard.h>
#include "ctronome.h"
#include "dsp.h"

static int dsp_8bit_unsigned_format = AFMT_U8; /* 0x00000008 unsigned 8 bit */
static int dsp_16bit_signed_format = AFMT_S16_LE; /* 0x00000010 signed 16 bit little endianness */

static int supported_dsp_formats;

struct dsp_device dsp_init(char *dsp_device_path, word bits_per_sample, word number_of_channels, dword sample_rate) {
    struct dsp_device dsp_device;

    parse_dsp_bits_per_sample(&dsp_device, bits_per_sample);
    parse_dsp_number_of_channels(&dsp_device, number_of_channels);
    parse_dsp_sample_rate(&dsp_device, sample_rate);

    open_dsp_device(&dsp_device, dsp_device_path);
    set_dsp_format(&dsp_device, dsp_device_path);
    set_dsp_number_of_channels(&dsp_device, dsp_device_path);
    set_dsp_sample_rate(&dsp_device, dsp_device_path);

    verify_dsp_device(&dsp_device, bits_per_sample, number_of_channels, sample_rate);

    return (dsp_device);
}

void parse_dsp_bits_per_sample(struct dsp_device *dsp_device, word bits_per_sample) {
    dsp_device->dsp_format = 0;
    if (debug) printf("debug: dsp.c: bits per sample: >%d<\n", bits_per_sample);

    if (bits_per_sample == 8) {
        dsp_device->dsp_format = dsp_8bit_unsigned_format;
    } else if (bits_per_sample == 16) {
        dsp_device->dsp_format = dsp_16bit_signed_format;
    } else {
        printf("FATAL: only 8 and 16 bits per sample are supported. (got: %d\n", bits_per_sample);
        exit(1);
    }

    if (debug) printf("debug: dsp.c: wanted AFMT: >%d<\n", dsp_device->dsp_format);
}

void parse_dsp_number_of_channels(struct dsp_device *dsp_device, word number_of_channels) {
    if ((number_of_channels < 1) || (number_of_channels > 2)) {
        printf("FATAL: number of channels must be 1 (mono) or 2 (stereo) (got: %d\n", number_of_channels);
        exit(1);
    }
    dsp_device->number_of_channels = number_of_channels;
}

void parse_dsp_sample_rate(struct dsp_device *dsp_device, dword sample_rate) {
    if ((sample_rate < 8000) || (sample_rate > 96000)) {
        printf("FATAL: sample rate must be between 8000 and 96000 (got: %d\n", sample_rate);
        exit(1);
    }
    dsp_device->sample_rate = sample_rate;
}

void open_dsp_device(struct dsp_device *dsp_device, char *dsp_device_path) {
    if (debug) printf("debug: opening dsp '%s'\n", dsp_device_path);
    if ((dsp_device->handler = open(dsp_device_path, O_WRONLY)) == -1) {
        printf("FATAL: cannot open dsp device %s\n", dsp_device_path);
        perror(dsp_device_path);
        exit(1);
    }
    if (debug) printf("debug: dsp device %s opened successfully\n", dsp_device_path);
}

void set_dsp_format(struct dsp_device *dsp_device, char *dsp_device_path) {
    if (debug) {
        if (ioctl(dsp_device->handler, SNDCTL_DSP_GETFMTS, &supported_dsp_formats) == -1) {
            printf("FATAL: unable to get supported formats from '%s'\n", dsp_device_path);
            perror("SNDCTL_DSP_GETFMT");
            exit(1);
        }
        printf("debug: supported dsp formats: '%d'\n", supported_dsp_formats);
    }

    if (debug) printf("debug: dsp: set format: SNDCTL_DSP_SETFMT(%d)\n", dsp_device->dsp_format);
    if ((ioctl(dsp_device->handler, SNDCTL_DSP_SETFMT, &dsp_device->dsp_format)) == -1) {
        printf("FATAL: unable to set output format for %s\n", dsp_device_path);
        perror("SNDCTL_DSP_SETFMT");
        exit(1);
    }

    if (debug) printf("debug: dsp: set format returned '%d'\n", dsp_device->dsp_format);
}

void set_dsp_sample_rate(struct dsp_device *dsp_device, char *dsp_device_path) {
    if (debug) printf("debug: dsp: set sample rate: SNDCTL_DSP_SPEED(%d)\n", dsp_device->sample_rate);
    if (ioctl(dsp_device->handler, SNDCTL_DSP_SPEED, &dsp_device->sample_rate) == -1) {
        printf("FATAL: unable to set sample rate for %s\n", dsp_device_path);
        perror("SNDCTL_DSP_SPEED");
        exit(1);
    }
}

void set_dsp_number_of_channels(struct dsp_device *dsp_device, char *dsp_device_path) {
    if (debug) printf("debug: dsp: set number of channels: SNDCTL_DSP_CHANNELS(%d)\n", dsp_device->number_of_channels);
    if (ioctl(dsp_device->handler, SNDCTL_DSP_CHANNELS, &dsp_device->number_of_channels) == -1) {
        printf("FATAL: unable to set no. of channels for %s\n", dsp_device_path);
        perror("SNDCTL_DSP_CHANNELS");
        exit(1);
    }
}

void verify_dsp_device(struct dsp_device *dsp_device, word bits_per_sample, word number_of_channels, dword sample_rate) {
    if ((bits_per_sample == 8) && (dsp_device->dsp_format != dsp_8bit_unsigned_format)) {
        printf("FATAL: your dsp device does not seem to support unsigned 8 bit (AFMT_8U) format\n");
        exit(1);
    }

    if ((bits_per_sample == 16) && (dsp_device->dsp_format != dsp_16bit_signed_format)) {
        printf("FATAL: your dsp device does not seem to support signed 16 bit (AFMT_S16_LE) format\n");
        exit(1);
    }

    if (dsp_device->number_of_channels != number_of_channels) {
        printf("FATAL: your wav has %d channels, while your DSP only supports %d channels.\n", number_of_channels, dsp_device->number_of_channels);
        exit(1);
    }

    if (debug && (dsp_device->sample_rate != sample_rate)) {
        printf("debug: dsp: anticipated sample rate and actual dsp sample rate differs, will sound funny\n");
    }
}

void dsp_close(byte dsp_handle) {
    close(dsp_handle);
}

void dsp_write(byte dsp_handle, byte *from, dword count) {
    dword bytes_written;
    bytes_written = write(dsp_handle, from, count);
    if (bytes_written < count) {
        printf("FATAL: tried to write %d bytes to dsp device,\nonly %d is written\n", count, bytes_written);
        exit(1);
    }
}
