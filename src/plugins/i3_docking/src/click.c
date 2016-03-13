#undef I3__FILE__
#define I3__FILE__ "click.c"
/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * click.c: Button press (mouse click) events.
 *
 */


#include "data.h"
#include "con_.h"
#include "log.h"
#include "util.h"
#include "output.h"
#include "workspace.h"
#include "render.h"
#include "docksys.h"
#include <math.h>
#include <assert.h>
#include <pd_docking.h>

typedef struct MouseEvent
{
    void* userData;
    int x;
    int y;
    int dx;
    int dy;
    bool lmbDown;

} MouseEvent;

extern PDDockingCallbacks* g_callbacks;

typedef enum { CLICK_BORDER = 0,
               CLICK_DECORATION = 1,
               CLICK_INSIDE = 2 } click_destination_t;

typedef enum { BORDER_LEFT = (1 << 0),
               BORDER_RIGHT = (1 << 1),
               BORDER_TOP = (1 << 2),
               BORDER_BOTTOM = (1 << 3) } border_t;

bool resize_find_tiling_participants(Con **current, Con **other, direction_t direction) {
    DLOG("Find two participants for resizing container=%p in direction=%i\n", other, direction);
    Con *first = *current;
    Con *second = NULL;
    if (first == NULL) {
        DLOG("Current container is NULL, aborting.\n");
        return false;
    }

    /* Go up in the tree and search for a container to resize */
    const orientation_t search_orientation = ((direction == D_LEFT || direction == D_RIGHT) ? HORIZ : VERT);
    const bool dir_backwards = (direction == D_UP || direction == D_LEFT);
    while (first->type != CT_WORKSPACE &&
           first->type != CT_FLOATING_CON &&
           second == NULL) {
        /* get the appropriate first container with the matching
         * orientation (skip stacked/tabbed cons) */
        if ((con_orientation(first->parent) != search_orientation) ||
            (first->parent->layout == L_STACKED) ||
            (first->parent->layout == L_TABBED)) {
            first = first->parent;
            continue;
        }

        /* get the counterpart for this resizement */
        if (dir_backwards) {
            second = TAILQ_PREV(first, nodes_head, nodes);
        } else {
            second = TAILQ_NEXT(first, nodes);
        }

        if (second == NULL) {
            DLOG("No second container in this direction found, trying to look further up in the tree...\n");
            first = first->parent;
        }
    }

    DLOG("Found participants: first=%p and second=%p.\n", first, second);
    *current = first;
    *other = second;
    if (first == NULL || second == NULL) {
        DLOG("Could not find two participants for this resize request.\n");
        return false;
    }

    return true;
}


