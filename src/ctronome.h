#include <sys/soundcard.h>
#include <stdint.h>

#define MYNAME "ctronome"
#define HOMEPAGE "https://github.com/bivanbi/ctronome\n"

/* set up these three variables to your system */
static char *metronomewav1 = "/usr/share/ctronome/metronome1.wav\0";
static char *metronomewav2 = "/usr/share/ctronome/metronome2.wav\0";
static char *dsp_device_path = "/dev/dsp\0";

/* means: 1/4, 1/8 etc. */
static int default_base_note = 4;

static char *programfile;

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

static int pcount = 1; /* repeat tact/program pcount times then exit; 0 means repeat infinite times */
static int pdecrease = 0;

static int bpm[2] = {60, 4}; /* 60 bpm is given for 1/4 notes */
static int bpt[2] = {1, 4}; /* beat per tact */
static byte slash = 47; /* the / character */
static byte hashmark = 35; /* the # character */
static byte space = 32; /* the   character */

void next_program(FILE *); /* process the next line of program */
void parm_init(int, char *[]);

void dsp_close(byte);

void dsp_write(byte, byte *, dword);

extern byte debug;
