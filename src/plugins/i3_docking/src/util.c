#undef I3__FILE__
#define I3__FILE__ "util.c"
/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * util.c: Utility functions, which can be useful everywhere within i3 (see
 *         also libi3).
 *
 */

#include "util.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#undef min
#undef max

int min(int a, int b) {
    return (a < b ? a : b);
}

int max(int a, int b) {
    return (a > b ? a : b);
}

bool rect_contains(I3Rect rect, uint32_t x, uint32_t y) {
    return (x >= rect.x &&
            x <= (rect.x + rect.width) &&
            y >= rect.y &&
            y <= (rect.y + rect.height));
}

I3Rect rect_add(I3Rect a, I3Rect b) {
    return (I3Rect){a.x + b.x,
                  a.y + b.y,
                  a.width + b.width,
                  a.height + b.height};
}

I3Rect rect_sub(I3Rect a, I3Rect b) {
    return (I3Rect){a.x - b.x,
                  a.y - b.y,
                  a.width - b.width,
                  a.height - b.height};
}

/*
 * Returns true if the name consists of only digits.
 *
 */
bool name_is_digits(const char *name) {
    /* positive integers and zero are interpreted as numbers */
    for (size_t i = 0; i < strlen(name); i++)
        if (!isdigit(name[i]))
            return false;

    return true;
}

/*
 * Parses the workspace name as a number. Returns -1 if the workspace should be
 * interpreted as a "named workspace".
 *
 */

long ws_name_to_number(const char *name) {
    /* positive integers and zero are interpreted as numbers */
    char *endptr = NULL;
    long parsed_num = strtol(name, &endptr, 10);
    if (parsed_num == LONG_MIN ||
        parsed_num == LONG_MAX ||
        parsed_num < 0 ||
        endptr == name) {
        parsed_num = -1;
    }

    return parsed_num;
}