int resize_graphical_handler(Con *first, Con *second, orientation_t orientation, const MouseEvent* event) {
    DLOG("resize handler\n");

    /* TODO: previously, we were getting a rect containing all screens. why? */
    Con *output = con_get_output(first);
    DLOG("x = %d, width = %d\n", output->rect.x, output->rect.width);

#if 0
    x_mask_event_mask(~XCB_EVENT_MASK_ENTER_WINDOW);
    xcb_flush(conn);

    uint32_t mask = 0;
    uint32_t values[2];

    mask = XCB_CW_OVERRIDE_REDIRECT;
    values[0] = 1;

    /* Open a new window, the resizebar. Grab the pointer and move the window around
       as the user moves the pointer. */
    xcb_window_t grabwin = create_window(conn, output->rect, XCB_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT,
                                         XCB_WINDOW_CLASS_INPUT_ONLY, XCURSOR_CURSOR_POINTER, true, mask, values);

    /* Keep track of the coordinate orthogonal to motion so we can determine
     * the length of the resize afterward. */
    uint32_t initial_position, new_position;

    /* Configure the resizebar and snap the pointer. The resizebar runs along
     * the rect of the second con and follows the motion of the pointer. */
    Rect helprect;
    if (orientation == HORIZ) {
        helprect.x = second->rect.x;
        helprect.y = second->rect.y;
        helprect.width = logical_px(2);
        helprect.height = second->rect.height;
        initial_position = second->rect.x;
        //xcb_warp_pointer(conn, XCB_NONE, event->root, 0, 0, 0, 0, second->rect.x, event->root_y);
    } else {
        helprect.x = second->rect.x;
        helprect.y = second->rect.y;
        helprect.width = second->rect.width;
        helprect.height = logical_px(2);
        initial_position = second->rect.y;
        xcb_warp_pointer(conn, XCB_NONE, event->root, 0, 0, 0, 0,
                         event->root_x, second->rect.y);
    }

    mask = XCB_CW_BACK_PIXEL;
    values[0] = config.client.focused.border;

    mask |= XCB_CW_OVERRIDE_REDIRECT;
    values[1] = 1;

    xcb_window_t helpwin = create_window(conn, helprect, XCB_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT,
                                         XCB_WINDOW_CLASS_INPUT_OUTPUT, (orientation == HORIZ ? XCURSOR_CURSOR_RESIZE_HORIZONTAL : XCURSOR_CURSOR_RESIZE_VERTICAL), true, mask, values);

    xcb_circulate_window(conn, XCB_CIRCULATE_RAISE_LOWEST, helpwin);

    xcb_flush(conn);

    /* `new_position' will be updated by the `resize_callback'. */
    new_position = initial_position;

    const struct callback_params params = {orientation, output, helpwin, &new_position};

    /* `drag_pointer' blocks until the drag is completed. */
    drag_result_t drag_result = drag_pointer(NULL, event, grabwin, BORDER_TOP, 0, resize_callback, &params);

    xcb_destroy_window(conn, helpwin);
    xcb_destroy_window(conn, grabwin);
    xcb_flush(conn);
#endif

    const int pixels = orientation == VERT ? event->dy : event->dx;

    // if we got thus far, the containers must have
    // percentages associated with them
    assert(first->percent > 0.0);
    assert(second->percent > 0.0);

    // calculate the new percentage for the first container
    double new_percent, difference;
    double percent = first->percent;
    DLOG("percent = %f\n", percent);
    int original = (orientation == HORIZ ? first->rect.width : first->rect.height);
    DLOG("original = %d\n", original);
    new_percent = (original + pixels) * (percent / original);
    difference = percent - new_percent;
    DLOG("difference = %f\n", difference);
    DLOG("new percent = %f\n", new_percent);
    first->percent = new_percent;

    // calculate the new percentage for the second container
    double s_percent = second->percent;
    second->percent = s_percent + difference;
    DLOG("second->percent = %f\n", second->percent);

    // now we must make sure that the sum of the percentages remain 1.0
    con_fix_percent(first->parent);

    return 0;
}

/*
 * Finds the correct pair of first/second cons between the resize will take
 * place according to the passed border position (top, left, right, bottom),
 * then calls resize_graphical_handler().
 *
 */
static bool tiling_resize_for_border(Con *con, border_t border, const MouseEvent *event) {
    DLOG("border = %d, con = %p\n", border, con);
    Con *second = NULL;
    Con *first = con;
    direction_t search_direction;
    switch (border) {
        case BORDER_LEFT:
            search_direction = D_LEFT;
            break;
        case BORDER_RIGHT:
            search_direction = D_RIGHT;
            break;
        case BORDER_TOP:
            search_direction = D_UP;
            break;
        case BORDER_BOTTOM:
            search_direction = D_DOWN;
            break;
        default:
            assert(false);
            break;
    }

    bool res = resize_find_tiling_participants(&first, &second, search_direction);
    if (!res) {
        LOG("No second container in this direction found.\n");
        return false;
    }

    assert(first != second);
    assert(first->parent == second->parent);

    /* The first container should always be in front of the second container */
    if (search_direction == D_UP || search_direction == D_LEFT) {
        Con *tmp = first;
        first = second;
        second = tmp;
    }

    const orientation_t orientation = ((border == BORDER_LEFT || border == BORDER_RIGHT) ? HORIZ : VERT);

    resize_graphical_handler(first, second, orientation, event);

    DLOG("After resize handler, rendering\n");
    tree_render();
    return true;
}

