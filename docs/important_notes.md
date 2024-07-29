# Ctronome Important Notes
## Precision
Ctronome trusts timing on the sound card, by bulding and looping audio
samples with appropriate length. It *seems* to be accurate enough for
general practicing purposes.

## Input File Validation
### Program Files
The format of the program file is not strictly checked, however, ctronome
is not expected to crash if it encounters one.

### WAV Files
Ctronome does *not* thoroughly check the format of WAV files, only a portion of the
WAV header, e.g. number of channels, bits per sample, sample rate.
