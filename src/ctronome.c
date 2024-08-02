#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ctronome.h"
#include "routines.h"
#include "dsp.h"
#include "defaults.h"

byte debug;

int main(int argc, char *argv[]) {
    FILE *program = NULL;
    byte wav_sample_size_bytes;
    dword dsp_pattern_length;
    struct DspDevice dsp_device;
    struct Arguments args;
    struct WavData wav1, wav2;

    args = parse_command_line_arguments(argc, argv);
    if (args.help_requested) exit_with_help();
    if (args.version_requested) exit_with_version();

    print_arguments(&args);

    dword bpm_base_length;

    read_wav_file(&wav1, args.wav1_file_path);
    read_wav_file(&wav2, args.wav2_file_path);
    verify_wav_files(&wav1, &wav2);
    if (args.is_program) program = open_program_file(args.program_file_path);

    dsp_device = open_sound_device(args.dsp_device_path, &wav1);

    wav_sample_size_bytes = wav1.bits_per_sample / 8;

    if (debug) printf("debug: repeat_count: '%d', finite repetition: '%d'\n", args.repeat_count, args.finite_repetition);

    /* create an endless or finite times loop depending on the value of finite_repetition */
    while (args.repeat_count > 0) {
        if (args.is_program) {
            parse_next_program_line(&args, program);
        }

        if (args.repeat_count > 0) {
            if (args.is_program)
                printf("count: %d, bpm: %d/%d, bpt: %d/%d\n", args.tact_repeat_count, args.beat_per_minute[0], args.beat_per_minute[1],
                       args.beat_per_tact[0], args.beat_per_tact[1]);

            /* calculate the appropriate pattern length for bpm and bpt */
            bpm_base_length = dsp_device.sample_rate * wav_sample_size_bytes * dsp_device.number_of_channels * 60 / args.beat_per_minute[0];
            dsp_pattern_length = bpm_base_length * args.beat_per_minute[1] / args.beat_per_tact[1];

            while (dsp_pattern_length % (wav_sample_size_bytes * dsp_device.number_of_channels)) {
                dsp_pattern_length++;
            }

            for (dword tact_repetitions = 0; tact_repetitions < args.tact_repeat_count; tact_repetitions++) {
                dsp_write(dsp_device.handler, wav1.data, dsp_pattern_length); // accented beat
                for (dword remaining_beat_count = args.beat_per_tact[0]; remaining_beat_count > 1; remaining_beat_count--) {
                    dsp_write(dsp_device.handler, wav2.data, dsp_pattern_length);
                }
            }

            if (!(args.is_program) && (args.finite_repetition)) {
                args.repeat_count--;
                if (debug) printf("debug: Finite repetition, repetitions remaining: %d\n", args.repeat_count);
            }
        }
    }
    dsp_close(dsp_device.handler);
    return (0);
}

struct Arguments parse_command_line_arguments(int argc, char *argv[]) {
    struct Arguments args = get_default_arguments();

    /* first, get the parameters */
    int current_argument, slash_position;

    debug = 0;

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
            args.help_requested = 1;
        }

        /* version */
        if ((strcmp(argv[current_argument], "-v") == 0) ||
            (strcmp(argv[current_argument], "--v") == 0) ||
            (strcmp(argv[current_argument], "-version") == 0) ||
            (strcmp(argv[current_argument], "--version") == 0)) {
            args.version_requested = 1;
        }

        /* wav1 */
        if ((strcmp(argv[current_argument], "-w1") == 0) && (current_argument + 1 < argc)) {
            args.wav1_file_path = argv[++current_argument];
            if (debug) printf("debug: wav1: '%s'\n", args.wav1_file_path);
        }

        /* wav2 */
        if ((strcmp(argv[current_argument], "-w2") == 0) && (current_argument + 1 < argc)) {
            args.wav2_file_path = argv[++current_argument];
            if (debug) printf("debug: wav2: '%s'\n", args.wav2_file_path);
        }

        /* dsp device */
        if ((strcmp(argv[current_argument], "-d") == 0) && (current_argument + 1 < argc)) {
            args.dsp_device_path = argv[++current_argument];
            if (debug) printf("debug: dsp: '%s'\n", args.dsp_device_path);
        }

        /* bpt */
        if ((strcmp(argv[current_argument], "-t") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_character_in_buffer(argv[++current_argument], slash);
            args.beat_per_tact[0] = atoi(argv[current_argument]);
            if (slash_position >= 0) {
                args.beat_per_tact[1] = atoi(argv[current_argument] + ++slash_position);
                args.bpt_base_specified = 1;
            }
            if (debug) printf("debug: bpt: '%d'/'%d'\n", args.beat_per_tact[0], args.beat_per_tact[1]);
        }

        /* bpm */
        if ((strcmp(argv[current_argument], "-b") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_character_in_buffer(argv[++current_argument], slash);
            args.beat_per_minute[0] = atoi(argv[current_argument]);
            if (slash_position >= 0) {
                args.beat_per_minute[1] = atoi(argv[current_argument] + ++slash_position);
                args.bpm_base_specified = 1;
            }
            if (debug) printf("debug: bpm: '%d'/'%d'\n", args.beat_per_minute[0], args.beat_per_minute[1]);
        }

        /* repetition count */
        if ((strcmp(argv[current_argument], "-c") == 0) && (current_argument + 1 < argc)) {
            args.repeat_count = atoi(argv[++current_argument]);
            args.finite_repetition = 1;
            if (debug) printf("debug: count: '%d', finite repetition: '%d'\n", args.repeat_count, args.finite_repetition);
        }

        /* program file */
        if ((strcmp(argv[current_argument], "-p") == 0) && (current_argument + 1 < argc)) {
            args.program_file_path = argv[++current_argument];
            args.is_program = 1;
            if (debug) printf("debug: program: '%s'\n", args.program_file_path);
        }
    }

    apply_base_note_defaults(&args);
    apply_beat_per_minute_limits(&args);
    apply_beat_per_tact_limits(&args);

    return args;
}