/*
 * Called when the user clicks using the floating_modifier, but the client is in
 * tiling layout.
 *
 * Returns false if it does not do anything (that is, the click should be sent
 * to the client).
 *
 */
#if 0
static bool floating_mod_on_tiled_client(Con *con, MouseEvent *event) {
    /* The client is in tiling layout. We can still initiate a resize with the
     * right mouse button, by chosing the border which is the most near one to
     * the position of the mouse pointer */
    int to_right = con->rect.width - event->event_x,
        to_left = event->event_x,
        to_top = event->event_y,
        to_bottom = con->rect.height - event->event_y;

    DLOG("click was %d px to the right, %d px to the left, %d px to top, %d px to bottom\n",
         to_right, to_left, to_top, to_bottom);

    if (to_right < to_left &&
        to_right < to_top &&
        to_right < to_bottom)
        return tiling_resize_for_border(con, BORDER_RIGHT, event);

    if (to_left < to_right &&
        to_left < to_top &&
        to_left < to_bottom)
        return tiling_resize_for_border(con, BORDER_LEFT, event);

    if (to_top < to_right &&
        to_top < to_left &&
        to_top < to_bottom)
        return tiling_resize_for_border(con, BORDER_TOP, event);

    if (to_bottom < to_right &&
        to_bottom < to_left &&
        to_bottom < to_top)
        return tiling_resize_for_border(con, BORDER_BOTTOM, event);

    return false;
}
#endif

/*
 * Finds out which border was clicked on and calls tiling_resize_for_border().
 *
 */
static bool tiling_resize(Con *con, MouseEvent *event, const click_destination_t dest) {
    /* check if this was a click on the window border (and on which one) */
    I3Rect bsr = con_border_style_rect(con);
    DLOG("BORDER x = %d, y = %d for con %p, window 0x%08x\n",
         event->x, event->y, con, event->userData);
    DLOG("checks for right >= %d\n", con->window_rect.x + con->window_rect.width);
    if (dest == CLICK_DECORATION) {
        /* The user clicked on a window decoration. We ignore the following case:
         * The container is a h-split, tabbed or stacked container with > 1
         * window. Decorations will end up next to each other and the user
         * expects to switch to a window by clicking on its decoration. */

        /* Since the container might either be the child *or* already a split
         * container (in the case of a nested split container), we need to make
         * sure that we are dealing with the split container here. */
        Con *check_con = con;
        if (con_is_leaf(check_con) && check_con->parent->type == CT_CON)
            check_con = check_con->parent;

        if ((check_con->layout == L_STACKED ||
             check_con->layout == L_TABBED ||
             con_orientation(check_con) == HORIZ) &&
            con_num_children(check_con) > 1) {
            DLOG("Not handling this resize, this container has > 1 child.\n");
            return false;
        }
        return tiling_resize_for_border(con, BORDER_TOP, event);
    }

    if (event->x >= 0 && event->x <= (int32_t)bsr.x &&
        event->y >= (int32_t)bsr.y && event->y <= (int32_t)(con->rect.height + bsr.height))
        return tiling_resize_for_border(con, BORDER_LEFT, event);

    if (event->x >= (int32_t)(con->window_rect.x + con->window_rect.width) &&
        event->y >= (int32_t)bsr.y && event->y <= (int32_t)(con->rect.height + bsr.height))
        return tiling_resize_for_border(con, BORDER_RIGHT, event);

    if (event->y >= (int32_t)(con->window_rect.y + con->window_rect.height))
        return tiling_resize_for_border(con, BORDER_BOTTOM, event);

    return false;
}

/*
 * Being called by handle_button_press, this function calls the appropriate
 * functions for resizing/dragging.
 *
 */
