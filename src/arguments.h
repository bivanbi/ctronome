#ifndef CTRONOME_ARGUMENTS_H
#define CTRONOME_ARGUMENTS_H
#include <stdint.h>

/* my lazy driver definitions */
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

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

struct Arguments {
    char *wav1_file_path;
    char *wav2_file_path;
    char *dsp_device_path;
    char *program_file_path;
    byte help_requested;
    byte version_requested;
    byte is_program;
    dword current_program_line;
    int beat_per_minute[2];
    int beat_per_tact[2];
    byte bpm_base_specified;
    byte bpt_base_specified;
    dword repeat_count;
    dword tact_repeat_count; // used in programs to set tact repetition times for each program line
    int finite_repetition;
};

struct Arguments parse_command_line_arguments(int, char *[]);

struct Arguments get_default_arguments();

#endif //CTRONOME_ARGUMENTS_H
