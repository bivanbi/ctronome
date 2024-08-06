#ifndef CTRONOME_PROGRAM_FILE_ADAPTER_H
#define CTRONOME_PROGRAM_FILE_ADAPTER_H

#include <stdint.h>

typedef uint8_t BYTE;
typedef BYTE byte;
typedef uint32_t DWORD;
typedef DWORD dword;

static const char slash = 47; /* the / character */
static const char hashmark = 35; /* the # character */
static const char space = 32; /* the   character */

struct ProgramFile {
    FILE *handle;
};

struct ProgramLine {
    dword tact_repeat_count;
    int beat_per_minute[2];
    int beat_per_tact[2];
    byte bpm_base_specified;
    byte bpt_base_specified;
    byte is_program_end_reached; // will be set to one if reached end of program file and looped
};

struct ProgramFile open_program_file(char *);

struct ProgramLine parse_next_program_line(struct ProgramFile *);

void close_program_file(struct ProgramFile *);

#endif //CTRONOME_PROGRAM_FILE_ADAPTER_H