static int route_click(Con *con, MouseEvent *event, const bool mod_pressed, const click_destination_t dest) {
    DLOG("--> click properties: mod = %d, destination = %d\n", mod_pressed, dest);
    DLOG("--> OUTCOME = %p\n", con);
    DLOG("type = %d, name = %s\n", con->type, con->name);

    /* don’t handle dockarea cons, they must not be focused */
    if (con->parent->type == CT_DOCKAREA)
        goto done;

    //const bool is_left_or_right_click = (event->detail == XCB_BUTTON_INDEX_1 || event->detail == XCB_BUTTON_INDEX_3);
    const bool is_left_or_right_click = event->lmbDown; 
#if 0
    /* if the user has bound an action to this click, it should override the
     * default behavior. */
    if (dest == CLICK_DECORATION || dest == CLICK_INSIDE || dest == CLICK_BORDER) {
        Binding *bind = get_binding_from_xcb_event((xcb_generic_event_t *)event);
        /* clicks over a window decoration will always trigger the binding and
         * clicks on the inside of the window will only trigger a binding if
         * the --whole-window flag was given for the binding. */
        if (bind && ((dest == CLICK_DECORATION || bind->whole_window) ||
                     (dest == CLICK_BORDER && bind->border))) {
            CommandResult *result = run_binding(bind, con);

            /* ASYNC_POINTER eats the event */
            xcb_allow_events(conn, XCB_ALLOW_ASYNC_POINTER, event->time);
            xcb_flush(conn);

            if (result->needs_tree_render)
                tree_render();

            command_result_free(result);

            return 0;
        }
    }
#endif

    /* There is no default behavior for button release events so we are done. */
    if (!event->lmbDown) {
        goto done;
    }

    /* Any click in a workspace should focus that workspace. If the
     * workspace is on another output we need to do a workspace_show in
     * order for i3bar (and others) to notice the change in workspace. */
    Con *ws = con_get_workspace(con);
    Con *focused_workspace = con_get_workspace(focused);

    if (!ws) {
        ws = TAILQ_FIRST(&(output_get_content(con_get_output(con))->focus_head));
        if (!ws)
            goto done;
    }

    if (ws != focused_workspace)
        workspace_show(ws);

    /* get the floating con */
    //Con *floatingcon = 0; //con_inside_floating(con);
    //const bool proportional = (event->state & BIND_SHIFT);
    const bool in_stacked = (con->parent->layout == L_STACKED || con->parent->layout == L_TABBED);

    /* 1: see if the user scrolled on the decoration of a stacked/tabbed con */
#if 0
    if (in_stacked &&
        dest == CLICK_DECORATION &&
        (event->detail == XCB_BUTTON_INDEX_4 ||
         event->detail == XCB_BUTTON_INDEX_5)) {
        DLOG("Scrolling on a window decoration\n");
        orientation_t orientation = (con->parent->layout == L_STACKED ? VERT : HORIZ);
        /* Focus the currently focused container on the same level that the
         * user scrolled on. e.g. the tabbed decoration contains
         * "urxvt | i3: V[xterm geeqie] | firefox",
         * focus is on the xterm, but the user scrolled on urxvt.
         * The splitv container will be focused. */
        Con *focused = con->parent;
        focused = TAILQ_FIRST(&(focused->focus_head));
        con_focus(focused);
        /* To prevent scrolling from going outside the container (see ticket
         * #557), we first check if scrolling is possible at all. */
        bool scroll_prev_possible = (TAILQ_PREV(focused, nodes_head, nodes) != NULL);
        bool scroll_next_possible = (TAILQ_NEXT(focused, nodes) != NULL);
        if (event->detail == XCB_BUTTON_INDEX_4 && scroll_prev_possible)
            tree_next('p', orientation);
        else if (event->detail == XCB_BUTTON_INDEX_5 && scroll_next_possible)
            tree_next('n', orientation);
        goto done;
    }
#endif

    /* 2: focus this con. */
    con_focus(con);

#if 0
    /* 3: For floating containers, we also want to raise them on click.
     * We will skip handling events on floating cons in fullscreen mode */
    Con *fs = (ws ? con_get_fullscreen_con(ws, CF_OUTPUT) : NULL);
    if (floatingcon != NULL && fs != con) {
        floating_raise_con(floatingcon);

        /* 4: floating_modifier plus left mouse button drags */
        if (mod_pressed && event->detail == XCB_BUTTON_INDEX_1) {
            floating_drag_window(floatingcon, event);
            return 1;
        }

        /*  5: resize (floating) if this was a (left or right) click on the
         * left/right/bottom border, or a right click on the decoration.
         * also try resizing (tiling) if it was a click on the top */
        if (mod_pressed && event->detail == XCB_BUTTON_INDEX_3) {
            DLOG("floating resize due to floatingmodifier\n");
            floating_resize_window(floatingcon, proportional, event);
            return 1;
        }

        if (!in_stacked && dest == CLICK_DECORATION &&
            is_left_or_right_click) {
            /* try tiling resize, but continue if it doesn’t work */
            DLOG("tiling resize with fallback\n");
            if (tiling_resize(con, event, dest))
                goto done;
        }

        if (dest == CLICK_DECORATION && event->detail == XCB_BUTTON_INDEX_3) {
            DLOG("floating resize due to decoration right click\n");
            floating_resize_window(floatingcon, proportional, event);
            return 1;
        }

        if (dest == CLICK_BORDER && is_left_or_right_click) {
            DLOG("floating resize due to border click\n");
            floating_resize_window(floatingcon, proportional, event);
            return 1;
        }

        /* 6: dragging, if this was a click on a decoration (which did not lead
         * to a resize) */
        if (!in_stacked && dest == CLICK_DECORATION &&
            (event->detail == XCB_BUTTON_INDEX_1)) {
            floating_drag_window(floatingcon, event);
            return 1;
        }

        goto done;
    }
#endif

    if (in_stacked) {
        /* for stacked/tabbed cons, the resizing applies to the parent
         * container */
        con = con->parent;
    }
#if 0
    /* 7: floating modifier pressed, initiate a resize */
    if (dest == CLICK_INSIDE && mod_pressed && event->detail == XCB_BUTTON_INDEX_3) {
        if (floating_mod_on_tiled_client(con, event))
            return 1;
    }
#endif
    /* 8: otherwise, check for border/decoration clicks and resize */
    else if ((dest == CLICK_BORDER || dest == CLICK_DECORATION) && is_left_or_right_click) {
        DLOG("Trying to resize (tiling)\n");
        tiling_resize(con, event, dest);
    }

done:
    //xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, event->time);
    //xcb_flush(conn);
    //tree_render();

    return 0;
}

