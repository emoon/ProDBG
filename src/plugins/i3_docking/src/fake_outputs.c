#undef I3__FILE__
#define I3__FILE__ "fake_outputs.c"
/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * Faking outputs is useful in pathological situations (like network X servers
 * which don’t support multi-monitor in a useful way) and for our testsuite.
 *
 */

#include "data.h"
#include "output.h"
#include "con_.h"
#include "log.h"
#include "util.h"
#include "render.h"
#include "workspace.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include "all.h"
//

extern struct Con *croot;

static int num_screens;

struct outputs_head outputs = TAILQ_HEAD_INITIALIZER(outputs);
extern TAILQ_HEAD(ws_assignments_head, Workspace_Assignment) ws_assignments;

extern int sasprintf(char **strp, const char *fmt, ...);

/*
 * Get a specific output by its internal X11 id. Used by randr_query_outputs
 * to check if the output is new (only in the first scan) or if we are
 * re-scanning.
 *
 */
/*
static Output *get_output_by_id(xcb_randr_output_t id) {
    Output *output;
    TAILQ_FOREACH(output, &outputs, outputs)
    if (output->id == id)
        return output;

    return NULL;
}
*/

Output *get_output_next(direction_t direction, Output *current, output_close_far_t close_far) {
    I3Rect *cur = &(current->rect),
         *other;
    Output *output,
        *best = NULL;
    TAILQ_FOREACH(output, &outputs, outputs) {
        if (!output->active)
            continue;

        other = &(output->rect);

        if ((direction == D_RIGHT && other->x > cur->x) ||
            (direction == D_LEFT && other->x < cur->x)) {
            /* Skip the output when it doesn’t overlap the other one’s y
             * coordinate at all. */
            if ((other->y + other->height) <= cur->y ||
                (cur->y + cur->height) <= other->y)
                continue;
        } else if ((direction == D_DOWN && other->y > cur->y) ||
                   (direction == D_UP && other->y < cur->y)) {
            /* Skip the output when it doesn’t overlap the other one’s x
             * coordinate at all. */
            if ((other->x + other->width) <= cur->x ||
                (cur->x + cur->width) <= other->x)
                continue;
        } else
            continue;

        /* No candidate yet? Start with this one. */
        if (!best) {
            best = output;
            continue;
        }

        if (close_far == CLOSEST_OUTPUT) {
            /* Is this output better (closer to the current output) than our
             * current best bet? */
            if ((direction == D_RIGHT && other->x < best->rect.x) ||
                (direction == D_LEFT && other->x > best->rect.x) ||
                (direction == D_DOWN && other->y < best->rect.y) ||
                (direction == D_UP && other->y > best->rect.y)) {
                best = output;
                continue;
            }
        } else {
            /* Is this output better (farther to the current output) than our
             * current best bet? */
            if ((direction == D_RIGHT && other->x > best->rect.x) ||
                (direction == D_LEFT && other->x < best->rect.x) ||
                (direction == D_DOWN && other->y > best->rect.y) ||
                (direction == D_UP && other->y < best->rect.y)) {
                best = output;
                continue;
            }
        }
    }

    DLOG("current = %s, best = %s\n", current->name, (best ? best->name : "NULL"));
    return best;
}

Output *get_output_next_wrap(direction_t direction, Output *current) {
    Output *best = get_output_next(direction, current, CLOSEST_OUTPUT);
    /* If no output can be found, wrap */
    if (!best) {
        direction_t opposite;
        if (direction == D_RIGHT)
            opposite = D_LEFT;
        else if (direction == D_LEFT)
            opposite = D_RIGHT;
        else if (direction == D_DOWN)
            opposite = D_UP;
        else
            opposite = D_DOWN;
        best = get_output_next(opposite, current, FARTHEST_OUTPUT);
    }
    if (!best)
        best = current;
    DLOG("current = %s, best = %s\n", current->name, best->name);
    return best;
}


Output *get_output_from_string(Output *current_output, const char *output_str) {
    Output *output;

    if (strcmp(output_str, "left") == 0)
        output = get_output_next_wrap(D_LEFT, current_output);
    else if (strcmp(output_str, "right") == 0)
        output = get_output_next_wrap(D_RIGHT, current_output);
    else if (strcmp(output_str, "up") == 0)
        output = get_output_next_wrap(D_UP, current_output);
    else if (strcmp(output_str, "down") == 0)
        output = get_output_next_wrap(D_DOWN, current_output);
    else
        output = get_output_by_name(output_str);

    return output;
}

/*
 * Returns the output with the given name if it is active (!) or NULL.
 *
 */
