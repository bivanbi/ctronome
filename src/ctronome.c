#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ctronome.h"
#include "routines.h"
#include "dsp.h"

dword dsp_pattern_length;

struct wav_data {
    byte data[1000000];
    word number_of_channels;
    dword sample_rate;
    word bits_per_sample;
};

struct wav_data wav1, wav2;

dword wav_bytes_to_read;

byte wav_header[100];

FILE *program;

byte is_program;
dword bpm_base_length;
int tact_repetition_count = 1; /* tact counter */

struct dsp_device dsp_device;
byte dsp_depth;

int bpt_base_specified, bpm_base_specified;

byte debug;

int main(int argc, char *argv[]) {
    parm_init(argc, argv);

    /* create an endless or pcount times loop */
    while (pcount > 0) {
        if (is_program) {
            next_program(program);
        }

        if (pcount > 0) {
            if (is_program) printf("count: %d, bpm: %d/%d, bpt: %d/%d\n", tact_repetition_count, bpm[0], bpm[1], bpt[0], bpt[1]);

            /* calculate the appropriate pattern length for bpm and bpt */
            bpm_base_length = dsp_device.sample_rate * dsp_depth * dsp_device.number_of_channels * 60 / bpm[0];
            dsp_pattern_length = bpm_base_length * bpm[1] / bpt[1];

            while (dsp_pattern_length % (dsp_depth * dsp_device.number_of_channels)) {
                dsp_pattern_length++;
            }

            for (dword tact_repetitions = 0; tact_repetitions < tact_repetition_count; tact_repetitions++) {
                dsp_write(dsp_device.handler, wav1.data, dsp_pattern_length); // accented beat
                for (dword remaining_beat_count = bpt[0]; remaining_beat_count > 1; remaining_beat_count--) {
                    dsp_write(dsp_device.handler, wav2.data, dsp_pattern_length);
                }
            }
            if (!(is_program)) pcount -= pdecrease;
        }
    }
    dsp_close(dsp_device.handler);
    return (0);
}

/**
 * Process the next line of the program file; restart from beginning if reached the end.
 *
 * @param programfile the file handle to the program file
 */
