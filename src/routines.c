#include <stdio.h>
#include <stdlib.h>
#include "ctronome.h"
#include "routines.h"

int str_search(byte *buffer, byte search) {
    dword i = 0;
    while ((buffer[i] != 0) && (buffer[i++] != search));
    if (buffer[--i] == search) return (i);
    return (-1);
}

dword get_next_line(byte *buffer, FILE *filehandle, dword max) {
    dword length;
    length = 0;

    while (fread(&buffer[length], 1, 1, filehandle) &&
           (buffer[length++] != '\n') &&
           (length < max));

    if (length == max) {
        printf("line buffer overflow\n");
        return (0);
    }

    buffer[length] = 0;
    return (length);
}

FILE *openread(byte *filename) {
    FILE *filehandle;

    if (!(filehandle = fopen(filename, "rb"))) {
        printf("cannot open %s\n", filename);
        exit(1);
    }

    return (filehandle);
}
