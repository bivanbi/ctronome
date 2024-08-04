# Ctronome
## Disclaimer
THIS SOFTWARE COMES AS-IS. ALTHOUGH THE AUTHOR DOES ITS BEST TO MAKE
THIS PROGRAM SAFE TO USE, MISTAKE HAPPENS. USE IT AT YOUR OWN RISK.

THE AUTHOR DOES NOT TAKE ANY RESPONSIBILITY FOR WHAT THIS SOFTWARE
DOES TO YOUR PC. BACKUP FREQUENTLY.

## Introduction
Ctronome is a very-very simple programmable metronome for Linux console,
with /dev/dsp (OSS) audio output. BPM ranges from 30 to 250, tacts like 3/4, 7/8
etc. are possible. It plays external WAV files, so one could use custom
sound samples.

Aim is to keep it as simple as it can be with the least dependencies.

*Note: Non-OSS sound systems need additional utils/libraries to 
provide OSS compatibility.*


## Installation
### Dependencies
Only the very basic C libraries are needed that I believe to exist
on virtually all Linuxes. You need to have Linux kernel header
files installed (they come with the Linux kernel source), and OSS
(or OSS compatible) sound card support.

#### CMake
Build ctronome requires [CMake 3.10](https://cmake.org/getting-started/) or later.
On Ubuntu or Debian, you can install it with:
```bash
sudo apt-get install cmake
```

### Compile-time Configuration
Edit [defaults.c](src/defaults.c) for defaults, the most important ones are:
   ```c
   const char *default_wav1_file_path = "/usr/share/ctronome/metronome1.wav\0";
   const char *default_wav2_file_path = "/usr/share/ctronome/metronome2.wav\0";
   const char *default_dsp_device_path = "/dev/dsp\0";
   ```
 
### Build Binaries
Go into ctronome repository root directory and issue:
```bash
cmake .
cmake --build .
```

A [single binary](build/bin/ctronome) will be compiled into the `build/bin` directory.

### Install Binary and wav files
Ctronome is a single binary. You can put it anywhere you want in
your local or remote filesystem. You need two, preferably short
WAV files as metronome sample - one for accented beats -, 
put in the place you specified in [ctronome.h](src/ctronome.h), or specified on the command line.

There are [two sets of wav samples](assets) included, recorded from a real metronome;
one for mono and one stereo output. You have to install the one that fits your sound
system (see [important notes](docs/important_notes.md)).

### Example compilation and installation into default location
```bash
git clone https://github.com/bivanbi/ctronome.git
cd ctronome/src/
make all

# Usually root permission is needed to install into default installation
sudo cp ctronome /usr/local/bin
sudo mkdir /usr/share/ctronome
sudo cp ../assets/*wav /usr/share/ctronome/
```

## Using Ctronome
### BPM and BPT Explanation
For BPM, you can specify the base note. E.g. 60/4 means:
60 1/4 notes per minute -> length of 1/4 note is 1 second.

For BPT you can also define the base note. E.g. 4/4 means:
4 1/4 notes per tact -> length of a tact is 4 seconds.

### Command Line Options
```bash
 ctronome -b <bpm> -t <bpt> -c <count> -w1 <wav1> -w2 <wav2> -d <dsp_device> -p <program> -h -v
    -b <bpm>            beat per minute default: 60/4, range: 30-250/1-20
    -t <bpt>            beat per tact default: 1/4, range: 1-50/1-50
    -p <filename>       program file
    -c <count>          play tact/program <count> times then exit
    -w1 <filename>      wav to use for accented (first) beat of tact
                         default: /usr/share/ctronome/metronome1.wav
    -w2 <filename>      wav to use for other beat of tact
                         default: /usr/share/ctronome/metronome2.wav
    -d <device>         dsp device, default /dev/dsp
    -h                  display help screen
    -debug              display debug messages
```

### Simple Metronome
Usage Examples:
```bash
# 60 BPM, no accents
ctronome

# 90 BPM, accent every second beat
ctronome -b 90 -t 2

# 120 BPM, accent every 4th beat
ctronome -b 120 -t 4

# Using 60 BPM / 4 notes per tact as basis, then re-calculate it to 7 notes per tact, accent ever 7th beat
ctronome -b 60/4 -t 7/7 
```

*If the base note for BPM is not specified, it defaults to the base note
of the tact.*

### Programmed Metronome

See [prog_example.txt](docs/prog_example.txt) to learn how to program ctronome.

### Using Custom WAV Files
Ctronome requires two 16 bit per sample WAV format, mono or stereo, and preferably
at least 44100Hz sample rate. *The two WAV file formats must be the same!*
Please note that if your sound system does not support the exact format
provided by the WAV files, it will try its best to play it anyway, but
the result will be unpredictable. Debug mode will tell you the details.


## Running on Non-OSS Sound Systems
On modern Linux distributions it is not very likely that native OSS is used.
In this case the following error is displayed when running ctronome:
```bash
FATAL: cannot open dsp device /dev/dsp
/dev/dsp: No such file or directory
```

### PulseAudio
PulseAudio has an OSS compatibility module that can be installed to provide
OSS support. For example on Ubuntu systems, the package is called `pulseaudio-utils`.
```bash
sudo apt-get install pulseaudio-utils
padsp ctronome <ctronome options>
```

A simple shell script can also be created, lets call it `ctronome-padsp`:
```sh
#!/usr/bin/env sh
padsp ctronome $@
```

### Alsa
An ALSA utility similar to PulseAudio's wrapper exists called `aoss`.
For example on Ubuntu system, the package is called `alsa-oss`.

```bash
sudo apt-get install alsa-oss
aoss ctronome <ctronome options>
```

A simple shell script can also be created, lets call it `ctronome-aoss`:
```sh
#!/usr/bin/env sh
aoss ctronome $@
```

## Bug Reports
Please open an [Issue at GitHub](https://github.com/bivanbi/ctronome/issues)

## Credits
* dsp code is based on gTick by Alex Roberts <bse@dial.pipex.com>,
  and also the OSS Programmers Guide

* Patch to eliminate warnings with Gcc4, and included WAV samples digitally
  remastered to be more pronounced and precise by Martin Tarenskeen.

* Patch to fix amd64 platform bug by Dmitry Kazarov
 
* Help from my friends: Zola & Arpy.

