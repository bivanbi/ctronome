#include "audio_output_adapter.h"
#include "logging_adapter.h"

#ifdef HAVE_SOUNDCARD_H

#include "audio_adapters/dsp_adapter.h"

#endif

const char *AudioOutputTypeNames[NUMBER_OF_AUDIO_OUTPUT_TYPES] = {
        "dsp",
};

int open_audio_output_device(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_audio_output_device: auto select driver: %d, preferred driver %s (%d)\n",
                device->settings->auto_select_driver, get_audio_output_name(device->settings->driver), device->settings->driver);
    if (device->settings->auto_select_driver == AUTO_SELECT_ENABLED) {
        return open_automatically_selected_audio_output_device(device);
    }

    return open_specific_audio_output_device(device);
}

int open_automatically_selected_audio_output_device(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_automatically_selected_audio_output_device\n");

#ifdef HAVE_SOUNDCARD_H
    device->dsp_device.path = device->settings->dsp_device_path;
    return dsp_init(&device->dsp_device, device->settings->bits_per_sample,
                    device->settings->number_of_channels, device->settings->sample_rate);
#endif

    log_message(LEVEL_ERROR, "audio_output_adapter.c: open_automatically_selected_audio_output_device: no supported / functioning audio output driver found\n");
    exit(EXIT_FAILURE);
}

int open_specific_audio_output_device(struct AudioOutputDevice *device) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: open_specific_audio_output_device: %s (driver %d)\n",
                get_audio_output_name(device->settings->driver), device->settings->driver);

    if (is_audio_output_driver_available(device->settings->driver) == EXIT_FAILURE) {
        log_message(LEVEL_ERROR, "audio_output_adapter.c: open_specific_audio_output_device: support for %s (%d) is not compiled in. Programming error?\n",
                    AudioOutputTypeNames[device->settings->driver], device->settings->driver);
        exit(EXIT_FAILURE);
    }

    switch (device->settings->driver) {
#ifdef HAVE_SOUNDCARD_H
        case AUDIO_OUTPUT_TYPE_DSP:
            device->dsp_device.path = device->settings->dsp_device_path;
            return dsp_init(&device->dsp_device, device->settings->bits_per_sample,
                            device->settings->number_of_channels, device->settings->sample_rate);
#endif
        default:
            log_message(LEVEL_ERROR, "audio_output_adapter.c: open_specific_audio_output_device: unsupported audio output driver %d\n",
                        device->settings->driver);
            exit(EXIT_FAILURE);
    }
}

int output_to_audio_device(struct AudioOutputDevice *device, byte *audio_data, dword number_of_bytes) {
    log_message(LEVEL_DEBUG, "audio_output_adapter.c: output_to_audio_device: %d bytes\n", number_of_bytes);
    switch (device->settings->driver) {
#ifdef HAVE_SOUNDCARD_H
        case AUDIO_OUTPUT_TYPE_DSP:
            return dsp_write(&device->dsp_device, audio_data, number_of_bytes);
#endif
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
#endif
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
#ifdef HAVE_SOUNDCARD_H
        case AUDIO_OUTPUT_TYPE_DSP:
            return is_dsp_output_driver_available();
#endif
        default:
            return EXIT_FAILURE;
    }
}

#ifdef HAVE_SOUNDCARD_H

int is_dsp_output_driver_available() {
    return EXIT_SUCCESS;
}

#else
int is_dsp_output_driver_available() {
    return EXIT_FAILURE;
}
#endif // HAVE_SOUNDCARD_H
