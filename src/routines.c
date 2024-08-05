#include <stdio.h>
#include <stdlib.h>
#include "ctronome.h"
#include "routines.h"
#include "logging_adapter.h"

/**
 * Search fora particular byte value in a null-terminated haystack, and return the position of the first match.
 *
 * @param haystack the null-terminated buffer to search in
 * @param needle the value to look for
 * @return First position of the value in the buffer if found, else -1.
 */
int search_character_in_buffer(char *haystack, char needle) {
    dword position = 0;
    while ((haystack[position] != 0) && (haystack[position++] != needle));
    if (haystack[--position] == needle) return (position);
    return (-1);
}

dword get_next_line(char *buffer, FILE *file, dword max) {
    dword length;
    length = 0;

    while (fread(&buffer[length], 1, 1, file) &&
           (buffer[length++] != '\n') &&
           (length < max));

    if (length == max) {
        log_message(LEVEL_WARNING, "get_next_line: line buffer overflow\n");
        return (0);
    }

    buffer[length] = 0;
    return (length);
}

FILE *open_file_for_reading(char *file_path) {
    FILE *file;

    if (!(file = fopen(file_path, "rb"))) {
        log_message(LEVEL_ERROR, "cannot open file for reading '%s'\n", file_path);
        exit(1);
    }

    return (file);
}
