#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ctronome.h"
#include "routines.h"
#include "dsp.h"
#include "defaults.h"

dword dsp_pattern_length;

struct WavData wav1, wav2;

char *wav1_file_path;
char *wav2_file_path;
char *dsp_device_path;

char *program_file_path;
FILE *program;
byte is_program = 0;

int repeat_count = 1; /* repeat tact/program this many times then exit; 0 means repeat infinite times */
int finite_repetition = 0; /* if repeat count is specified on command line, this will be set to 1, to create a finite loop */

int beat_per_minute[2];
int beat_per_tact[2];

dword bpm_base_length;
int tact_repetition_count = 1; /* repeat tact this many times - each program line sets this accordingly */

struct DspDevice dsp_device;
byte wav_sample_size_bytes;

int bpt_base_specified, bpm_base_specified;

byte debug;

int main(int argc, char *argv[]) {
    set_default_values();
    parse_command_line_arguments(argc, argv);
    read_wav_file(&wav1, wav1_file_path);
    read_wav_file(&wav2, wav2_file_path);
    verify_wav_files();
    open_program_file();
    open_sound_device();

    wav_sample_size_bytes = wav1.bits_per_sample / 8;

    if (debug) printf("debug: repeat_count: '%d', finite repetition: '%d'\n", repeat_count, finite_repetition);


    /* create an endless or finite times loop depending on the value of finite_repetition */
    while (repeat_count > 0) {
        if (is_program) {
            parse_next_program_line(program);
        }

        if (repeat_count > 0) {
            if (is_program)
                printf("count: %d, bpm: %d/%d, bpt: %d/%d\n", tact_repetition_count, beat_per_minute[0], beat_per_minute[1], beat_per_tact[0],
                       beat_per_tact[1]);

            /* calculate the appropriate pattern length for bpm and bpt */
            bpm_base_length = dsp_device.sample_rate * wav_sample_size_bytes * dsp_device.number_of_channels * 60 / beat_per_minute[0];
            dsp_pattern_length = bpm_base_length * beat_per_minute[1] / beat_per_tact[1];

            while (dsp_pattern_length % (wav_sample_size_bytes * dsp_device.number_of_channels)) {
                dsp_pattern_length++;
            }

            for (dword tact_repetitions = 0; tact_repetitions < tact_repetition_count; tact_repetitions++) {
                dsp_write(dsp_device.handler, wav1.data, dsp_pattern_length); // accented beat
                for (dword remaining_beat_count = beat_per_tact[0]; remaining_beat_count > 1; remaining_beat_count--) {
                    dsp_write(dsp_device.handler, wav2.data, dsp_pattern_length);
                }
            }

            if (!(is_program) && (finite_repetition)) {
                repeat_count--;
                if (debug) printf("debug: Finite repetition, repetitions remaining: %d\n", repeat_count);
            }
        }
    }
    dsp_close(dsp_device.handler);
    return (0);
}

void set_default_values() {
    wav1_file_path = (char *) default_wav1_file_path;
    wav2_file_path = (char *) default_wav2_file_path;
    dsp_device_path = (char *) default_dsp_device_path;

    memcpy(&beat_per_minute, &default_beat_per_minute, sizeof(default_beat_per_minute));
    memcpy(&beat_per_tact, &default_beat_per_tact, sizeof(default_beat_per_tact));
}

/**
 * Process the next line of the program file; restart from beginning if reached the end.
 *
 * @param program the file handle to the program file
 */
