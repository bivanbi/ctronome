#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/soundcard.h>
#include "../ctronome.h"
#include "dsp_adapter.h"
#include "../logging_adapter.h"

static int dsp_8bit_unsigned_format = AFMT_U8; /* 0x00000008 unsigned 8 bit */
static int dsp_16bit_signed_format = AFMT_S16_LE; /* 0x00000010 signed 16 bit little endianness */

static int supported_dsp_formats;

int dsp_init(struct DspDevice *device, word bits_per_sample, word number_of_channels, dword sample_rate) {
    if (parse_dsp_bits_per_sample(device, bits_per_sample) == EXIT_FAILURE) return EXIT_FAILURE;
    if (parse_dsp_number_of_channels(device, number_of_channels) == EXIT_FAILURE) return EXIT_FAILURE;
    if (parse_dsp_sample_rate(device, sample_rate) == EXIT_FAILURE) return EXIT_FAILURE;

    if (dsp_open(device) == EXIT_FAILURE) return EXIT_FAILURE;
    if (set_dsp_format(device) == EXIT_FAILURE) return EXIT_FAILURE;
    if (set_dsp_number_of_channels(device) == EXIT_FAILURE) return EXIT_FAILURE;
    if (set_dsp_sample_rate(device) == EXIT_FAILURE) return EXIT_FAILURE;

    if (verify_dsp_device(device, bits_per_sample, number_of_channels, sample_rate) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int parse_dsp_bits_per_sample(struct DspDevice *dsp_device, word bits_per_sample) {
    dsp_device->dsp_format = 0;
    log_message(LEVEL_DEBUG, "dsp.c: bits per sample: >%d<\n", bits_per_sample);
    log_message(LEVEL_DEBUG, "dsp.c: bits per sample: >%d<\n", bits_per_sample);

    if (bits_per_sample == 8) {
        dsp_device->dsp_format = dsp_8bit_unsigned_format;
    } else if (bits_per_sample == 16) {
        dsp_device->dsp_format = dsp_16bit_signed_format;
    } else {
        log_message(LEVEL_ERROR, "DSP:only 8 and 16 bits per sample are supported. (got: %d)\n", bits_per_sample);
        return EXIT_FAILURE;
    }

    log_message(LEVEL_DEBUG, "dsp.c: wanted AFMT: >%d<\n", dsp_device->dsp_format);
    return EXIT_SUCCESS;
}

int parse_dsp_number_of_channels(struct DspDevice *dsp_device, word number_of_channels) {
    if ((number_of_channels < 1) || (number_of_channels > 2)) {
        log_message(LEVEL_ERROR, "number of channels must be 1 (mono) or 2 (stereo) (got: %d\n", number_of_channels);
        return EXIT_FAILURE;
    }
    dsp_device->number_of_channels = number_of_channels;
    return EXIT_SUCCESS;
}

int parse_dsp_sample_rate(struct DspDevice *dsp_device, dword sample_rate) {
    if ((sample_rate < 8000) || (sample_rate > 96000)) {
        log_message(LEVEL_ERROR, "sample rate must be between 8000 and 96000 (got: %d\n", sample_rate);
        return EXIT_FAILURE;
    }
    dsp_device->sample_rate = sample_rate;
    return EXIT_SUCCESS;
}

int dsp_open(struct DspDevice *dsp_device) {
    log_message(LEVEL_DEBUG, "opening dsp '%s'\n", dsp_device->path);
    log_message(LEVEL_DEBUG, "opening dsp '%s'\n", dsp_device->path);
    if ((dsp_device->handle = open(dsp_device->path, O_WRONLY)) == -1) {
        log_message(LEVEL_ERROR, "FATAL: cannot open dsp device %s\n", dsp_device->path);
        log_message(LEVEL_ERROR, "cannot open dsp device %s\n", dsp_device->path);
        perror(dsp_device->path);
        return EXIT_FAILURE;
    }
    log_message(LEVEL_DEBUG, "dsp device %s opened successfully\n", dsp_device->path);
    return EXIT_SUCCESS;
}

int set_dsp_format(struct DspDevice *dsp_device) {
    if (ioctl(dsp_device->handle, SNDCTL_DSP_GETFMTS, &supported_dsp_formats) == -1) {
        log_message(LEVEL_ERROR, "unable to get supported formats from '%s'\n", dsp_device->path);
        perror("SNDCTL_DSP_GETFMT");
        return EXIT_FAILURE;
    }
    log_message(LEVEL_DEBUG, "supported dsp formats: '%d'\n", supported_dsp_formats);

    log_message(LEVEL_DEBUG, "dsp: set format: SNDCTL_DSP_SETFMT(%d)\n", dsp_device->dsp_format);
    if ((ioctl(dsp_device->handle, SNDCTL_DSP_SETFMT, &dsp_device->dsp_format)) == -1) {
        log_message(LEVEL_ERROR, "unable to set output format for %s\n", dsp_device->path);
        perror("SNDCTL_DSP_SETFMT");
        return EXIT_FAILURE;
    }

    log_message(LEVEL_DEBUG, "dsp: set format returned '%d'\n", dsp_device->dsp_format);
    return EXIT_SUCCESS;
}

int set_dsp_sample_rate(struct DspDevice *dsp_device) {
    log_message(LEVEL_DEBUG, "dsp: set sample rate: SNDCTL_DSP_SPEED(%d)\n", dsp_device->sample_rate);
    if (ioctl(dsp_device->handle, SNDCTL_DSP_SPEED, &dsp_device->sample_rate) == -1) {
        log_message(LEVEL_ERROR, "unable to set sample rate for %s\n", dsp_device->path);
        perror("SNDCTL_DSP_SPEED");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int set_dsp_number_of_channels(struct DspDevice *dsp_device) {
    log_message(LEVEL_DEBUG, "dsp: set number of channels: SNDCTL_DSP_CHANNELS(%d)\n", dsp_device->number_of_channels);
    if (ioctl(dsp_device->handle, SNDCTL_DSP_CHANNELS, &dsp_device->number_of_channels) == -1) {
        log_message(LEVEL_ERROR, "unable to set no. of channels for %s\n", dsp_device->path);
        perror("SNDCTL_DSP_CHANNELS");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int verify_dsp_device(struct DspDevice *dsp_device, word bits_per_sample, word number_of_channels, dword sample_rate) {
    if ((bits_per_sample == 8) && (dsp_device->dsp_format != dsp_8bit_unsigned_format)) {
        log_message(LEVEL_ERROR, "your dsp device does not seem to support unsigned 8 bit (AFMT_8U) format\n");
        return EXIT_FAILURE;
    }

    if ((bits_per_sample == 16) && (dsp_device->dsp_format != dsp_16bit_signed_format)) {
        log_message(LEVEL_ERROR, "your dsp device does not seem to support signed 16 bit (AFMT_S16_LE) format\n");
        return EXIT_FAILURE;
    }

    if (dsp_device->number_of_channels != number_of_channels) {
        log_message(LEVEL_ERROR, "your wav has %d channels, while your DSP only supports %d channels.\n", number_of_channels, dsp_device->number_of_channels);
        return EXIT_FAILURE;
    }

    if (dsp_device->sample_rate != sample_rate) {
        log_message(LEVEL_ERROR, "dsp: anticipated sample rate %d and actual dsp sample rate %d differs\n", sample_rate, dsp_device->sample_rate);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int dsp_write(struct DspDevice *dsp_device, byte *from, dword count) {
    dword bytes_written;
    bytes_written = write(dsp_device->handle, from, count);
    if (bytes_written < count) {
        log_message(LEVEL_ERROR, "tried to write %d bytes to dsp device,\nonly %d is written\n", count, bytes_written);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int dsp_close(struct DspDevice *dsp_device) {
    return close(dsp_device->handle);
}
