dword getnextline(byte *, FILE *, dword); /* read next line from file */
byte str_compare(byte *, byte *, byte); /* string comparision with delimiter */
dword str_copy(byte *, byte *, dword); /* string copy */
dword str_dcopy(byte *, byte *, byte, dword); /* string copy with delimiter */
dword str_replace(byte *, byte, byte); /* search-and-replace one character */
int str_search(byte *, byte); /* search one character and return its pos. */
FILE *openread(byte *); /* open file for reading */