struct Arguments get_default_arguments() {
    struct Arguments args;
    args.wav1_file_path = (char *) default_wav1_file_path;
    args.wav2_file_path = (char *) default_wav2_file_path;
    args.dsp_device_path = (char *) default_dsp_device_path;
    args.is_program = 0;
    args.finite_repetition = 0; // by default, loop endlessly
    args.repeat_count = 1;
    args.tact_repeat_count = 1;
    args.help_requested = 0;
    args.version_requested = 0;
    args.bpm_base_specified = 0;
    args.bpt_base_specified = 0;

    memcpy(&args.beat_per_minute, &default_beat_per_minute, sizeof(default_beat_per_minute));
    memcpy(&args.beat_per_tact, &default_beat_per_tact, sizeof(default_beat_per_tact));

    return args;
}

void apply_base_note_defaults(struct Arguments *args) {
    if (!(args->bpt_base_specified)) {
        if (args->bpm_base_specified) {
            args->beat_per_tact[1] = args->beat_per_minute[1];
        } else {
            args->beat_per_tact[1] = default_base_note;
        }
    }

    if (!(args->bpm_base_specified)) {
        if (args->bpt_base_specified) {
            args->beat_per_minute[1] = args->beat_per_tact[1];
        } else {
            args->beat_per_minute[1] = default_base_note;
        }
    }
}

void apply_beat_per_minute_limits(struct Arguments *args) {
    if (args->beat_per_minute[0] < MINIMUM_BEAT_PER_MINUTE) {
        printf("warning: beat per minute too low: '%d', setting to minimum: '%d'\n", args->beat_per_minute[0], MINIMUM_BEAT_PER_MINUTE);
        args->beat_per_minute[0] = MINIMUM_BEAT_PER_MINUTE;
    }

    if (args->beat_per_minute[0] > MAXIMUM_BEAT_PER_MINUTE) {
        printf("warning: beat per minute too high: '%d', setting to maximum: '%d'\n", args->beat_per_minute[0], MAXIMUM_BEAT_PER_MINUTE);
        args->beat_per_minute[0] = MAXIMUM_BEAT_PER_MINUTE;
    }

    if (args->beat_per_minute[1] < BPM_MINIMUM_BASE_NOTE) {
        printf("warning: beat per minute base note too low: '%d', setting to minimum: '%d'\n", args->beat_per_minute[1], BPM_MINIMUM_BASE_NOTE);
        args->beat_per_minute[1] = BPM_MINIMUM_BASE_NOTE;
    }

    if (args->beat_per_minute[1] > BPM_MAXIMUM_BASE_NOTE) {
        printf("warning: beat per minute base note too high: '%d', setting to maximum: '%d'\n", args->beat_per_minute[1], BPM_MAXIMUM_BASE_NOTE);
        args->beat_per_minute[1] = BPM_MAXIMUM_BASE_NOTE;
    }
}

void apply_beat_per_tact_limits(struct Arguments *args) {
    if (args->beat_per_tact[0] < MINIMUM_BEAT_PER_TACT) {
        printf("warning: beat per tact too low: '%d', setting to minimum: '%d'\n", args->beat_per_tact[0], MINIMUM_BEAT_PER_TACT);
        args->beat_per_tact[0] = MINIMUM_BEAT_PER_TACT;
    }

    if (args->beat_per_tact[0] > MAXIMUM_BEAT_PER_TACT) {
        printf("warning: beat per tact too high: '%d', setting to maximum: '%d'\n", args->beat_per_tact[0], MAXIMUM_BEAT_PER_TACT);
        args->beat_per_tact[0] = MAXIMUM_BEAT_PER_TACT;
    }

    if (args->beat_per_tact[1] < BPT_MINIMUM_BASE_NOTE) {
        printf("warning: beat per tact base note too low: '%d', setting to minimum: '%d'\n", args->beat_per_tact[1], BPT_MINIMUM_BASE_NOTE);
        args->beat_per_tact[1] = BPT_MINIMUM_BASE_NOTE;
    }

    if (args->beat_per_tact[1] > BPT_MAXIMUM_BASE_NOTE) {
        printf("warning: beat per tact base note too high: '%d', setting to maximum: '%d'\n", args->beat_per_tact[1], BPT_MAXIMUM_BASE_NOTE);
        args->beat_per_tact[1] = BPT_MAXIMUM_BASE_NOTE;
    }
}

