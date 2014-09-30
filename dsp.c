#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/soundcard.h>
#include "ctronome.h"

static int dsp_8bit_unsigned_format = AFMT_U8; /* 0x00000008 unsigned 8 bit*/
static int dsp_16bit_signed_format = AFMT_S16_LE; /* 0x00000010 signed 16 bit little endianess*/

static int debug_dsp_speed, debug_dsp_channels, debug_dsp_depth, supported_dsp_formats;

int dsp_init(byte *dsp_dev,word bitspersample, word channels, dword samplerate){
 int dsp_handle;

 dsp_format = 0;
 if (debug) printf ("debug: dsp.c: bitspersample: >%d<\n",bitspersample);
 /* check parameters */
 if (bitspersample == 8)
  dsp_format = dsp_8bit_unsigned_format;
 if (bitspersample == 16){
  dsp_format = dsp_16bit_signed_format;
 }

 if(!(dsp_format)){
  printf ("FATAL: only 8 and 16 bits per sample are supported. (got: %d\n",bitspersample);
  exit (1);
 }
 if (debug) printf ("debug: dsp.c: wanted AFMT: >%d<\n",dsp_format);

 if ((channels < 1) && (channels > 2)){
  printf ("FATAL: number of channels must be 1 (mono) or 2 (stereo) (got: %d\n",channels);
  exit (1);
 }
 dsp_channels = channels;

 if ((samplerate < 8000) && (samplerate > 96000)){
  printf ("FATAL: samplerate must be between 8000 and 96000 (got: %d\n",samplerate);
  exit (1);
 }
 dsp_speed = samplerate;

 /* Initialise dsp_dev */
 if (debug) printf ("debug: opening dsp '%s'\n",dsp_dev);
 if ((dsp_handle = open (dsp_dev, O_WRONLY)) == -1){
  printf ("FATAL: cannot open dsp device %s\n",dsp_dev);
  perror(dsp_dev);
  exit (1);
 }
 if (debug) printf ("debug: dsp device %s opened successfully\n",dsp_dev);

 if (debug){
  if (ioctl(dsp_handle, SNDCTL_DSP_GETFMTS, &supported_dsp_formats) == -1){
   printf ("FATAL: unable to get supported formats for %s\n",dsp_dev);
   perror("SNDCTL_DSP_GETFMT");
   exit(1);  
  }
  printf("debug: supported dsp formats: '%d'\n",supported_dsp_formats);
 }

 if (debug) printf ("debug: dsp: set format: SNDCTL_DSP_SETFMT(%d)\n",dsp_format);
 if ((ioctl (dsp_handle, SNDCTL_DSP_SETFMT, &dsp_format)) == -1){
  printf ("FATAL: unable to set output format for %s\n",dsp_dev);
  perror("SNDCTL_DSP_SETFMT");
  exit(1);  
 }

 if (debug) printf ("debug: dsp: set format returned '%d'\n",dsp_format);
 if ((bitspersample == 8) && (dsp_format != dsp_8bit_unsigned_format)){
  printf("FATAL: your dsp device does not seem to support unsigned 8 bit (AFMT_8U) format\n");
  exit(1);
 }

 if ((bitspersample == 16) && (dsp_format != dsp_16bit_signed_format)){
  printf("FATAL: your dsp device does not seem to support signed 16 bit (AFMT_S16_LE) format\n");
  exit(1);
 }

 /* Set dsp channels */
 if (debug) printf ("debug: dsp: set up channels: SNDCTL_DSP_CHANNELS(%d)\n",dsp_channels);
 /* ioctl (dsp_handle, SNDCTL_DSP_CHANNELS, &dsp_channels); */
 if (ioctl (dsp_handle, SNDCTL_DSP_CHANNELS, &dsp_channels) == -1){
  printf ("FATAL: unable to set no. of channels for %s\n",dsp_dev);
  perror("SNDCTL_DSP_CHANNELS");
  exit(1);
 }

 if (debug) printf ("debug: dsp: set no. of channels returned '%d'\n",dsp_channels);
 if (dsp_channels != channels){
  printf ("FATAL: your wav has %d channels, while your DSP only supports %d channels.\n",channels,dsp_channels);
  exit(1);
 }

 /* Set the DSP rate (in Hz) */
 if (debug) printf ("debug: dsp: set up speed (Hz): SNDCTL_DSP_SPEED(%d)\n",dsp_speed);
 if (ioctl (dsp_handle, SNDCTL_DSP_SPEED, &dsp_speed) == -1){
  printf ("FATAL: unable to set DSP speed for %s\n",dsp_dev);
  perror("SNDCTL_DSP_SPEED");
  exit(1);
 }

 if (debug) printf ("debug: dsp: set speed returned '%d'\n",dsp_speed);
 if (debug && (dsp_speed != samplerate)){
  printf ("debug: dsp: wav samplerate and dsp samplerate differs, will sound funny\n");
 }

 return(dsp_handle);
}

void dsp_close (byte dsp_handle)
{
 close(dsp_handle);
}

void dsp_write (byte dsp_handle,byte *from,dword count){
 dword bytes_written;
 bytes_written = write (dsp_handle,from,count);
 if (bytes_written < count){
  printf ("FATAL: tried to write %d bytes to dsp device,\n\
only %d is written\n",count,bytes_written);
  exit(1);  
 }
 return;
}

