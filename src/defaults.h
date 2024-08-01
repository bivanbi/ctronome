#ifndef CTRONOME_DEFAULTS_H
#define CTRONOME_DEFAULTS_H

extern const char *default_wav1_file_path;
extern const char *default_wav2_file_path;
extern const char *default_dsp_device_path;

/* means: 1/4, 1/8 etc. */
extern int default_base_note;

extern const int default_beat_per_minute[2];
extern const int default_beat_per_tact[2];

#define MAXIMUM_WAV_DATA_SIZE_BYTES 1000000

#endif //CTRONOME_DEFAULTS_H
