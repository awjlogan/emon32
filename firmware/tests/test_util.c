#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

/* After directed tests, run some random values */
#define RANDOM_TESTS    1024ul

void
util_strlen(const char *str)
{
    unsigned int strlen_gold, strlen_util;

    strlen_gold = strlen(str);
    strlen_util = utilStrlen(str);
    if (strlen_util != strlen_gold)
    {
        printf("    %s : %d\n", str, strlen_util);
        assert(strlen_util == strlen_gold);
    }
}

void
util_itoa()
{
    int32_t vals[5] = {0, -1, 1, INT32_MAX, INT32_MIN};
    uint32_t valtohex[4] = {0, 1, 255, UINT32_MAX};
    char abuf_gold[24], abuf_util[24];

    printf("    base 10 ... ");
    for (unsigned int idx = 0; idx < 5; idx++)
    {
        int32_t val = vals[idx];
        memset(abuf_util, 0, 24);
        memset(abuf_gold, 0, 24);
        snprintf(abuf_gold, 24, "%d", val);
        utilItoa(abuf_util, val, ITOA_BASE10);
        if (0 != strcmp(abuf_gold, abuf_util))
        {
            printf("    %d : %s\n", val, abuf_util);
            assert(!(strcmp(abuf_gold, abuf_util)));
        }
    }

    for (unsigned int idx = 0; idx < RANDOM_TESTS; idx++)
    {
        int32_t val = rand() % UINT32_MAX - INT32_MAX;
        memset(abuf_util, 0, 24);
        memset(abuf_util, 0, 24);
        snprintf(abuf_gold, 24, "%d", val);
        utilItoa(abuf_util, val, ITOA_BASE10);
        if (0 != strcmp(abuf_gold, abuf_util))
        {
            printf("    %d : %s\n", val, abuf_util);
            assert(!(strcmp(abuf_gold, abuf_util)));
        }
    }
    printf(" PASSED\n");

    printf("    base 16 ... ");
    for (unsigned int idx = 0; idx < 4; idx++)
    {
        uint32_t val = valtohex[idx];
        memset(abuf_util, 0, 24);
        memset(abuf_gold, 0, 24);
        snprintf(abuf_gold, 24, "%x", val);
        utilItoa(abuf_util, val, ITOA_BASE16);
        if (0 != strcmp(abuf_gold, abuf_util))
        {
            printf("  %d : %s\n", val, abuf_util);
            assert(!(strcmp(abuf_gold, abuf_util)));
        }
    }

    for (unsigned int idx = 0; idx < RANDOM_TESTS; idx++)
    {
        uint32_t val = rand() % UINT32_MAX;
        memset(abuf_util, 0, 24);
        memset(abuf_util, 0, 24);
        snprintf(abuf_gold, 24, "%x", val);
        utilItoa(abuf_util, val, ITOA_BASE16);
        if (0 != strcmp(abuf_gold, abuf_util))
        {
            printf("    %d : %s\n", val, abuf_util);
            assert(!(strcmp(abuf_gold, abuf_util)));
        }
    }
    printf(" PASSED\n\n");
}

void
util_insert(const char *ins, const char *gold, unsigned int pos, unsigned int len, unsigned int gold_cursor)
{
    char base_str[] = "This is an existing string to insert into";
    unsigned int cursor = utilStrInsert(base_str, ins, pos, len);

    if ((strcmp(gold, base_str)) || (gold_cursor != cursor))
    {
        printf("\n    %s (%s) | %d (%d) \n", base_str, gold, cursor, gold_cursor);
        assert(0);
    }
}

int
main(int argc, char *argv[])
{
    printf("---- emon32 util test ----\n\n");

    /* Returns the length of a NULL terminated string excluding termination */
    printf("  utilStrlen ... ");
    const char str0[] = "";
    const char str5[] = "TEST5";
    const char str32[] = "01234567890123456789012345678901";
    util_strlen(str0);
    util_strlen(str5);
    util_strlen(str32);
    printf("PASSED\n\n");

    /* Convert an up to 32 bit value to string */
    srand(time(0));
    printf("  utilItoa:\n");
    util_itoa();

    /* Insert a string into an existing buffer */
    printf("  utilStrInsert ...");
    util_insert("MSG", "MSGs is an existing string to insert into", 0, 3, 3);
    util_insert("012345", "This is an existing 012345 to insert into", 20, 6, 26);
    util_insert("IS", "This IS an existing string to insert into", 5, 2, 7);

    printf("PASSED\n\n");
}