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
### Required Tools
- C compiler (gcc, clang, etc.)
- CMake 3.19 or later
- Git (required at compile time to get the version number from Git repo)

### Dependencies
- Linux Kernel with OSS* support

**OSS support can be provided with other tools such as PulseAudio OSS emulator*

### TL;DR: Example Build and Installation Into Default Location
```bash
git clone https://github.com/bivanbi/ctronome.git
cd ctronome
cmake --preset default
cmake --build --preset default
sudo cmake --install build
```

### Configure CMake project
Configure build for the default installation prefix of `/usr/local`:
```bash
cmake --preset default
```

To override the installation prefix, either edit [CMakePresets.json](CMakePresets.json),
or specify it on the command line, for example:
```bash
cmake --preset default -DCMAKE_INSTALL_PREFIX=/opt
```

*The reason it needs to be configured before compiling, is because the ctronome binary needs to know where
the WAV files will be installed.*
 
### Build Binaries
```bash
cmake --build --preset default
```

A [single binary](build/bin/ctronome) will be compiled into the `build/bin` directory.

### Install files
Binary will be installed in `<installation prefix>/bin`, WAV files will be installed into `<installation prefix>/share/ctronome`.
Default installation prefix is `/usr/local`. See [Confiure CMake project](#configure-cmake-project) to change the installation prefix.

```bash
cmake --install build
```

Notes:
- Root privileges may be required to install files into the default installation prefix. Use sudo if necessary, e.g.: `sudo cmake --install build`
- At the time of writing this document, CMake v3.22 did not support the `--preset` option for the `install` command.
Hence, the default build directory `build` is specified instead.
- Overriding the installation prefix with the `--prefix` option causes `ctronome` and its WAV files to be installed
into a location where `ctronome` will not be able to find them. In this case, wav file locations need to be
specified when running `ctronome` with the `-w1` and `-w2` options.

### Uninstall
Simply remove the following files / directories:
```bash
sudo rm -rfv <installation prefix>/bin/ctronome <installation prefix>/share/ctronome
```

Where `<installation prefix>` is the installation prefix used during the installation.

Example removal from default installation prefix of `/usr/local`:
```bash
sudo rm -rfv /usr/local/bin/ctronome /usr/local/share/ctronome
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
of the tact and vica verse. If neither BPM nor BPT base note is set, it defaults to `DEFAULT_BASE_NOTE` set
in [defaults.h](src/defaults.h)*

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