/*
 * The button press X callback. This function determines whether the floating
 * modifier is pressed and where the user clicked (decoration, border, inside
 * the window).
 *
 * Then, route_click is called on the appropriate con.
 *
 */
#if 0
int handle_button_press(int x, int y, int mxd, int myd, bool lmb_down) {
    Con *con;

    MouseEvent event = { user_data, x, y, mxd, myd, lmb_down };

    /*
    DLOG("Button %d (state %d) %s on window 0x%08x (child 0x%08x) at (%d, %d) (root %d, %d)\n",
         event->detail, event->state, (event->response_type == XCB_BUTTON_PRESS ? "press" : "release"),
         event->event, event->child, event->event_x, event->event_y, event->root_x,
         event->root_y);
    */

    //const uint32_t mod = config.floating_modifier;
    const bool mod_pressed = false; //(mod != 0 && (event->state & mod) == mod);
    //DLOG("floating_mod = %d, detail = %d\n", mod_pressed, event->detail);
    //if ((con = con_by_window_id(event->event)))
    con = con_by_user_data(user_data);
    //if ((con = con_by_user_data(user_data)))
    //  return route_click(con, &event, mod_pressed, CLICK_INSIDE);

    if (!con)
        return 0;
#if 0
    if (!(con = con_by_frame_id(event->event))) {
        /* If the root window is clicked, find the relevant output from the
         * click coordinates and focus the output's active workspace. */
        if (event->event == root && event->response_type == XCB_BUTTON_PRESS) {
            Con *output, *ws;
            TAILQ_FOREACH(output, &(croot->nodes_head), nodes) {
                if (con_is_internal(output) ||
                    !rect_contains(output->rect, event->event_x, event->event_y))
                    continue;

                ws = TAILQ_FIRST(&(output_get_content(output)->focus_head));
                if (ws != con_get_workspace(focused)) {
                    workspace_show(ws);
                    tree_render();
                }
                return 1;
            }
            return 0;
        }

        ELOG("Clicked into unknown window?!\n");
        xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, event->time);
        xcb_flush(conn);
        return 0;
    }