void parse_next_program_line(FILE *program) {
    char program_read_buffer[8192];
    dword number_of_bytes_read;
    int position_in_current_line;
    int hashmark_position = 0;
    int slash_position;
    /* a normal line should be at least 5 characters long + newline */
    int minimum_line_length = 6;

    /* Skip any line containing a hashmark */
    while (hashmark_position >= 0) {
        while ((number_of_bytes_read = get_next_line(program_read_buffer, program, 8192)) < minimum_line_length) {
            /* if 0 character has been read, seek to the beginning of the file */
            if (number_of_bytes_read < 1) {
                fseek(program, SEEK_SET, 0);
                if (finite_repetition) repeat_count--;
                if (repeat_count) printf("repeat\n");
            }
        }
        hashmark_position = search_character_in_buffer(program_read_buffer, hashmark);
    }
    // read number of repetitions for the tact described in current line
    tact_repetition_count = atoi(program_read_buffer);

    position_in_current_line = search_character_in_buffer(program_read_buffer, space);
    position_in_current_line++;
    position_in_current_line += search_character_in_buffer(&program_read_buffer[position_in_current_line], space);
    bpm_base_specified = bpt_base_specified = 0;

    if (position_in_current_line) {
        slash_position = search_character_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        beat_per_tact[0] = atoi(&program_read_buffer[position_in_current_line]);

        if (slash_position >= 0) {
            beat_per_tact[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            bpt_base_specified = 1;
        }
    }

    position_in_current_line = search_character_in_buffer(program_read_buffer, space);
    if (position_in_current_line) {
        slash_position = search_character_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        beat_per_minute[0] = atoi(&program_read_buffer[position_in_current_line]);
        if (debug)
            printf("debug: program line: bpm: '%d/%d', bpt: '%d/%d', slash position: %d\n",
                   beat_per_minute[0], beat_per_minute[1], beat_per_tact[0], beat_per_tact[1], slash_position);
        if (slash_position >= 0) {
            beat_per_minute[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            bpm_base_specified = 1;
        }
    }

    /* some parameter post-processing */
    if (!(bpt_base_specified)) {
        if (bpm_base_specified) {
            beat_per_tact[1] = beat_per_minute[1];
        } else {
            beat_per_tact[1] = default_base_note;
        }
    }

    if (!(bpm_base_specified)) {
        if (bpt_base_specified) {
            beat_per_minute[1] = beat_per_tact[1];
        } else {
            beat_per_minute[1] = default_base_note;
        }
    }

    if (beat_per_tact[0] < 1) beat_per_tact[0] = 1;
    if (beat_per_tact[0] > 50) beat_per_tact[0] = 50;
    if (beat_per_tact[1] < 1) beat_per_tact[1] = 1;
    if (beat_per_tact[1] > 50) beat_per_tact[1] = 50;

    if (beat_per_minute[0] > 250) beat_per_minute[0] = 250;
    if (beat_per_minute[0] < 30) beat_per_minute[0] = 30;
    if (beat_per_minute[1] > 20) beat_per_minute[1] = 20;
    if (beat_per_minute[1] < 1) beat_per_minute[1] = 1;
}

void parse_command_line_arguments(int argc, char *argv[]) {

    /* first, get the parameters */
    int current_argument, slash_position;

    debug = 0;
    bpm_base_specified = bpt_base_specified = 0;

    for (current_argument = 1; current_argument < argc; current_argument++) {
        /* debug */
        if ((strcmp(argv[current_argument], "-debug") == 0) ||
            (strcmp(argv[current_argument], "--debug") == 0)) {
            printf("debug mode\n");
            debug = 1;
        }
    }

    for (current_argument = 1; current_argument < argc; current_argument++) {
        /* help */
        if ((strcmp(argv[current_argument], "-h") == 0) ||
            (strcmp(argv[current_argument], "--h") == 0) ||
            (strcmp(argv[current_argument], "-help") == 0) ||
            (strcmp(argv[current_argument], "--help") == 0)) {
            printf(HELP);
            exit(0);
        }

        /* version */
        if ((strcmp(argv[current_argument], "-v") == 0) ||
            (strcmp(argv[current_argument], "--v") == 0) ||
            (strcmp(argv[current_argument], "-version") == 0) ||
            (strcmp(argv[current_argument], "--version") == 0)) {
            printf("%s %s\n", MY_NAME, VERSION);
            exit(0);
        }

        /* wav1 */
        if ((strcmp(argv[current_argument], "-w1") == 0) && (current_argument + 1 < argc)) {
            wav1_file_path = argv[++current_argument];
            if (debug) printf("debug: wav1: '%s'\n", wav1_file_path);
        }

        /* wav2 */
        if ((strcmp(argv[current_argument], "-w2") == 0) && (current_argument + 1 < argc)) {
            wav2_file_path = argv[++current_argument];
            if (debug) printf("debug: wav2: '%s'\n", wav2_file_path);
        }

        /* dsp device */
        if ((strcmp(argv[current_argument], "-d") == 0) && (current_argument + 1 < argc)) {
            dsp_device_path = argv[++current_argument];
            if (debug) printf("debug: dsp: '%s'\n", dsp_device_path);
        }

        /* bpt */
        if ((strcmp(argv[current_argument], "-t") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_character_in_buffer(argv[++current_argument], slash);
            beat_per_tact[0] = atoi(argv[current_argument]);
            if (slash_position >= 0) {
                beat_per_tact[1] = atoi(argv[current_argument] + ++slash_position);
                bpt_base_specified = 1;
            }
            if (debug) printf("debug: bpt: '%d'/'%d'\n", beat_per_tact[0], beat_per_tact[1]);
        }

        /* bpm */
        if ((strcmp(argv[current_argument], "-b") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_character_in_buffer(argv[++current_argument], slash);
            beat_per_minute[0] = atoi(argv[current_argument]);
            if (slash_position >= 0) {
                beat_per_minute[1] = atoi(argv[current_argument] + ++slash_position);
                bpm_base_specified = 1;
            }
            if (debug) printf("debug: bpm: '%d'/'%d'\n", beat_per_minute[0], beat_per_minute[1]);
        }

        /* repetition count */
        if ((strcmp(argv[current_argument], "-c") == 0) && (current_argument + 1 < argc)) {
            repeat_count = atoi(argv[++current_argument]);
            finite_repetition = 1;
            if (debug) printf("debug: count: '%d', finite repetition: '%d'\n", repeat_count, finite_repetition);
        }

        /* program file */
        if ((strcmp(argv[current_argument], "-p") == 0) && (current_argument + 1 < argc)) {
            program_file_path = argv[++current_argument];
            is_program = 1;
            if (debug) printf("debug: program: '%s'\n", program_file_path);
        }
    }

    /* some parameter post-processing */
    if (!(bpt_base_specified)) {
        if (bpm_base_specified) {
            beat_per_tact[1] = beat_per_minute[1];
        } else {
            beat_per_tact[1] = default_base_note;
        }
    }

    if (!(bpm_base_specified)) {
        if (bpt_base_specified) {
            beat_per_minute[1] = beat_per_tact[1];
        } else {
            beat_per_minute[1] = default_base_note;
        }
    }

    if (beat_per_minute[0] > 250) beat_per_minute[0] = 250;
    if (beat_per_minute[0] < 30) beat_per_minute[0] = 30;
    if (beat_per_minute[1] > 20) beat_per_minute[1] = 20;
    if (beat_per_minute[1] < 1) beat_per_minute[1] = 1;

    if (beat_per_tact[0] < 1) beat_per_tact[0] = 1;
    if (beat_per_tact[0] > 50) beat_per_tact[0] = 50;
    if (beat_per_tact[1] < 1) beat_per_tact[1] = 1;
    if (beat_per_tact[1] > 50) beat_per_tact[1] = 50;

    printf("bpm: %d/%d, bpt: %d/%d", beat_per_minute[0], beat_per_minute[1], beat_per_tact[0], beat_per_tact[1]);
    if (finite_repetition) printf(", repeat count: %d", repeat_count);
    if (is_program) printf("\nprogram file: %s", program_file_path);
    printf("\n");
}

void read_wav_file(struct WavData *wav, char *wav_file_path) {
    FILE *wav_file;
    dword bytes_read;
    byte wav_header[100];

    /* cleanup buffer */
    for (dword buffer_position = 0; buffer_position < MAXIMUM_WAV_DATA_SIZE_BYTES; buffer_position++) {
        wav->data[buffer_position] = 0;
    }

    /* open wav file */
    if (debug) printf("debug: Opening WAV file at '%s'\n", wav_file_path);
    wav_file = open_file_for_reading(wav_file_path);

    /* read the header first */
    bytes_read = fread(&wav_header, 1, 44, wav_file);
    if (bytes_read < 44) {
        printf("wav file %s too short\n", wav_file_path);
        exit(1);
    }

    wav->number_of_channels = *(word *) &wav_header[22];
    wav->sample_rate = *(dword *) &wav_header[24];
    wav->bits_per_sample = *(word *) &wav_header[34];

    if (debug)
        printf("debug: '%s': channels: '%d', sample rate: '%d', bits per sample: '%d'\n",
               wav_file_path, wav->number_of_channels, wav->sample_rate, wav->bits_per_sample);

    if (debug) printf("debug: '%s': maximum bytes to read: '%d'\n", wav_file_path, MAXIMUM_WAV_DATA_SIZE_BYTES);
    bytes_read = fread(&wav->data, 1, MAXIMUM_WAV_DATA_SIZE_BYTES, wav_file);
    if (debug) printf("debug: '%s': bytes read: '%d'\n", wav_file_path, bytes_read);
    if (bytes_read < 10) {
        printf("ERROR: '%s': file too short\n", wav_file_path);
        exit(1);
    }

    fclose(wav_file);
}

void verify_wav_files() {
    if ((wav1.number_of_channels != wav2.number_of_channels) ||
        (wav1.bits_per_sample != wav2.bits_per_sample)) {
        printf("ERROR: the two WAV files must have the same number of channels and bits per sample\n");
        exit(1);
    }

    if (wav1.sample_rate != wav2.sample_rate) {
        printf("warning: wav1 and wav2 sample rate differs, may sound funny\n");
    }
}

void open_program_file() {
    if (is_program) {
        if (debug) printf("debug: opening program file: '%s'\n", program_file_path);
        program = open_file_for_reading(program_file_path);
    }
}

void open_sound_device() {
    if (debug) printf("debug: calling dsp_init(%s)\n", dsp_device_path);

    dsp_device = dsp_init(dsp_device_path, wav1.bits_per_sample, wav1.number_of_channels, wav1.sample_rate);

    if (debug)
        printf("debug: dsp channels: '%d', sample rate: '%d', bits per sample: '%d'\n",
               dsp_device.number_of_channels, dsp_device.sample_rate, dsp_device.dsp_format);
}
