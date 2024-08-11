#ifndef CTRONOME_PROGRAM_FILE_ADAPTER_H
#define CTRONOME_PROGRAM_FILE_ADAPTER_H

#include <stdint.h>

typedef uint8_t BYTE;
typedef BYTE byte;
typedef uint32_t DWORD;
typedef DWORD dword;

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

struct CharacterPositions get_space_positions_in_buffer(const char *, int);

void close_program_file(struct ProgramFile *);

void assert_program_file_contains_valid_line(struct ProgramFile *, struct ProgramLine *);

int parse_tact_count(const char *, struct ProgramFile *, struct ProgramLine *);

int parse_beat_per_minute(const char *, struct ProgramFile *, struct ProgramLine *, struct CharacterPositions *);

int parse_bpm_base_note(char *, struct ProgramFile *, struct ProgramLine *, struct CharacterPositions *);

int parse_beat_per_tact(const char *, struct ProgramFile *, struct ProgramLine *, struct CharacterPositions *);

int parse_bpt_base_note(char *, struct ProgramFile *, struct ProgramLine *, struct CharacterPositions *, int);

#endif //CTRONOME_PROGRAM_FILE_ADAPTER_H
