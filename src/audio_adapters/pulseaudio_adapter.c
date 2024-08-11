#include <stdlib.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "pulseaudio_adapter.h"
#include "logging_adapter.h"

int pulseaudio_init(struct PulseAudioDevice *pulseaudio_device, word bits_per_sample, word number_of_channels, dword sample_rate) {
    pa_sample_spec sample_spec;
    log_message(LEVEL_DEBUG, "PulseAudio: initializing pulseaudio device, channels: %d, bits per sample: %d, sample rate: %d\n",
                number_of_channels, bits_per_sample, sample_rate);

    if (bits_per_sample == 8) {
        sample_spec.format = PA_SAMPLE_U8;
    } else if (bits_per_sample == 16) {
        sample_spec.format = PA_SAMPLE_S16LE;
    } else {
        log_message(LEVEL_ERROR, "PulseAudio: only 8 and 16 bits per sample are supported. (got: %d)\n", bits_per_sample);
        return EXIT_FAILURE;
    }

    sample_spec.channels = number_of_channels;
    sample_spec.rate = sample_rate;

    log_message(LEVEL_DEBUG, "PulseAudio: opening connection to server with sample format: %d\n", sample_spec.format);
    pulseaudio_device->pulseaudio_handler = pa_simple_new(NULL, "Ctronome", PA_STREAM_PLAYBACK, NULL, "playback", &sample_spec, NULL, NULL,
                                                          &pulseaudio_device->error_code);
    if (pulseaudio_device->pulseaudio_handler == NULL) {
        pulseaudio_device->error_message = (char *) pa_strerror(pulseaudio_device->error_code);
        log_message(LEVEL_ERROR, "PulseAudio: pa_simple_new() failed: %s\n", pulseaudio_device->error_message);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int pulseaudio_write(struct PulseAudioDevice *pulseaudio_device, byte *buffer, dword buffer_size) {
    int error_code = pa_simple_write(pulseaudio_device->pulseaudio_handler, buffer, buffer_size, &pulseaudio_device->error_code);
    if (error_code < 0) {
        pulseaudio_device->error_message = (char *) pa_strerror(pulseaudio_device->error_code);
        log_message(LEVEL_ERROR, "PulseAudio: pa_simple_write() failed: %s\n", pulseaudio_device->error_message);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int pulseaudio_close(struct PulseAudioDevice *pulseaudio_device) {
    log_message(LEVEL_DEBUG, "PulseAudio: closing pulseaudio device\n");
    if (pulseaudio_device->pulseaudio_handler != NULL) {
        pa_simple_free(pulseaudio_device->pulseaudio_handler);
    }
    return EXIT_SUCCESS;
}