void next_program(FILE *programfile) {
    byte program_read_buffer[8192];
    dword number_of_bytes_read;
    int position_in_current_line;
    int hashmark_position = 0;
    int slash_position;
    /* a normal line should be at least 5 characters long + newline */
    int minimum_line_length = 6;

    /* Skip any line containing a hashmark */
    while (hashmark_position >= 0) {
        while ((number_of_bytes_read = get_next_line(program_read_buffer, programfile, 8192)) < minimum_line_length) {
            /* if 0 character has been read, seek to the beginning of the file */
            if (number_of_bytes_read < 1) {
                fseek(programfile, SEEK_SET, 0);
                pcount -= pdecrease;
                if (pcount) printf("repeat\n");
            }
        }
        hashmark_position = search_byte_in_buffer(program_read_buffer, hashmark);
    }
    // read number of repetitions for the tact described in current line
    tact_repetition_count = atoi(program_read_buffer);

    position_in_current_line = search_byte_in_buffer(program_read_buffer, space);
    position_in_current_line++;
    position_in_current_line += search_byte_in_buffer(&program_read_buffer[position_in_current_line], space);
    bpm_base_specified = bpt_base_specified = 0;

    if (position_in_current_line) {
        slash_position = search_byte_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        bpt[0] = atoi(&program_read_buffer[position_in_current_line]);

        if (slash_position >= 0) {
            bpt[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            bpt_base_specified = 1;
        }
    }

    position_in_current_line = search_byte_in_buffer(program_read_buffer, space);
    if (position_in_current_line) {
        slash_position = search_byte_in_buffer(&program_read_buffer[++position_in_current_line], slash);
        bpm[0] = atoi(&program_read_buffer[position_in_current_line]);
        if (debug) printf("debug: prg: bpm0: '%d', lo3: '%d'\n", bpm[0], slash_position);
        if (slash_position >= 0) {
            bpm[1] = atoi(&program_read_buffer[position_in_current_line] + ++slash_position);
            bpm_base_specified = 1;
        }
    }

    /* some parameter post-processing */
    if (!(bpt_base_specified)) {
        if (bpm_base_specified) {
            bpt[1] = bpm[1];
        } else {
            bpt[1] = default_base_note;
        }
    }

    if (!(bpm_base_specified)) {
        if (bpt_base_specified) {
            bpm[1] = bpt[1];
        } else {
            bpm[1] = default_base_note;
        }
    }

    if (bpt[0] < 1) bpt[0] = 1;
    if (bpt[0] > 50) bpt[0] = 50;
    if (bpt[1] < 1) bpt[1] = 1;
    if (bpt[1] > 50) bpt[1] = 50;

    if (bpm[0] > 250) bpm[0] = 250;
    if (bpm[0] < 30) bpm[0] = 30;
    if (bpm[1] > 20) bpm[1] = 20;
    if (bpm[1] < 1) bpm[1] = 1;
}

void parm_init(int argc, char *argv[]) {

    /* first, get the parameters */
    int current_argument, slash_position;
    dword bytes_read;
    FILE *wav_file;

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
            printf("%s %s\n", MYNAME, VERSION);
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
            slash_position = search_byte_in_buffer(argv[++current_argument], slash);
            bpt[0] = atoi(argv[current_argument]);
            if (slash_position >= 0) {
                bpt[1] = atoi(argv[current_argument] + ++slash_position);
                bpt_base_specified = 1;
            }
            if (debug) printf("debug: bpt: '%d'/'%d'\n", bpt[0], bpt[1]);
        }

        /* bpm */
        if ((strcmp(argv[current_argument], "-b") == 0) && (current_argument + 1 < argc)) {
            slash_position = search_byte_in_buffer(argv[++current_argument], slash);
            bpm[0] = atoi(argv[current_argument]);
            if (slash_position >= 0) {
                bpm[1] = atoi(argv[current_argument] + ++slash_position);
                bpm_base_specified = 1;
            }
            if (debug) printf("debug: bpm: '%d'/'%d'\n", bpm[0], bpm[1]);
        }

        /* pcount */
        if ((strcmp(argv[current_argument], "-c") == 0) && (current_argument + 1 < argc)) {
            pcount = atoi(argv[++current_argument]);
            pdecrease = 1;
            if (debug) printf("debug: count: '%d'\n", pcount);
        }

        /* program file */
        if ((strcmp(argv[current_argument], "-p") == 0) && (current_argument + 1 < argc)) {
            programfile = argv[++current_argument];
            is_program = 1;
            if (debug) printf("debug: program: '%s'\n", programfile);
        }
    }

    /* some parameter post-processing */
    if (!(bpt_base_specified)) {
        if (bpm_base_specified) {
            bpt[1] = bpm[1];
        } else {
            bpt[1] = default_base_note;
        }
    }

    if (!(bpm_base_specified)) {
        if (bpt_base_specified) {
            bpm[1] = bpt[1];
        } else {
            bpm[1] = default_base_note;
        }
    }

    if (bpm[0] > 250) bpm[0] = 250;
    if (bpm[0] < 30) bpm[0] = 30;
    if (bpm[1] > 20) bpm[1] = 20;
    if (bpm[1] < 1) bpm[1] = 1;

    if (bpt[0] < 1) bpt[0] = 1;
    if (bpt[0] > 50) bpt[0] = 50;
    if (bpt[1] < 1) bpt[1] = 1;
    if (bpt[1] > 50) bpt[1] = 50;

    /* cleanup buffers */
    for (dword buffer_position = 0; buffer_position < 1000000; buffer_position++) {
        wav1.data[buffer_position] = 0;
        wav2.data[buffer_position] = 0;
    }

    /* open wav file 1 */
    wav_file = open_file_for_reading(wav1_file_path);

    /* read the header first */
    bytes_read = fread(&wav_header, 1, 44, wav_file);
    if (bytes_read < 44) {
        printf("wav file %s too short\n", wav1_file_path);
        exit(1);
    }

    wav1.number_of_channels = *(word *) &wav_header[22];
    wav1.sample_rate = *(dword *) &wav_header[24];
    wav1.bits_per_sample = *(word *) &wav_header[34];

    if (debug)
        printf("debug: wav1 channels: '%d', sample rate: '%d', bits per sample: '%d'\n",
               wav1.number_of_channels, wav1.sample_rate, wav1.bits_per_sample);

    if (debug) printf("debug: calling dsp_init(%s)\n", dsp_device_path);

    dsp_device = dsp_init(dsp_device_path, wav1.bits_per_sample, wav1.number_of_channels, wav1.sample_rate);

    if (debug)
        printf("debug: dsp channels: '%d', samplerate: '%d', bits per sample: '%d'\n",
               dsp_device.number_of_channels, dsp_device.sample_rate, dsp_device.dsp_format);

    dsp_depth = dsp_device.dsp_format / 8;

    wav_bytes_to_read = dsp_depth * dsp_device.number_of_channels * dsp_device.sample_rate / 2;
    if (debug) printf("debug: maximum wav bytes to read: '%d'\n", wav_bytes_to_read);

    bytes_read = fread(&wav1.data, 1, wav_bytes_to_read, wav_file);
    if (debug) printf("debug: wav1 bytes read: '%d'\n", bytes_read);
    if (bytes_read < 10) {
        printf("wav file %s too short\n", wav1_file_path);
        exit(1);
    }

    fclose(wav_file);

    /* open wav file 2 */
    wav_file = open_file_for_reading(wav2_file_path);

    /* read the header first */
    bytes_read = fread(&wav_header, 1, 44, wav_file);
    if (bytes_read < 44) {
        printf("wav file %s too short\n", wav2_file_path);
        exit(1);
    }

    wav2.number_of_channels = *(word *) &wav_header[22];
    wav2.sample_rate = *(dword *) &wav_header[24];
    wav2.bits_per_sample = *(word *) &wav_header[34];

    if (debug)
        printf("debug: wav2 channels: '%d', sample rate: '%d', bits per sample: '%d'\n",
               wav2.number_of_channels, wav2.sample_rate, wav2.bits_per_sample);

    if ((wav1.number_of_channels != wav2.number_of_channels) ||
        (wav1.bits_per_sample != wav2.bits_per_sample)) {
        printf("the two WAV files must have the same number of channels and bits per sample\n");
        exit(1);
    }

    if ((debug) && (wav1.sample_rate != wav2.sample_rate)) {
        printf("debug: wav1 and wav2 sample rate differs, may sound funny\n");
    }

    bytes_read = fread(&wav2.data, 1, wav_bytes_to_read, wav_file);
    if (debug) printf("debug: wav2 bytes read: '%d'\n", bytes_read);
    if (bytes_read < 10) {
        printf("wav file %s too short\n", wav2_file_path);
        exit(1);
    }
    fclose(wav_file);

    /* open program file */
    if (is_program) program = open_file_for_reading(programfile);

    printf("bpm: %d/%d, bpt: %d/%d", bpm[0], bpm[1], bpt[0], bpt[1]);
    if (pdecrease) printf(", repeat count: %d", pcount);
    if (is_program) printf("\nprogram file: %s", programfile);
    printf("\n");
}
