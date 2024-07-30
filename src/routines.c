#include <stdio.h>
#include <stdlib.h>
#include "ctronome.h"
#include "routines.h"

dword str_dcopy(byte *source, byte *destination, byte delimiter, dword max) {
    dword counter;
    counter = 0;

    while ((source[counter] != 0) && (source[counter] != delimiter) &&
           (counter < max))
        destination[counter] = source[counter++];

    if (source[counter] != delimiter)
        return (0);

    destination[counter] = 0;

    return (counter);
}

dword str_copy(byte *source, byte *destination, dword max) {
    dword counter;
    counter = 0;

    while ((source[counter] != 0) &&
           (counter < max))
        destination[counter] = source[counter++];

    destination[counter] = 0;

    return (counter);
}

byte str_compare(byte *a, byte *b, byte delimiter) {
    dword i = 0;

    while ((a[i] == b[i]) &&
           (a[i] != 0))
        i++;

    if ((a[i] == 0) &&
        ((b[i] == 0) || (b[i] == delimiter)))
        return (1);

    return (0);
}

int str_search(byte *buffer, byte search) {
    dword i = 0;
    while ((buffer[i] != 0) && (buffer[i++] != search));
    if (buffer[--i] == search) return (i);
    return (-1);
}

dword str_replace(byte *buffer, byte search, byte replace) {
    dword count = 0;
    dword i = 0;

    while (buffer[i] != 0) {
        if (buffer[i] == search) {
            buffer[i] = replace;
            count++;
        }
        i++;
    }

    return (count);
}

dword getnextline(byte *buffer, FILE *filehandle, dword max) {
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
