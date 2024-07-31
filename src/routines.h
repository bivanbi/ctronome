#ifndef CTRONOME_ROUTINES_H
#define CTRONOME_ROUTINES_H

dword get_next_line(char *, FILE *, dword); /* read next line from file */
int search_character_in_buffer(char *haystack, char needle); /* search one character and return its position. */
FILE *open_file_for_reading(char *); /* open file for reading */

#endif //CTRONOME_ROUTINES_H