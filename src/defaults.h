#ifndef CTRONOME_DEFAULTS_H
#define CTRONOME_DEFAULTS_H

#define MY_NAME "ctronome"
#define HOMEPAGE "https://github.com/bivanbi/ctronome\n"

// ASSETS_INSTALL_DIR is set by CMake, see CMakePresets.json and README.md for more information
#define WAV1_DEFAULT_FILE_PATH ASSETS_INSTALL_DIR "/metronome1.wav"
#define WAV2_DEFAULT_FILE_PATH ASSETS_INSTALL_DIR "/metronome2.wav"
#define DSP_DEFAULT_DEVICE_PATH "/dev/dsp"
#define DEFAULT_BASE_NOTE 4

static const char slash = 47; /* the / character */
static const char hashmark = 35; /* the # character */
static const char space = 32; /* the   character */
static const char newline = 10; /* the newline character */

extern const int default_beat_per_minute[2];
extern const int default_beat_per_tact[2];

#endif //CTRONOME_DEFAULTS_H
