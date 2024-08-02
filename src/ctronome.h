#ifndef CTRONOME_H
#define CTRONOME_H

#include <sys/soundcard.h>
#include <stdint.h>
#include "defaults.h"

#define MYNAME "ctronome"
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

/* my lazy type definitions */
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef DWORD dword;
typedef WORD word;
typedef BYTE byte;

static const char slash = 47; /* the / character */
static const char hashmark = 35; /* the # character */
static const char space = 32; /* the   character */

struct WavData {
    byte data[MAXIMUM_WAV_DATA_SIZE_BYTES];
    word number_of_channels;
    dword sample_rate;
    word bits_per_sample;
};

void set_default_values();

void parse_next_program_line(FILE *); /* process the next line of program */
void parse_command_line_arguments(int, char *[]);

void read_wav_file(struct WavData *, char *);

void verify_wav_files();

void open_program_file();

void open_sound_device();

extern byte debug;

#endif //CTRONOME_H