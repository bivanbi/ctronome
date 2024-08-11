#include <pulse/simple.h>
#include <pulse/error.h>

#ifndef CTRONOME_PULSEAUDIO_ADAPTER_H
#define CTRONOME_PULSEAUDIO_ADAPTER_H

/* my lazy driver definitions */
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

struct PulseAudioDevice {
    pa_simple *pulseaudio_handler;
    int error_code;
    char *error_message;
};

int pulseaudio_init(struct PulseAudioDevice *, word, word, dword);

int pulseaudio_write(struct PulseAudioDevice *, byte *, dword);

int pulseaudio_close(struct PulseAudioDevice *);

#endif //CTRONOME_PULSEAUDIO_ADAPTER_H
