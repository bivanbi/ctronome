#include <string.h>
#include "audio_output_adapter.h"
#include "logging_adapter.h"

#ifdef HAVE_SOUNDCARD_H

#include "audio_adapters/dsp_adapter.h"

#endif // HAVE_SOUNDCARD_H

#ifdef HAVE_PULSEAUDIO_SIMPLE_H

#include "audio_adapters/pulseaudio_adapter.h"

#endif // HAVE_PULSEAUDIO_SIMPLE_H

const char *AudioOutputTypeNames[NUMBER_OF_AUDIO_OUTPUT_TYPES] = {
        "dsp",
        "pulseaudio"
};

int get_audio_output_device_by_name(enum AudioOutputDriver *driver, const char *name) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: get_audio_output_device_by_name: %s\n", name);
    for (int i = 0; i < NUMBER_OF_AUDIO_OUTPUT_TYPES; i++) {
        if (strcmp(name, AudioOutputTypeNames[i]) == 0) {
            *driver = i;
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}

int open_audio_output_device(struct AudioOutputDevice *device) {
    if (device->settings->auto_select_driver == AUTO_SELECT_ENABLED) {
        return open_automatically_selected_audio_output_device(device);
    }

    return open_specific_audio_output_device(device);
}

int open_automatically_selected_audio_output_device(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_automatically_selected_audio_output_device\n");
#ifdef HAVE_PULSEAUDIO_SIMPLE_H
    if ((is_pulseaudio_output_driver_available() == EXIT_SUCCESS) && (open_pulseaudio_device_for_playback(device) == EXIT_SUCCESS)) {
        log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_automatically_selected_audio_output_device: pulseaudio opened successfully\n");
        device->settings->driver = AUDIO_OUTPUT_TYPE_PULSEAUDIO;
        return EXIT_SUCCESS;
    } else {
        log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_automatically_selected_audio_output_device: pulseaudio not available\n");
    }
#endif
#ifdef HAVE_SOUNDCARD_H
    if ((is_dsp_output_driver_available() == EXIT_SUCCESS) && (open_dsp_device_for_playback(device) == EXIT_SUCCESS)) {
        log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_automatically_selected_audio_output_device: dsp opened successfully\n");
        device->settings->driver = AUDIO_OUTPUT_TYPE_DSP;
        return EXIT_SUCCESS;
    } else {
        log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_automatically_selected_audio_output_device: dsp not available\n");
    }
#endif

    log_message(LEVEL_ERROR, "audio_output_adapter.c: open_automatically_selected_audio_output_device: no supported / functioning audio output driver found\n");
    exit(EXIT_FAILURE);
}

int open_specific_audio_output_device(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_specific_audio_output_device: %s (driver %d)\n",
                get_audio_output_name(device->settings->driver), device->settings->driver);

    if (is_audio_output_driver_available(device->settings->driver) == EXIT_FAILURE) {
        log_message(LEVEL_ERROR, "audio_output_adapter.c: open_specific_audio_output_device: support for %s (%d) is not compiled in. Programming error?\n",
                    get_audio_output_name(device->settings->driver), device->settings->driver);
        exit(EXIT_FAILURE);
    }

    switch (device->settings->driver) {
#ifdef HAVE_PULSEAUDIO_SIMPLE_H
        case AUDIO_OUTPUT_TYPE_PULSEAUDIO:
            return open_pulseaudio_device_for_playback(device);
#endif // HAVE_PULSEAUDIO_SIMPLE_H
#ifdef HAVE_SOUNDCARD_H
        case AUDIO_OUTPUT_TYPE_DSP:
            return open_dsp_device_for_playback(device);
#endif // HAVE_SOUNDCARD_H
        default:
            log_message(LEVEL_ERROR, "audio_output_adapter.c: open_specific_audio_output_device: unsupported audio output driver %d\n",
                        device->settings->driver);
            exit(EXIT_FAILURE);
    }
}

