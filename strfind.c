#include <arm_neon.h>
#include <stdio.h>
#include <string.h>

// Locate a substring in a given string.
// This algorithm comes from http://0x80.pl/articles/simd-strfind.html.
size_t strfind(char *haystack, size_t haystack_size, char *needle, size_t needle_size)
{
    if (haystack_size == 0 || needle_size == 0)
    {
        return -1;
    }

    // comments assume looking for `cat` in `a_cat_tries`
    // load 16-byte registers
    // F = [ c | c | c | c | c | c | c | c | c | c | c | c | c | c | c | c ]
    // L = [ t | t | t | t | t | t | t | t | t | t | t | t | t | t | t | t ]
    uint8x16_t first = vdupq_n_u8(needle[0]);
    uint8x16_t last = vdupq_n_u8(needle[needle_size - 1]);

    const void *ptr = haystack;

    uint8_t mask[16];

    for (size_t i = 0; i < haystack_size; i += 16)
    {
        // get iteration blocks
        // A = [ a | _ | c | a | t | _ | t | r | i | e | s | X | X | X | X | X ]
        // B = [ c | a | t | _ | t | r | i | e | s | X | X | X | X | X | X | X ]
        uint8x16_t block_first = vld1q_u8(ptr + i);
        uint8x16_t block_last = vld1q_u8(ptr + i + needle_size - 1);

        // compare blocks with first and last
        // AF = [ 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 ]
        // BL = [ 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 ]
        uint8x16_t eq_first = vceqq_u8(first, block_first);
        uint8x16_t eq_last = vceqq_u8(last, block_last);

        // get predicate / mask
        // mask = [ 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 ]
        uint8x16_t pred = vandq_u8(eq_first, eq_last);
        vst1q_u8(mask, pred);

        for (size_t j = 0; j < 16; j++)
        {
            if (mask[j])
            // possible substring occurrence: perform substring comparison
            {
                if (memcmp(haystack + i + j + 1, needle + 1, needle_size - 2) == 0)
                // substring match: return current iteration + offset
                {
                    return i + j;
                }
            }
        }
    }

    return -1;
}

// tests
int main()
{
    char *haystack;
    char *needle;
    size_t idx;

    haystack = "a_cat_tries";
    needle = "cat";
    idx = strfind(haystack, 11, needle, 3);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // 2

    haystack = "a_dog_tries_cat_dog_tries_a_cat_tries";
    needle = "tries";
    idx = strfind(haystack, 37, needle, 5);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // 6

    haystack = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    needle = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    idx = strfind(haystack, 261, needle, 26);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // 182

    haystack = "prefixabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    needle = "prefix";
    idx = strfind(haystack, 58, needle, 6);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // 0

    haystack = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzsuffix";
    needle = "suffix";
    idx = strfind(haystack, 58, needle, 6);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // 52

    haystack = "a_dog_tries";
    needle = "cat";
    idx = strfind(haystack, 11, needle, 3);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // -1 = 18446744073709551615 (unsigned)

    haystack = "";
    needle = "cat";
    idx = strfind(haystack, 0, needle, 3);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // -1 = 18446744073709551615 (unsigned)

    haystack = "a_dog_tries";
    needle = "";
    idx = strfind(haystack, 11, needle, 0);
    printf("`%s` in `%s`: %zu\n", needle, haystack, idx); // -1 = 18446744073709551615 (unsigned)

    return 0;
}
