#ifndef CTRONOME_H
#define CTRONOME_H

#include <sys/soundcard.h>
#include <stdint.h>
#include "defaults.h"
#include "program_file_adapter.h"

#define MY_NAME "ctronome"
#define HOMEPAGE "https://github.com/bivanbi/ctronome\n"

#define HELP "usage: ctronome <parameters>\n\
Play metronome sound at desired speed through sound card\n\
Complex rhytmic patterns can be achieved by using a program file.\n\
\n\
Valid parameters are:\n\
    -b <bpm>            beat per minute\n\
    -t <bpt>            beat per tact\n\
    -p <filename>       program file\n\
    -c <count>          play tact/program <count> times then exit\n\
    -w1 <filename>      wav to use for first (accented) beat of tact\n\
    -w2 <filename>      wav to use for other beat of tact\n\
    -d <device>         dsp device\n\
    -h                  display this help screen\n\
    -v                  print version\n\
    -debug              verbose output\n\
\n\
Example: ctronome -b 60 -t 4\n\
\n\
For defaults/limits and required WAV format see README.md\n\
For example program file see docs/prog_example.txt\n\
\n\
Home page: " HOMEPAGE "\n"

#define MAXIMUM_WAV_DATA_SIZE_BYTES 1000000

#define MAXIMUM_BEAT_PER_MINUTE 250
#define MINIMUM_BEAT_PER_MINUTE 30
#define BPM_MINIMUM_BASE_NOTE 1
#define BPM_MAXIMUM_BASE_NOTE 20

#define MINIMUM_BEAT_PER_TACT 1
#define MAXIMUM_BEAT_PER_TACT 50
#define BPT_MINIMUM_BASE_NOTE 1
#define BPT_MAXIMUM_BASE_NOTE 50

/* my lazy type definitions */
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

struct Arguments {
    char *wav1_file_path;
    char *wav2_file_path;
    char *dsp_device_path;
    char *program_file_path;
    byte help_requested;
    byte version_requested;
    byte is_program;
    int beat_per_minute[2];
    int beat_per_tact[2];
    byte bpm_base_specified;
    byte bpt_base_specified;
    int repeat_count;
    int tact_repeat_count; // used in programs to set tact repetition times for each program line
    int finite_repetition;
};


struct Arguments get_default_arguments();

struct Arguments parse_command_line_arguments(int, char *[]);

void exit_with_help();

void exit_with_version();

void print_arguments(struct Arguments *);

void apply_base_note_defaults(struct Arguments *);

void apply_beat_per_minute_limits(struct Arguments *);

void apply_beat_per_tact_limits(struct Arguments *);

void read_next_program_line(struct Arguments *, struct ProgramFile *); /* process the next line of program */

void read_wav_file(struct WavData *, char *);

void verify_wav_files(struct WavData *, struct WavData *);

struct DspDevice open_sound_device(char *, struct WavData *);

#endif //CTRONOME_H