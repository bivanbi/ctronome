#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ctronome.h"
#include "routines.h"
#include "defaults.h"
#include "logging_adapter.h"
#include "program_file_adapter.h"
#include "audio_output_adapter.h"
#include "git_version.h" // generated by CMake from git_version.h.in

int main(int argc, char *argv[]) {
    struct Arguments args;
    struct WavData wav1, wav2;
    struct AudioOutputSettings audio_output_settings;
    struct AudioOutputDevice audio_output_device;

    args = get_arguments(argc, argv);
    log_message(LEVEL_DEBUG, "Starting %s %s\n", MY_NAME, VERSION);
    if (args.help_requested) exit_with_help();
    if (args.version_requested) exit_with_version();

    print_arguments(&args);

    read_wav_file(&wav1, args.wav1_file_path);
    read_wav_file(&wav2, args.wav2_file_path);
    verify_wav_files(&wav1, &wav2);

    audio_output_settings = get_audio_settings(&args, &wav1);
    audio_output_device.settings = &audio_output_settings;

    if (open_audio_output_device(&audio_output_device) == EXIT_FAILURE) {
        log_message(LEVEL_ERROR, "failed to open audio output device\n");
        exit(1);
    }

    log_message(LEVEL_DEBUG, "repeat_count: '%d', finite repetition: '%d'\n", args.repeat_count, args.finite_repetition);

    if (args.is_program) {
        play_program(&audio_output_device, &wav1, &wav2, &args);
    } else {
        play_simple_tact(&audio_output_device, &wav1, &wav2, &args);
    }

    close_audio_output_device(&audio_output_device);
    return (0);
}

struct Arguments get_arguments(int argc, char *argv[]) {
    struct Arguments args = parse_command_line_arguments(argc, argv);

    apply_base_note_defaults(&args);
    apply_beat_per_minute_limits(&args);
    apply_beat_per_tact_limits(&args);

    return args;
}

struct AudioOutputSettings get_audio_settings(struct Arguments *args, struct WavData *wav) {
    struct AudioOutputSettings audio_settings;

    parse_audio_output_driver(&audio_settings, args);

    audio_settings.dsp_device_path = args->dsp_device_path;
    audio_settings.number_of_channels = wav->number_of_channels;
    audio_settings.sample_rate = wav->sample_rate;
    audio_settings.bits_per_sample = wav->bits_per_sample;
    return audio_settings;
}

void parse_audio_output_driver(struct AudioOutputSettings *settings, struct Arguments *args) {
    if (args->audio_output_driver == NULL) {
        settings->auto_select_driver = AUTO_SELECT_ENABLED;
    } else {
        if (get_audio_output_device_by_name(&settings->driver, args->audio_output_driver) != EXIT_SUCCESS) {
            log_message(LEVEL_ERROR, "Unknown audio output driver: '%s'\n", args->audio_output_driver);
            exit(EXIT_FAILURE);
        } else if (is_audio_output_driver_available(settings->driver) != EXIT_SUCCESS) {
            log_message(LEVEL_ERROR, "Audio output driver '%s' support not compiled into this ctronome binary\n",
                        get_audio_output_name(settings->driver));
            exit(EXIT_FAILURE);
        }
        settings->auto_select_driver = AUTO_SELECT_DISABLED;
    }
}

void apply_base_note_defaults(struct Arguments *args) {
    if (!(args->bpt_base_specified)) {
        if (args->bpm_base_specified) {
            args->beat_per_tact[1] = args->beat_per_minute[1];
        } else {
            args->beat_per_tact[1] = DEFAULT_BASE_NOTE;
        }
    }

    if (!(args->bpm_base_specified)) {
        if (args->bpt_base_specified) {
            args->beat_per_minute[1] = args->beat_per_tact[1];
        } else {
            args->beat_per_minute[1] = DEFAULT_BASE_NOTE;
        }
    }
}

