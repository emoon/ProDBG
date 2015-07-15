/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * output.c: Output (monitor) related functions.
 *
 */
#pragma once

typedef enum {
    CLOSEST_OUTPUT = 0,
    FARTHEST_OUTPUT = 1
} output_close_far_t;

/**
 * Returns the output container below the given output container.
 *
 */
Con *output_get_content(Con *output);

/**
 * Returns an 'output' corresponding to one of left/right/down/up or a specific
 * output name.
 *
 */
Output *get_output_from_string(Output *current_output, const char *output_str);

TAILQ_HEAD(outputs_head, xoutput);
extern struct outputs_head outputs;


Output *get_output_by_name(const char *name);

Output *get_output_next(direction_t direction, Output *current, output_close_far_t close_far);

void fake_outputs_init(uint32_t x, uint32_t y, uint32_t width, uint32_t height);