Output *get_output_by_name(const char *name) {
    Output *output;
    TAILQ_FOREACH(output, &outputs, outputs)
    if (output->active &&
        strcmp(output->name, name) == 0)
        return output;

    return NULL;
}



/*
 * Looks in outputs for the Output whose start coordinates are x, y
 *
 */
static Output *get_screen_at(unsigned int x, unsigned int y) {
    Output *output;
    TAILQ_FOREACH(output, &outputs, outputs)
    if (output->rect.x == x && output->rect.y == y)
        return output;

    return NULL;
}

void output_init_con(Output *output) {
    Con *con = NULL, *current;
    bool reused = false;

    DLOG("init_con for output %s\n", output->name);

    /* Search for a Con with that name directly below the root node. There
     * might be one from a restored layout. */
    TAILQ_FOREACH(current, &(croot->nodes_head), nodes) {
        if (strcmp(current->name, output->name) != 0)
            continue;

        con = current;
        reused = true;
        DLOG("Using existing con %p / %s\n", con, con->name);
        break;
    }

    if (con == NULL) {
        con = con_new(croot, NULL);
        FREE(con->name);
        con->name = strdup(output->name);
        con->type = CT_OUTPUT;
        con->layout = L_OUTPUT;
        con_fix_percent(croot);
    }
    con->rect = output->rect;
    output->con = con;

    char *name;
    sasprintf(&name, "[i3 con] output %s", con->name);
    //x_set_name(con, name);
    FREE(name);

    if (reused) {
        DLOG("Not adding workspace, this was a reused con\n");
        return;
    }

    DLOG("Changing layout, adding top/bottom dockarea\n");
    Con *topdock = con_new(NULL, NULL);
    topdock->type = CT_DOCKAREA;
    topdock->layout = L_DOCKAREA;
    /* this container swallows dock clients */
    Match *match = malloc(sizeof(Match));
    memset(match, 0, sizeof(Match));
    //match_init(match);
    match->dock = M_DOCK_TOP;
    match->insert_where = M_BELOW;
    TAILQ_INSERT_TAIL(&(topdock->swallow_head), match, matches);

    FREE(topdock->name);
    topdock->name = strdup("topdock");

    sasprintf(&name, "[i3 con] top dockarea %s", con->name);
    //x_set_name(topdock, name);
    FREE(name);
    DLOG("attaching\n");
    con_attach(topdock, con, false);

    /* content container */

    DLOG("adding main content container\n");
    Con *content = con_new(NULL, NULL);
    content->type = CT_CON;
    content->layout = L_SPLITH;
    FREE(content->name);
    content->name = strdup("content");

    sasprintf(&name, "[i3 con] content %s", con->name);
    //x_set_name(content, name);
    FREE(name);
    con_attach(content, con, false);

    /* bottom dock container */
    Con *bottomdock = con_new(NULL, NULL);
    bottomdock->type = CT_DOCKAREA;
    bottomdock->layout = L_DOCKAREA;
    /* this container swallows dock clients */
    match = malloc(sizeof(Match));
    memset(match, 0, sizeof(Match));
    //ou
    //match_init(match);
    match->dock = M_DOCK_BOTTOM;
    match->insert_where = M_BELOW;
    TAILQ_INSERT_TAIL(&(bottomdock->swallow_head), match, matches);

    FREE(bottomdock->name);
    bottomdock->name = strdup("bottomdock");

    sasprintf(&name, "[i3 con] bottom dockarea %s", con->name);
    //x_set_name(bottomdock, name);
    FREE(name);
    DLOG("attaching\n");
    con_attach(bottomdock, con, false);
}