int output_to_audio_device(struct AudioOutputDevice *device, byte *audio_data, dword number_of_bytes) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: output_to_audio_device: %s (%d): %d bytes\n",
                AudioOutputTypeNames[device->settings->driver], device->settings->driver, number_of_bytes);

    switch (device->settings->driver) {
#ifdef HAVE_SOUNDCARD_H
        case AUDIO_OUTPUT_TYPE_DSP:
            return dsp_write(&device->dsp_device, audio_data, number_of_bytes);
#endif // HAVE_SOUNDCARD_H
#ifdef HAVE_PULSEAUDIO_SIMPLE_H
        case AUDIO_OUTPUT_TYPE_PULSEAUDIO:
            return pulseaudio_write(&device->pulseaudio_device, audio_data, number_of_bytes);
#endif // HAVE_PULSEAUDIO_SIMPLE_H
        default:
            log_message(LEVEL_ERROR, "audio_output_adapter.c: output_to_audio_device: unsupported audio output driver %d\n",
                        device->settings->driver);
            exit(EXIT_FAILURE);
    }
}

int close_audio_output_device(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: close_audio_device\n");
    switch (device->settings->driver) {
#ifdef HAVE_SOUNDCARD_H
        case AUDIO_OUTPUT_TYPE_DSP:
            return dsp_close(&device->dsp_device);
#endif // HAVE_SOUNDCARD_H
#ifdef HAVE_PULSEAUDIO_SIMPLE_H
        case AUDIO_OUTPUT_TYPE_PULSEAUDIO:
            return pulseaudio_close(&device->pulseaudio_device);
#endif // HAVE_PULSEAUDIO_SIMPLE_H
        default:
            log_message(LEVEL_ERROR, "audio_output_adapter.c: close_audio_device: unsupported audio output driver %d\n",
                        device->settings->driver);
            exit(EXIT_FAILURE);
    }
}

const char *get_audio_output_name(enum AudioOutputDriver type) {
    if (type < NUMBER_OF_AUDIO_OUTPUT_TYPES) {
        return AudioOutputTypeNames[type];
    } else {
        return "UNKNOWN";
    }
}

int is_audio_output_driver_available(enum AudioOutputDriver type) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: is_audio_output_driver_available: driver %d\n", type);
    switch (type) {
        case AUDIO_OUTPUT_TYPE_DSP:
            return is_dsp_output_driver_available();
        case AUDIO_OUTPUT_TYPE_PULSEAUDIO:
            return is_pulseaudio_output_driver_available();
        default:
            return EXIT_FAILURE;
    }
}

#ifdef HAVE_SOUNDCARD_H

int is_dsp_output_driver_available() {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: is_dsp_output_driver_available: true\n");
    return EXIT_SUCCESS;
}

int open_dsp_device_for_playback(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_dsp_device_for_playback\n");
    device->dsp_device.path = device->settings->dsp_device_path;
    return dsp_init(&device->dsp_device, device->settings->bits_per_sample,
                    device->settings->number_of_channels, device->settings->sample_rate);
}

#else
int is_dsp_output_driver_available() {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: is_dsp_output_driver_available: false\n");
    return EXIT_FAILURE;
}
#endif // HAVE_SOUNDCARD_H

#ifdef HAVE_PULSEAUDIO_SIMPLE_H

int is_pulseaudio_output_driver_available() {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: is_pulseaudio_output_driver_available: true\n");
    return EXIT_SUCCESS;
}

int open_pulseaudio_device_for_playback(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_pulseaudio_device_for_playback\n");
    // word bits_per_sample, word number_of_channels, dword sample_rate
    return pulseaudio_init(&device->pulseaudio_device, device->settings->bits_per_sample,
                           device->settings->number_of_channels, device->settings->sample_rate);

}

#else

int is_pulseaudio_output_driver_available() {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: is_pulseaudio_output_driver_available: false\n");
    return EXIT_FAILURE;
}

#endif // HAVE_PULSEAUDIO_SIMPLE_H
