# strfind

Implementation of a SIMD-friendly algorithm to locate a substring (needle) in a given string (haystack). See: http://0x80.pl/articles/simd-strfind.html.

It consists in loading the first and the last bytes of the needle in SIMD vectors that are then used to look for those two bytes in their respective positions in the haystack. This search is much faster than checking a full match of the needle. Only when a match of those two bytes is found is the full substring match verified.