void apply_beat_per_minute_limits(struct Arguments *args) {
    if (args->beat_per_minute[0] < MINIMUM_BEAT_PER_MINUTE) {
        log_message(LEVEL_WARNING, "line: %d, beat per minute too low: '%d', setting to minimum: '%d'\n", args->current_program_line, args->beat_per_minute[0],
                    MINIMUM_BEAT_PER_MINUTE);
        args->beat_per_minute[0] = MINIMUM_BEAT_PER_MINUTE;
    }

    if (args->beat_per_minute[0] > MAXIMUM_BEAT_PER_MINUTE) {
        log_message(LEVEL_WARNING, "line: %d, beat per minute too high: '%d', setting to maximum: '%d'\n", args->current_program_line, args->beat_per_minute[0],
                    MAXIMUM_BEAT_PER_MINUTE);
        args->beat_per_minute[0] = MAXIMUM_BEAT_PER_MINUTE;
    }

    if (args->beat_per_minute[1] < BPM_MINIMUM_BASE_NOTE) {
        log_message(LEVEL_WARNING, "line: %d, beat per minute base note too low: '%d', setting to minimum: '%d'\n", args->current_program_line,
                    args->beat_per_minute[1], BPM_MINIMUM_BASE_NOTE);
        args->beat_per_minute[1] = BPM_MINIMUM_BASE_NOTE;
    }

    if (args->beat_per_minute[1] > BPM_MAXIMUM_BASE_NOTE) {
        log_message(LEVEL_WARNING, "line: %d, beat per minute base note too high: '%d', setting to maximum: '%d'\n", args->current_program_line,
                    args->beat_per_minute[1], BPM_MAXIMUM_BASE_NOTE);
        args->beat_per_minute[1] = BPM_MAXIMUM_BASE_NOTE;
    }
}

void apply_beat_per_tact_limits(struct Arguments *args) {
    if (args->beat_per_tact[0] < MINIMUM_BEAT_PER_TACT) {
        log_message(LEVEL_WARNING, "line: %d, beat per tact too low: '%d', setting to minimum: '%d'\n", args->current_program_line, args->beat_per_tact[0],
                    MINIMUM_BEAT_PER_TACT);
        args->beat_per_tact[0] = MINIMUM_BEAT_PER_TACT;
    }

    if (args->beat_per_tact[0] > MAXIMUM_BEAT_PER_TACT) {
        log_message(LEVEL_WARNING, "line: %d, beat per tact too high: '%d', setting to maximum: '%d'\n", args->current_program_line, args->beat_per_tact[0],
                    MAXIMUM_BEAT_PER_TACT);
        args->beat_per_tact[0] = MAXIMUM_BEAT_PER_TACT;
    }

    if (args->beat_per_tact[1] < BPT_MINIMUM_BASE_NOTE) {
        log_message(LEVEL_WARNING, "line: %d, beat per tact base note too low: '%d', setting to minimum: '%d'\n", args->current_program_line,
                    args->beat_per_tact[1], BPT_MINIMUM_BASE_NOTE);
        args->beat_per_tact[1] = BPT_MINIMUM_BASE_NOTE;
    }

    if (args->beat_per_tact[1] > BPT_MAXIMUM_BASE_NOTE) {
        log_message(LEVEL_WARNING, "line: %d, beat per tact base note too high: '%d', setting to maximum: '%d'\n", args->current_program_line,
                    args->beat_per_tact[1], BPT_MAXIMUM_BASE_NOTE);
        args->beat_per_tact[1] = BPT_MAXIMUM_BASE_NOTE;
    }
}