void exit_with_help() {
    printf(HELP);
    exit(0);
}

void exit_with_version() {
    printf("%s %s\n", MY_NAME, VERSION);
    exit(0);
}

void print_arguments(struct Arguments *args) {
    printf("bpm: %d/%d, bpt: %d/%d", args->beat_per_minute[0], args->beat_per_minute[1],
           args->beat_per_tact[0], args->beat_per_tact[1]);
    if (args->finite_repetition) {
        printf(", repeat count: %d", args->repeat_count);
    } else {
        printf(", repeat count: infinite");
    }

    if (args->is_program) {
        printf(", program file: %s", args->program_file_path);
    } else {
        printf(", no program file set");
    }
    printf("\n");
}

/**
 * Process the next line of the program file; restart from beginning if reached the end.
 *
 * @param program the file handle to the program file
 */
void parse_next_program_line(struct Arguments *args, FILE *program) {
    char program_read_buffer[8192];
    dword number_of_bytes_read;
    int position_in_current_line;
    int hashmark_position = 0;
    int slash_position;
    /* a normal line should be at least 5 characters long + newline */
    int minimum_line_length = 6;

    args->bpm_base_specified = 0;
    args->bpt_base_specified = 0;

    /* Skip any line containing a hashmark */
    while (hashmark_position >= 0) {
        while ((number_of_bytes_read = get_next_line(program_read_buffer, program, 8192)) < minimum_line_length) {
            /* if 0 character has been read, seek to the beginning of the file */
            if (number_of_bytes_read < 1) {
                fseek(program, SEEK_SET, 0);
                if (args->finite_repetition) args->repeat_count--;
                if (args->repeat_count) printf("repeat\n");
            }
        }
        hashmark_position = search_character_in_buffer(program_read_buffer, hashmark);
    }
    // read number of repetitions for the tact described in current line
    args->tact_repeat_count = atoi(program_read_buffer);

    position_in_current_line = search_character_in_buffer(program_read_buffer, space);
    if (position_in_current_line) {
        slash_position = search_character_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        args->beat_per_minute[0] = atoi(&program_read_buffer[position_in_current_line]);
        if (debug)
            printf("debug: program line: bpm: '%d/%d', bpt: '%d/%d', slash position: %d\n",
                   args->beat_per_minute[0], args->beat_per_minute[1], args->beat_per_tact[0], args->beat_per_tact[1], slash_position);
        if (slash_position >= 0) {
            args->beat_per_minute[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            args->bpm_base_specified = 1;
        }
    }

    position_in_current_line += search_character_in_buffer(&program_read_buffer[position_in_current_line], space);
    if (position_in_current_line) {
        slash_position = search_character_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        args->beat_per_tact[0] = atoi(&program_read_buffer[position_in_current_line]);

        if (slash_position >= 0) {
            args->beat_per_tact[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            args->bpt_base_specified = 1;
        }
    }

    apply_base_note_defaults(args);
    apply_beat_per_minute_limits(args);
    apply_beat_per_tact_limits(args);
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

void verify_wav_files(struct WavData *wav1, struct WavData *wav2) {
    if ((wav1->number_of_channels != wav2->number_of_channels) ||
        (wav1->bits_per_sample != wav2->bits_per_sample)) {
        printf("ERROR: the two WAV files must have the same number of channels and bits per sample\n");
        exit(1);
    }

    if (wav1->sample_rate != wav2->sample_rate) {
        printf("warning: wav1 and wav2 sample rate differs, may sound funny\n");
    }
}

FILE *open_program_file(char *program_file_path) {
    if (debug) printf("debug: opening program file: '%s'\n", program_file_path);
    return open_file_for_reading(program_file_path);
}

struct DspDevice open_sound_device(char *dsp_device_path, struct WavData *wav) {
    if (debug) printf("debug: calling dsp_init(%s)\n", dsp_device_path);

    struct DspDevice dsp_device = dsp_init(dsp_device_path, wav->bits_per_sample, wav->number_of_channels, wav->sample_rate);

    if (debug)
        printf("debug: dsp channels: '%d', sample rate: '%d', bits per sample: '%d'\n",
               dsp_device.number_of_channels, dsp_device.sample_rate, dsp_device.dsp_format);

    return dsp_device;
}
