#include <stdio.h>
#include <stdlib.h>
#include "program_file_adapter.h"
#include "routines.h"
#include "logging_adapter.h"

struct ProgramFile open_program_file(char *filename) {
    struct ProgramFile program_file;
    program_file.handle = open_file_for_reading(filename);
    return program_file;
}

struct ProgramLine parse_next_program_line(struct ProgramFile *program_file) {
    struct ProgramLine program_line;
    program_line.is_program_end_reached = 0;
    program_line.bpm_base_specified = 0;
    program_line.bpt_base_specified = 0;

    char program_read_buffer[8192];
    dword number_of_bytes_read;
    int position_in_current_line;
    int hashmark_position = 0;
    int slash_position;
    /* a normal line should be at least 5 characters long + newline */
    int minimum_line_length = 6;

    /* Skip any line containing a hashmark */
    while (hashmark_position >= 0) {
        while ((number_of_bytes_read = get_next_line(program_read_buffer, program_file->handle, 8192)) < minimum_line_length) {
            /* if 0 character has been read, seek to the beginning of the file */
            if (number_of_bytes_read < 1) {
                fseek(program_file->handle, SEEK_SET, 0);
                program_line.is_program_end_reached = 1;
            }
        }
        hashmark_position = search_character_in_buffer(program_read_buffer, hashmark);
    }
    // read number of repetitions for the tact described in current line
    program_line.tact_repeat_count = atoi(program_read_buffer);

    position_in_current_line = search_character_in_buffer(program_read_buffer, space);
    if (position_in_current_line) {
        slash_position = search_character_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        program_line.beat_per_minute[0] = atoi(&program_read_buffer[position_in_current_line]);

        if (slash_position >= 0) {
            program_line.beat_per_minute[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            program_line.bpm_base_specified = 1;
        }
    }

    position_in_current_line += search_character_in_buffer(&program_read_buffer[position_in_current_line], space);
    if (position_in_current_line) {
        slash_position = search_character_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        program_line.beat_per_tact[0] = atoi(&program_read_buffer[position_in_current_line]);

        if (slash_position >= 0) {
            program_line.beat_per_tact[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            program_line.bpt_base_specified = 1;
        }
    }

    log_message(LEVEL_DEBUG, "program line: bpm: '%d/%d', bpm base note specified: %d, bpt: '%d/%d', bpt base note specified: %d\n",
                program_line.beat_per_minute[0], program_line.beat_per_minute[1], program_line.bpm_base_specified,
                program_line.beat_per_tact[0], program_line.beat_per_tact[1], program_line.bpt_base_specified);

    return program_line;
}

void close_program_file(struct ProgramFile *program_file) {
    fclose(program_file->handle);
}