#endif

    //con = con_get_workspace(focused);

    if (con)
    {
        /* Check if the click was on the decoration of a child */
        Con *child;
        TAILQ_FOREACH(child, &(con->nodes_head), nodes) {
            if (!rect_contains(child->deco_rect, event.x, event.y))
                continue;

            return route_click(child, &event, mod_pressed, CLICK_DECORATION);
        }
    }

    /*
    if (event.userData != 0) {
        DLOG("event->child not XCB_NONE, so this is an event which originated from a click into the application, but the application did not handle it.\n");
        return route_click(con, &event, mod_pressed, CLICK_INSIDE);
    }
    */

    return route_click(con, &event, mod_pressed, CLICK_BORDER);
}
#endif

typedef enum State
{
    State_None,
    State_HoverBorder,
    State_DragBorder,
    State_BeginDragView,
    State_DraggingView,
    State_EndDragView,
} State;

static State s_state = State_None; 
static int s_borderSize = 4;    // TODO: Use settings for border area
static Con* s_hoverCon = 0;
static border_t s_border;
static PDDockingCursor s_oldCursor = PDDockingCursor_Default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorStyle(PDDockingCursor cursor)
{
    if (!g_callbacks)
        return;

    if (!g_callbacks->set_cursor_style)
        return;

    if (s_oldCursor == cursor)
        return;

    g_callbacks->set_cursor_style(0, cursor);

    s_oldCursor = cursor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if cursor is i the border area of the window

static bool isHoveringBorder(Con* con, int mx, int my)
{
    Con *child;

    if (con->window)
    {
        // check right border

        int x = con->rect.x + con->rect.width - s_borderSize;
        int w = con->rect.x + con->rect.width;
        int y = con->rect.y;
        int h = y + con->rect.height;

        if ((mx >= x && mx < w) && (my >= y && my < h))
        {
            setCursorStyle(PDDockingCursor_SizeVertical);

            s_border = BORDER_RIGHT;
            s_hoverCon = con;
            return true;
        }

        // check lower border

        y = con->rect.y + con->rect.height - s_borderSize;
        h = con->rect.y + con->rect.height;
        x = con->rect.x;
        w = x + con->rect.width; 

        if ((mx >= x && mx < w) && (my >= y && my < h))
        {
            setCursorStyle(PDDockingCursor_SizeHorizontal);

            s_border = BORDER_BOTTOM;
            s_hoverCon = con;
            return true;
        }
    }

    TAILQ_FOREACH(child, &(con->nodes_head), nodes) 
    {
        if (isHoveringBorder(child, mx, my))
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateHoverBorder(const MouseEvent* event)
{
    s_hoverCon = 0;

    if (isHoveringBorder(croot, event->x, event->y) && event->lmbDown)
    {
        s_state = State_DragBorder;
    }
    else if (!s_hoverCon)
    {
        setCursorStyle(PDDockingCursor_Default);
        s_state = State_None;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDragBorder(const MouseEvent* event)
{
    if (!event->lmbDown)
    {
        setCursorStyle(PDDockingCursor_Default);
        s_state = State_None;
        return;
    }

    //tiling_resize(s_hoverCon, event, CLICK_BORDER); 
    tiling_resize_for_border(s_hoverCon, s_border, event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDefault(const MouseEvent* mouseEvent)
{
    // Check if we are hovering any sizer and we haven't pressed LMB

    if (isHoveringBorder(croot, mouseEvent->x, mouseEvent->y) && !mouseEvent->lmbDown)
    {
        s_state = State_HoverBorder;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int handle_button_press(int x, int y, int mxd, int myd, bool lmb_down) 
{
    MouseEvent event = { 0, x, y, mxd, myd, lmb_down };

    switch (s_state)
    {
        case State_None :
        {
            updateDefault(&event);
            break;
        }

        case State_HoverBorder:
        {
            updateHoverBorder(&event);
            break;
        }

        case State_DragBorder:
        {
            updateDragBorder(&event);
            break;
        }
    }

	return 0;
}



