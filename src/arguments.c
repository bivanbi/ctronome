#include <string.h>
#include "arguments.h"
#include "defaults.h"
#include "logging_adapter.h"
#include "routines.h"

struct Arguments parse_command_line_arguments(int argc, char *argv[]) {
    struct Arguments args = get_default_arguments();

    /* first, get the parameters */
    int current_argument, slash_position;

    for (current_argument = 1; current_argument < argc; current_argument++) {
        /* debug */
        if ((strcmp(argv[current_argument], "-debug") == 0) ||
            (strcmp(argv[current_argument], "--debug") == 0)) {
            set_log_level(LEVEL_DEBUG); // set log level as early as possible
            log_message(LEVEL_DEBUG, "set log level to debug\n");
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
            log_message(LEVEL_DEBUG, "wav1: '%s'\n", args.wav1_file_path);
        }

        /* wav2 */
        if ((strcmp(argv[current_argument], "-w2") == 0) && (current_argument + 1 < argc)) {
            args.wav2_file_path = argv[++current_argument];
            log_message(LEVEL_DEBUG, "wav2: '%s'\n", args.wav2_file_path);
        }

        /* dsp device */
        if ((strcmp(argv[current_argument], "-d") == 0) && (current_argument + 1 < argc)) {
            args.dsp_device_path = argv[++current_argument];
            log_message(LEVEL_DEBUG, "dsp: '%s'\n", args.dsp_device_path);
        }

        /* bpt */
        if ((strcmp(argv[current_argument], "-t") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_character_in_buffer(argv[++current_argument], slash);
            args.beat_per_tact[0] = string_to_decimal_int(argv[current_argument]);
            if (slash_position >= 0) {
                args.beat_per_tact[1] = string_to_decimal_int(argv[current_argument] + ++slash_position);
                args.bpt_base_specified = 1;
            }
            log_message(LEVEL_DEBUG, "bpt: '%d'/'%d'\n", args.beat_per_tact[0], args.beat_per_tact[1]);
        }

        /* bpm */
        if ((strcmp(argv[current_argument], "-b") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_character_in_buffer(argv[++current_argument], slash);
            args.beat_per_minute[0] = string_to_decimal_int(argv[current_argument]);
            if (slash_position >= 0) {
                args.beat_per_minute[1] = string_to_decimal_int(argv[current_argument] + ++slash_position);
                args.bpm_base_specified = 1;
            }
            log_message(LEVEL_DEBUG, "bpm: '%d'/'%d'\n", args.beat_per_minute[0], args.beat_per_minute[1]);
        }

        /* repetition count */
        if ((strcmp(argv[current_argument], "-c") == 0) && (current_argument + 1 < argc)) {
            args.repeat_count = string_to_decimal_int(argv[++current_argument]);
            args.finite_repetition = 1;
            log_message(LEVEL_DEBUG, "count: '%d', finite repetition: '%d'\n", args.repeat_count, args.finite_repetition);
        }

        /* program file */
        if ((strcmp(argv[current_argument], "-p") == 0) && (current_argument + 1 < argc)) {
            args.program_file_path = argv[++current_argument];
            args.is_program = 1;
            log_message(LEVEL_DEBUG, "program: '%s'\n", args.program_file_path);
        }
    }

    return args;
}

struct Arguments get_default_arguments() {
    struct Arguments args;
    args.wav1_file_path = WAV1_DEFAULT_FILE_PATH;
    args.wav2_file_path = WAV2_DEFAULT_FILE_PATH;
    args.dsp_device_path = DSP_DEFAULT_DEVICE_PATH;
    args.is_program = 0;
    args.current_program_line = 0;
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