void init_ws_for_output(Output *output, Con *content) {
    /* go through all assignments and move the existing workspaces to this output */
    struct Workspace_Assignment *assignment;
    TAILQ_FOREACH(assignment, &ws_assignments, ws_assignments) {
        if (strcmp(assignment->output, output->name) != 0)
            continue;

        /* check if this workspace actually exists */
        Con *workspace = NULL, *out;
        TAILQ_FOREACH(out, &(croot->nodes_head), nodes)
        GREP_FIRST(workspace, output_get_content(out),
                   !strcmp(child->name, assignment->name));
        if (workspace == NULL)
            continue;

        /* check that this workspace is not already attached (that means the
         * user configured this assignment twice) */
        Con *workspace_out = con_get_output(workspace);
        if (workspace_out == output->con) {
            LOG("Workspace \"%s\" assigned to output \"%s\", but it is already "
                "there. Do you have two assignment directives for the same "
                "workspace in your configuration file?\n",
                workspace->name, output->name);
            continue;
        }

        /* if so, move it over */
        LOG("Moving workspace \"%s\" from output \"%s\" to \"%s\" due to assignment\n",
            workspace->name, workspace_out->name, output->name);

        /* if the workspace is currently visible on that output, we need to
         * switch to a different workspace - otherwise the output would end up
         * with no active workspace */
        bool visible = workspace_is_visible(workspace);
        Con *previous = NULL;
        if (visible && (previous = TAILQ_NEXT(workspace, focused))) {
            LOG("Switching to previously used workspace \"%s\" on output \"%s\"\n",
                previous->name, workspace_out->name);
            workspace_show(previous);
        }

        /* Render the output on which the workspace was to get correct I3Rects.
         * Then, we need to work with the "content" container, since we cannot
         * be sure that the workspace itself was rendered at all (in case it’s
         * invisible, it won’t be rendered). */
        render_con(workspace_out, false);
        Con *ws_out_content = output_get_content(workspace_out);

        Con *floating_con;
        TAILQ_FOREACH(floating_con, &(workspace->floating_head), floating_windows)
        /* NB: We use output->con here because content is not yet rendered,
             * so it has a rect of {0, 0, 0, 0}. */
        //floating_fix_coordinates(floating_con, &(ws_out_content->rect), &(output->con->rect));

        con_detach(workspace);
        con_attach(workspace, content, false);

        /* In case the workspace we just moved was visible but there was no
         * other workspace to switch to, we need to initialize the source
         * output aswell */
        if (visible && previous == NULL) {
            LOG("There is no workspace left on \"%s\", re-initializing\n",
                workspace_out->name);
            init_ws_for_output(get_output_by_name(workspace_out->name),
                               output_get_content(workspace_out));
            DLOG("Done re-initializing, continuing with \"%s\"\n", output->name);
        }
    }

    /* if a workspace exists, we are done now */
    if (!TAILQ_EMPTY(&(content->nodes_head))) {
        /* ensure that one of the workspaces is actually visible (in fullscreen
         * mode), if they were invisible before, this might not be the case. */
        Con *visible = NULL;
        GREP_FIRST(visible, content, child->fullscreen_mode == CF_OUTPUT);
        if (!visible) {
            visible = TAILQ_FIRST(&(content->nodes_head));
            focused = content;
            workspace_show(visible);
        }
        return;
    }

    /* otherwise, we create the first assigned ws for this output */
    TAILQ_FOREACH(assignment, &ws_assignments, ws_assignments) {
        if (strcmp(assignment->output, output->name) != 0)
            continue;

        LOG("Initializing first assigned workspace \"%s\" for output \"%s\"\n",
            assignment->name, assignment->output);
        focused = content;
        workspace_show_by_name(assignment->name);
        return;
    }

    /* if there is still no workspace, we create the first free workspace */
    DLOG("Now adding a workspace\n");
    Con *ws = create_workspace_on_output(output, content);

    /* TODO: Set focus in main.c */
    con_focus(ws);
}


/*
 * Creates outputs according to the given specification.
 * The specification must be in the format wxh+x+y, for example 1024x768+0+0,
 * with multiple outputs separated by commas:
 *   1900x1200+0+0,1280x1024+1900+0
 *
 */

void fake_outputs_init(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    //char useless_buffer[1024];
    DLOG("Parsed output as width = %u, height = %u at (%u, %u)\n",
            width, height, x, y);
    Output *new_output = get_screen_at(x, y);
    if (new_output != NULL) {
        DLOG("Re-used old output %p\n", new_output);
        /* This screen already exists. We use the littlest screen so that the user
            can always see the complete workspace */
        new_output->rect.width = min(new_output->rect.width, width);
        new_output->rect.height = min(new_output->rect.height, height);
    } else {
        new_output = malloc(sizeof(Output));
        sasprintf(&(new_output->name), "fake-%d", num_screens);
        DLOG("Created new fake output %s (%p)\n", new_output->name, new_output);
        new_output->active = true;
        new_output->rect.x = x;
        new_output->rect.y = y;
        new_output->rect.width = width;
        new_output->rect.height = height;
        /* We always treat the screen at 0x0 as the primary screen */
        if (new_output->rect.x == 0 && new_output->rect.y == 0)
            TAILQ_INSERT_HEAD(&outputs, new_output, outputs);
        else
            TAILQ_INSERT_TAIL(&outputs, new_output, outputs);
        output_init_con(new_output);
        init_ws_for_output(new_output, output_get_content(new_output->con));
        num_screens++;
    }

    if (num_screens == 0) {
        ELOG("No screens found. Please fix your setup. i3 will exit now.\n");
        exit(0);
    }
}
