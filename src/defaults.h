#ifndef CTRONOME_DEFAULTS_H
#define CTRONOME_DEFAULTS_H

extern const char *default_dsp_device_path;

/* means: 1/4, 1/8 etc. */
extern int default_base_note;
// ASSETS_INSTALL_DIR is set by CMake, see CMakePresets.json and README.md for more information
#define WAV1_DEFAULT_FILE_PATH ASSETS_INSTALL_DIR "/metronome1.wav"
#define WAV2_DEFAULT_FILE_PATH ASSETS_INSTALL_DIR "/metronome2.wav"

extern const int default_beat_per_minute[2];
extern const int default_beat_per_tact[2];

#endif //CTRONOME_DEFAULTS_H
