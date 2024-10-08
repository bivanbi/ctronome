#include <stdio.h>
#include <stdlib.h>
#include "defaults.h"
#include "program_file_adapter.h"
#include "routines.h"
#include "logging_adapter.h"

#define PROGRAM_LINE_ELEMENT_COUNT 3 // tact repeat count, beat per minute, beat per tact
#define PROGRAM_LINE_MINIMUM_LENGTH 5 // 3 elements + 2 spaces. Newline is not counted.

struct ProgramFile open_program_file(char *filename) {
    struct ProgramFile program_file;
    program_file.handle = open_file_for_reading(filename);
    program_file.current_line_number = 0;
    return program_file;
}

struct ProgramLine parse_next_program_line(struct ProgramFile *program_file) {
    struct ProgramLine program_line;
    program_line.beat_per_minute[0] = 0;
    program_line.beat_per_minute[1] = 0;
    program_line.beat_per_tact[0] = 0;
    program_line.beat_per_tact[1] = 0;
    program_line.bpm_base_specified = 0;
    program_line.bpt_base_specified = 0;
    program_line.is_program_end_reached = 0;

    char program_line_read_buffer[8192];
    int number_of_bytes_read;
    int last_valid_position;
    struct CharacterPositions space_positions;

    /* a normal line should be at least 5 characters long + newline */
    byte valid_program_line_found = 0;

    while (!valid_program_line_found) {
        program_file->current_line_number++;
        number_of_bytes_read = (int) get_next_line(program_line_read_buffer, program_file->handle, 8192);

        if (number_of_bytes_read < 1) {
            assert_program_file_contains_valid_line(program_file, &program_line);

            log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: reached end of file, seeking to the beginning\n",
                        program_file->current_line_number);
            fseek(program_file->handle, SEEK_SET, 0);
            program_file->current_line_number = 0;
            program_line.is_program_end_reached = 1;

            continue;
        }

        last_valid_position = get_last_valid_line_position(program_line_read_buffer, number_of_bytes_read);
        if (last_valid_position < PROGRAM_LINE_MINIMUM_LENGTH - 1) { // position is counted from 0; positions from 0 to 4 = 5 characters
            log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: last valid position %d smaller than %d, skip\n",
                        program_file->current_line_number, last_valid_position, PROGRAM_LINE_MINIMUM_LENGTH);
            continue;
        }

        log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: bytes read: %d, last_valid_position: %d\n",
                    program_file->current_line_number, number_of_bytes_read, last_valid_position);

        space_positions = get_space_positions_in_buffer(program_line_read_buffer, last_valid_position);
        log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: space count: %d\n",
                    program_file->current_line_number, space_positions.character_count);

        if (space_positions.character_count < PROGRAM_LINE_ELEMENT_COUNT - 1) { // 3 elements -> 2 spaces minimum
            log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: not enough space separated values for a valid program line, skip\n",
                        program_file->current_line_number);
            continue;
        } else if ((space_positions.character_count == PROGRAM_LINE_ELEMENT_COUNT) && (last_valid_position > space_positions.positions[PROGRAM_LINE_ELEMENT_COUNT - 1])) {
            last_valid_position = space_positions.positions[PROGRAM_LINE_ELEMENT_COUNT - 1] - 1;
            log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: have excess spaces, adjusted last_valid_position to %d\n",
                        program_file->current_line_number, last_valid_position);
        }

        if (EXIT_SUCCESS != parse_tact_count(program_line_read_buffer, program_file, &program_line)) continue;
        if (EXIT_SUCCESS != parse_beat_per_minute(program_line_read_buffer, program_file, &program_line, &space_positions)) continue;
        if (EXIT_SUCCESS != parse_bpm_base_note(program_line_read_buffer, program_file, &program_line, &space_positions)) continue;
        if (EXIT_SUCCESS != parse_beat_per_tact(program_line_read_buffer, program_file, &program_line, &space_positions)) continue;
        if (EXIT_SUCCESS != parse_bpt_base_note(program_line_read_buffer, program_file, &program_line, &space_positions, last_valid_position)) continue;

        valid_program_line_found = 1;
    }

    program_line.line_number = program_file->current_line_number;

    log_message(LEVEL_DEBUG, "program line number: %d, bpm: '%d/%d', bpm base note specified: %d, bpt: '%d/%d', bpt base note specified: %d\n",
                program_line.line_number,
                program_line.beat_per_minute[0], program_line.beat_per_minute[1], program_line.bpm_base_specified,
                program_line.beat_per_tact[0], program_line.beat_per_tact[1], program_line.bpt_base_specified);

    return program_line;
}

int get_last_valid_line_position(char *line_buffer, int bytes_read) {
    int newline_position = search_character_in_buffer(line_buffer, newline);
    int hashmark_position = search_character_in_buffer(line_buffer, hashmark);
    int last_valid_position = bytes_read;

    if (newline_position >= 0) {
        last_valid_position = newline_position - 1;
    }

    if (hashmark_position < 0) { // there is no hashmark
        return last_valid_position;
    }

    return (hashmark_position < last_valid_position) ? hashmark_position : last_valid_position;
}

struct CharacterPositions get_space_positions_in_buffer(const char *buffer, int last_valid_position) {
    struct CharacterPositions space_positions;
    space_positions.character_count = 0;
    log_message(LEVEL_DEBUG, "get_space_positions_in_buffer: last valid position: %d\n", last_valid_position);

