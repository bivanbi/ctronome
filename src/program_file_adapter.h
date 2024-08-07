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
static const char newline = 10; /* the newline character */

struct ProgramFile {
    FILE *handle;
    dword current_line_number;
};

struct ProgramLine {
    dword line_number;
    dword tact_repeat_count;
    int beat_per_minute[2];
    int beat_per_tact[2];
    byte bpm_base_specified;
    byte bpt_base_specified;
    byte is_program_end_reached; // will be set to one if reached end of program file and looped
};

# define MAXIMUM_NUMBER_OF_CHARACTER_POSITIONS 10
struct CharacterPositions {
    int positions[MAXIMUM_NUMBER_OF_CHARACTER_POSITIONS];
    int character_count;
};

int get_last_valid_line_position(char *, int);

struct ProgramFile open_program_file(char *);

struct ProgramLine parse_next_program_line(struct ProgramFile *);

struct CharacterPositions get_space_positions_in_buffer(char *, int);

void close_program_file(struct ProgramFile *);

#endif //CTRONOME_PROGRAM_FILE_ADAPTER_H
