#ifndef CTRONOME_AUDIO_OUTPUT_ADAPTER_H
#define CTRONOME_AUDIO_OUTPUT_ADAPTER_H

#include <stdlib.h>
#include <stdint.h>

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

#ifdef HAVE_SOUNDCARD_H

#include "audio_adapters/dsp_adapter.h"

#endif

#ifdef HAVE_PULSEAUDIO_SIMPLE_H
#include "audio_adapters/pulseaudio_adapter.h"
#endif

enum AudioOutputDriver {
    AUDIO_OUTPUT_TYPE_DSP,
    AUDIO_OUTPUT_TYPE_PULSEAUDIO,
    NUMBER_OF_AUDIO_OUTPUT_TYPES
};

enum AutoSelectDriver {
    AUTO_SELECT_DISABLED,
    AUTO_SELECT_ENABLED
};

extern const char *AudioOutputTypeNames[NUMBER_OF_AUDIO_OUTPUT_TYPES];

const char *get_audio_output_name(enum AudioOutputDriver);

struct AudioOutputSettings {
    enum AutoSelectDriver auto_select_driver;
    enum AudioOutputDriver driver;
    char *dsp_device_path;
    word number_of_channels;
    dword sample_rate;
    word bits_per_sample;
};

struct AudioOutputDevice {
    struct AudioOutputSettings *settings;
#ifdef HAVE_SOUNDCARD_H
    struct DspDevice dsp_device;
#endif
#ifdef HAVE_PULSEAUDIO_SIMPLE_H
    struct PulseAudioDevice pulseaudio_device;
#endif
};

int is_audio_output_driver_available(enum AudioOutputDriver);

int is_dsp_output_driver_available();

int is_pulseaudio_output_driver_available();

int open_audio_output_device(struct AudioOutputDevice *);

int open_specific_audio_output_device(struct AudioOutputDevice *);

int open_automatically_selected_audio_output_device(struct AudioOutputDevice *device);

#ifdef HAVE_SOUNDCARD_H
int open_dsp_device_for_playback(struct AudioOutputDevice *);
#endif

#ifdef HAVE_PULSEAUDIO_SIMPLE_H
int open_pulseaudio_device_for_playback(struct AudioOutputDevice *);
#endif

int output_to_audio_device(struct AudioOutputDevice *, byte *, dword);

int close_audio_output_device(struct AudioOutputDevice *);

#endif //CTRONOME_AUDIO_OUTPUT_ADAPTER_H
