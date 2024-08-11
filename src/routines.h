#ifndef CTRONOME_ROUTINES_H
#define CTRONOME_ROUTINES_H

#include <stdio.h>

typedef uint32_t DWORD;
typedef DWORD dword;

dword get_next_line(char *, FILE *, dword); /* read next line from file */
int search_character_in_buffer(const char *, char); /* search one character and return its position. */
int search_character_in_range(const char *, char, int, int); /* search one character in position range and return its position. */
FILE *open_file_for_reading(char *); /* open file for reading */

int string_to_decimal_int(const char *); /* convert string to integer */

#endif //CTRONOME_ROUTINES_H