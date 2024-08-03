# Ctronome Changelog
## Latest Versions
Since moving to GitHub, the changelog is now maintained in the git commit logs, so this changelog
file will not be updated anymore.

### GitHub
Go to [Ctronome GitHub Repository](https://github.com/bivanbi/ctronome) -> [Releases](https://github.com/bivanbi/ctronome/releases)

### Git Log
Example: to display changes between v0.5.6 and v0.5.7, go to git repository folder and issue:
```bash
git log v0.5.6..v0.5.7
```

## v0.5.7
- Move codebase to GitHub
- fix compilation error 'multiple definition of ...'
- Add Ubuntu 22.04 padsp issue and workaround to documentation
- Update documentation to Markdown format
- Fix various grammar issues in documentation / help text etc.
- Fix version numbering
- Exit after displaying version number with `-v` option
- Update home page URL to point to GitHub repository
- Exit after displaying version number
- Do not display homepage with version number, only version number
- Update help text for readability and add home page
- Fix logical expressions in [dsp.c](../src/dsp.c) always evaluating to false
- Prevent a possible buffer overflow when loading wav files into memory
- Reformat and refactor code for readability and better maintainability
- Resolve compiler warnings (incompatible variable types etc.)

## v0.5.6
- fixed compile-time warning in dsp.c:
  `dsp.c:51:2: warning: too many arguments for format [-Wformat-extra-args]`
- minor documentation improvements

## v0.5.5
- Documentation update: running ctronome with pulseaudio (modern Ubuntu distributions etc.)
  and some minor documentation touch-ups.
  NO change to the ctronome code, so no need to recompile and reinstall.

## v0.5.4
- fixed a [bug on amd64 platform](http://www.freebsd.org/cgi/query-pr.cgi?pr=ports/170207)
  that caused ctronome to produce no sound due to incorrect width of integers used in
  bitwise operations. Thanks to Dmitry Kazarov for the fix.

## v0.5.3
- added `#include <stdlib.h>` to [routines.c](../src/routines.c) also. Silly me, forgot to
  do that. Again, thanks to Martin for letting me know.

## v0.5.2
- included WAV samples digitally remastered by Martin Tarenskeen
  to be more pronounced and precise.
- added `#include <stdlib.h>` to [ctronome.c](../src/ctronome.c) to eliminate compile-time
  warnings with newer (>4) GCC versions

## v0.5.1
- new parameter processing bug introduced in v0.5 fixed
- some old bugs are fixed around parameter and program processing

## v0.5
- WAV format is detected, and DSP is set up accordingly
- included 2 channel (stereo) wav format for those sound
  systems that do not support mono mode
- bugfix in parameter processing that caused bpm base note
  to be ignored

## v0.4
- [dsp.c](../src/dsp.c) code cleanup, according to OSS Programmers Guide.
- implemented some debug messages, mainly for DSP procedures

## v0.3
- fixed dsp pattern length setup that
  caused silence at certain bpm rates

## v0.2
- fixed possible segfault bug in argument processing 
- Implemented program file
- more flexible bpm and bpt parameters
  allowing to specify the base note for BPM and beat per tact
- number of repetition can be specified via command-line

## v0.1
First version. Features:
- 30 < beat per minute < 250
- 1 < beat per tact < 50 
- 44100Hz 16bit mono WAV sound via /dev/dsp