    for (int i = 0; i <= last_valid_position; i++) {
        if (buffer[i] == space) {
            log_message(LEVEL_DEBUG, "get_space_positions_in_buffer: space found at position %d\n", i);
            space_positions.positions[space_positions.character_count++] = i;
            if (space_positions.character_count == MAXIMUM_NUMBER_OF_CHARACTER_POSITIONS) {
                log_message(LEVEL_DEBUG, "get_space_positions_in_buffer: maximum number of space positions reached\n");
                break;
            }
        } else if (space_positions.character_count >= MAXIMUM_NUMBER_OF_CHARACTER_POSITIONS) {
            log_message(LEVEL_DEBUG, "get_space_positions_in_buffer: reached end of valid string\n");
            break;
        }
    }

    return space_positions;
}

void assert_program_file_contains_valid_line(struct ProgramFile *program_file, struct ProgramLine *program_line) {
    if (program_line->is_program_end_reached) {
        log_message(LEVEL_ERROR, "assert_program_file_contains_valid_line: line %d: no valid program line found in file\n",
                    program_file->current_line_number);
        exit(EXIT_FAILURE);
    }
}

int parse_tact_count(const char *line_buffer, struct ProgramFile *program_file, struct ProgramLine *program_line) {
    int tact_repeat_count = string_to_decimal_int(line_buffer);
    if (tact_repeat_count < 1) {
        log_message(LEVEL_WARNING, "line %d: tact repeat count must be an integer larger than 0\n",
                    program_file->current_line_number);
        return EXIT_FAILURE;
    }

    program_line->tact_repeat_count = tact_repeat_count;
    return EXIT_SUCCESS;
}

int parse_beat_per_minute(const char *line_buffer, struct ProgramFile *program_file, struct ProgramLine *program_line, struct CharacterPositions *space_positions) {
    program_line->beat_per_minute[0] = string_to_decimal_int(&line_buffer[space_positions->positions[0] + 1]);
    log_message(LEVEL_DEBUG, "line %d: beat per minute: %d\n",
                program_file->current_line_number, program_line->beat_per_minute[0]);

    if (program_line->beat_per_minute[0] < 1) {
        log_message(LEVEL_WARNING, "line %d: beat per minute must be an integer larger than 0\n",
                    program_file->current_line_number);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int parse_bpm_base_note(char *line_buffer, struct ProgramFile *program_file, struct ProgramLine *program_line, struct CharacterPositions *space_positions) {
    int slash_position = search_character_in_range(line_buffer, slash, space_positions->positions[0], space_positions->positions[1]);
    log_message(LEVEL_DEBUG, "line %d: BPM: slash position: %d, next space position: %d\n",
                program_file->current_line_number, slash_position, space_positions->positions[1]);

    if (slash_position < 0) {
        log_message(LEVEL_DEBUG, "line %d: BPM: base not not specified\n",
                    program_file->current_line_number);
    } else {
        program_line->beat_per_minute[1] = string_to_decimal_int(&line_buffer[slash_position + 1]);
        log_message(LEVEL_DEBUG, "line %d: beat per minute base note: %d\n",
                    program_file->current_line_number, program_line->beat_per_minute[1]);
        if (program_line->beat_per_minute[1] < 1) {
            log_message(LEVEL_WARNING, "line %d: beat per minute base note, if specified, must be an integer larger than 0\n",
                        program_file->current_line_number);
            return EXIT_FAILURE;
        }

        program_line->bpm_base_specified = 1;
    }

    return EXIT_SUCCESS;
}

int parse_beat_per_tact(const char *line_buffer, struct ProgramFile *program_file, struct ProgramLine *program_line, struct CharacterPositions *space_positions) {
    program_line->beat_per_tact[0] = string_to_decimal_int(&line_buffer[space_positions->positions[1] + 1]);
    log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: beat per tact: %d\n",
                program_file->current_line_number, program_line->beat_per_tact[0]);

    if (program_line->beat_per_tact[0] < 1) {
        log_message(LEVEL_WARNING, "parse_next_program_line: line %d: beat per tact must be an integer larger than 0\n",
                    program_file->current_line_number);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int parse_bpt_base_note(char *line_buffer, struct ProgramFile *program_file, struct ProgramLine *program_line, struct CharacterPositions *space_positions,
                        int last_valid_position) {
    int slash_position = search_character_in_range(line_buffer, slash, space_positions->positions[1], last_valid_position);
    log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: BPT: slash position: %d, last valid position: %d\n",
                program_file->current_line_number, slash_position, last_valid_position);

    if (slash_position < 0) {
        log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: BPT: base note not specified\n",
                    program_file->current_line_number);
    } else {
        program_line->beat_per_tact[1] = string_to_decimal_int(&line_buffer[slash_position + 1]);
        log_message(LEVEL_DEBUG, "parse_next_program_line: line %d: beat per tact base note: %d\n",
                    program_file->current_line_number, program_line->beat_per_tact[1]);
        if (program_line->beat_per_tact[1] < 1) {
            log_message(LEVEL_WARNING, "parse_next_program_line: line %3d: beat per tact base note, if specified, must be an integer larger than 0\n",
                        program_file->current_line_number);
            return EXIT_FAILURE;
        }

        program_line->bpt_base_specified = 1;
    }

    return EXIT_SUCCESS;
}

void close_program_file(struct ProgramFile *program_file) {
    fclose(program_file->handle);
}
