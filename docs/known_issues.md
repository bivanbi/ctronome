# Ctronome Known Issues
## Systems Not Supporting Mono Mode
There seem to be systems that do not support mono mode, or at least
Ctronome is unable to set it up.

In this case, use the [stereo WAVs](../assets) included.

Confirmed cases:
* `Intel Corp. 82801DB AC'97 Audio Controller (rev 2)` +
  `Linux Kernel v2.4.26*` +
  `Intel ICH (i8xx), SiS 7012, NVidia nForce Audio or AMD 768/811x` driver

## Ubuntu 22.04 with pulseaudio DSP emulator (padsp)
Error message:
```
ld.so: object '/usr/\788488LIB/pulseaudio/libpulsedsp.so' from LD_PRELOAD cannot be preloaded (cannot open shared object file): ignored."
```

The issue is with the bash script /usr/bin/padsp, which contains a bug.
See https://bugs.launchpad.net/ubuntu/+source/pulseaudio/+bug/2068940 for more details and workaround.
