#ifndef CTRONOME_H
#define CTRONOME_H

#include <sys/soundcard.h>
#include <stdint.h>
#include "defaults.h"
#include "program_file_adapter.h"
#include "audio_output_adapter.h"
#include "arguments.h"

#define MAXIMUM_WAV_DATA_SIZE_BYTES 1000000

#define MAXIMUM_BEAT_PER_MINUTE 250
#define MINIMUM_BEAT_PER_MINUTE 30
#define BPM_MINIMUM_BASE_NOTE 1
#define BPM_MAXIMUM_BASE_NOTE 20

#define MINIMUM_BEAT_PER_TACT 1
#define MAXIMUM_BEAT_PER_TACT 50
#define BPT_MINIMUM_BASE_NOTE 1
#define BPT_MAXIMUM_BASE_NOTE 50

/* my lazy driver definitions */
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

struct WavData {
    byte data[MAXIMUM_WAV_DATA_SIZE_BYTES];
    word number_of_channels;
    dword sample_rate;
    word bits_per_sample;
};

struct Arguments get_arguments(int, char *[]);

void exit_with_help();

void exit_with_version();

void print_arguments(struct Arguments *);

struct AudioOutputSettings get_audio_settings(struct Arguments *, struct WavData *);

void parse_audio_output_driver(struct AudioOutputSettings *, struct Arguments *);

void apply_base_note_defaults(struct Arguments *);

void apply_beat_per_minute_limits(struct Arguments *);

void apply_beat_per_tact_limits(struct Arguments *);

void apply_program_line(struct Arguments *, struct ProgramLine *);

void read_wav_file(struct WavData *, char *);

void verify_wav_files(struct WavData *, struct WavData *);

void output_tact(struct AudioOutputDevice *, struct WavData *, struct WavData *, struct Arguments *);

void play_program(struct AudioOutputDevice *, struct WavData *, struct WavData *, struct Arguments *);

void play_simple_tact(struct AudioOutputDevice *, struct WavData *, struct WavData *, struct Arguments *);

#endif //CTRONOME_H