void output_tact(struct AudioOutputDevice *device, struct WavData *wav1, struct WavData *wav2, struct Arguments *args) {
    int wav_sample_size_bytes = wav1->bits_per_sample / 8;
    dword bpm_base_length;
    dword dsp_pattern_length;

    /* calculate the appropriate pattern length for bpm and bpt */
    bpm_base_length = device->settings->sample_rate * wav_sample_size_bytes * device->settings->number_of_channels * 60 / args->beat_per_minute[0];
    dsp_pattern_length = bpm_base_length * args->beat_per_minute[1] / args->beat_per_tact[1];

    while (dsp_pattern_length % (wav_sample_size_bytes * device->settings->number_of_channels)) {
        dsp_pattern_length++;
    }

    for (dword tact_repetitions = 0; tact_repetitions < args->tact_repeat_count; tact_repetitions++) {
        output_to_audio_device(device, wav1->data, dsp_pattern_length); // accented beat
        for (dword remaining_beat_count = args->beat_per_tact[0]; remaining_beat_count > 1; remaining_beat_count--) {
            output_to_audio_device(device, wav2->data, dsp_pattern_length);
        }
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

void play_program(struct AudioOutputDevice *output_device, struct WavData *wav1, struct WavData *wav2, struct Arguments *args) {
    struct ProgramFile program_file;
    struct ProgramLine program_line;

    log_message(LEVEL_DEBUG, "Playing program\n");
    program_file = open_program_file(args->program_file_path);

    while (args->repeat_count > 0) {
        program_line = parse_next_program_line(&program_file);
        if (program_line.is_program_end_reached) {
            if (args->finite_repetition) {
                args->repeat_count--;
                printf("Program end reached, repetitions remaining: %d\n", args->repeat_count);
                if (!args->repeat_count) {
                    break; // we only get notified of program end reached when it already looped to the beginning
                }
            } else {
                printf("Program end reached, repeat endlessly\n");
            }
        }

        apply_program_line(args, &program_line);
        printf("line: %d, tact count: %d, bpm: %d/%d, bpt: %d/%d\n", program_line.line_number,
               args->tact_repeat_count, args->beat_per_minute[0], args->beat_per_minute[1],
               args->beat_per_tact[0], args->beat_per_tact[1]);

        output_tact(output_device, wav1, wav2, args);
    }

    close_program_file(&program_file);
}

void play_simple_tact(struct AudioOutputDevice *output_device, struct WavData *wav1, struct WavData *wav2, struct Arguments *args) {
    while (args->repeat_count > 0) {
        output_tact(output_device, wav1, wav2, args);

        if (args->finite_repetition) {
            args->repeat_count--;
            log_message(LEVEL_DEBUG, "Finite repetition, repetitions remaining: %d\n", args->repeat_count);
        }
    }
}

/**
 * Process the next line of the program file; restart from beginning if reached the end.
 *
 * @param program the file handle to the program file
 */
void apply_program_line(struct Arguments *args, struct ProgramLine *program_line) {
    args->tact_repeat_count = program_line->tact_repeat_count;
    memcpy(args->beat_per_minute, &program_line->beat_per_minute, sizeof(args->beat_per_minute));
    memcpy(args->beat_per_tact, &program_line->beat_per_tact, sizeof(args->beat_per_tact));
    args->bpm_base_specified = program_line->bpm_base_specified;
    args->bpt_base_specified = program_line->bpt_base_specified;
    args->current_program_line = program_line->line_number;

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
    log_message(LEVEL_DEBUG, "Opening WAV file at '%s'\n", wav_file_path);
    wav_file = open_file_for_reading(wav_file_path);

    /* read the header first */
    bytes_read = fread(&wav_header, 1, 44, wav_file);
    if (bytes_read < 44) {
        log_message(LEVEL_ERROR, "wav file %s too short\n", wav_file_path);
        exit(1);
    }

    wav->number_of_channels = *(word *) &wav_header[22];
    wav->sample_rate = *(dword *) &wav_header[24];
    wav->bits_per_sample = *(word *) &wav_header[34];

    log_message(LEVEL_DEBUG, "'%s': channels: '%d', sample rate: '%d', bits per sample: '%d'\n",
                wav_file_path, wav->number_of_channels, wav->sample_rate, wav->bits_per_sample);

    log_message(LEVEL_DEBUG, "'%s': maximum bytes to read: '%d'\n", wav_file_path, MAXIMUM_WAV_DATA_SIZE_BYTES);
    bytes_read = fread(&wav->data, 1, MAXIMUM_WAV_DATA_SIZE_BYTES, wav_file);
    log_message(LEVEL_DEBUG, "'%s': bytes read: '%d'\n", wav_file_path, bytes_read);
    if (bytes_read < 10) {
        log_message(LEVEL_ERROR, "'%s': file too short\n", wav_file_path);
        exit(1);
    }

    fclose(wav_file);
}

void verify_wav_files(struct WavData *wav1, struct WavData *wav2) {
    if ((wav1->number_of_channels != wav2->number_of_channels) ||
        (wav1->bits_per_sample != wav2->bits_per_sample)) {
        log_message(LEVEL_ERROR, "the two WAV files must have the same number of channels and bits per sample\n");
        exit(1);
    }

    if (wav1->sample_rate != wav2->sample_rate) {
        log_message(LEVEL_WARNING, "wav1 and wav2 sample rate differs, may sound funny\n");
    }
}
