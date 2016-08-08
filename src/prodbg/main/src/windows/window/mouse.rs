//! Adds mouse handling implementation to `Window`

use imgui_sys::Imgui;
use minifb::{CursorStyle, MouseButton, MouseMode};
use super::Window;
use super::super::viewdock::{Direction, DockHandle, SizerPos};

/// Tracks dragging state of mouse with next flow:
/// `Default` -> `PreDraggingDock`, `DraggingSizer`, `DraggingNothing`
/// `PreDraggingDock` -> `Default`, `DraggingDock`
/// any `Dragging` -> `Default`
pub enum MouseState {
    /// Nothing is done with mouse, it behaves like usual
    Default,
    /// Drag is started but no target was under cursor when drag was started. Mouse can only switch
    /// back to `Default` state from this.
    DraggingNothing,
    /// Sizer between docks is being dragged. It will update ratios to be under cursor every frame.
    DraggingSizer(SizerPos),
    /// Dock is being dragged. `Window` will highlight drop position under cursor every frame and
    /// change position of dock on drop.
    DraggingDock(DockHandle),
    /// Special state to prevent docks from being drag-and-dropped instantly on mouse click. Changes
    /// to `DraggingDock` when cursor is dragged far enough from starting point.
    PreDraggingDock(DockHandle, (f32, f32)),
}


impl Window {
    pub fn update_mouse_state(&mut self) -> bool {
        let mouse = self.get_mouse_pos();
        let show_context_menu = self.win.get_mouse_down(MouseButton::Right);
        if show_context_menu {
            self.context_menu_data = self.ws
                .get_dock_handle_at_pos(mouse)
                .map(|handle| (handle, mouse));
            self.mouse_state = MouseState::Default;
        } else {
            self.update_mouse_dragging_state(mouse);
        }
        show_context_menu
    }

    pub fn get_mouse_pos(&self) -> (f32, f32) {
        self.win.get_mouse_pos(MouseMode::Clamp).unwrap_or((0.0, 0.0))
    }

    /// Sends current mouse keys and scroll state to ImGui
    pub fn update_imgui_mouse(&self) {
        Imgui::set_mouse_pos(self.get_mouse_pos());
        Imgui::set_mouse_state(0, self.win.get_mouse_down(MouseButton::Left));
        if let Some(scroll) = self.win.get_scroll_wheel() {
            Imgui::set_scroll(scroll.1 * 0.25);
        }
    }

    /// See `MouseState` description for more.
    fn update_mouse_dragging_state(&mut self, mouse_pos: (f32, f32)) {
        let mut next_state = None;
        let cursor;
        let mut should_save_ws_state = false;
        match self.mouse_state {
            MouseState::Default => {
                if let Some(sizer) = self.ws.get_sizer_at_pos(mouse_pos) {
                    cursor = match sizer.2 {
                        Direction::Vertical => CursorStyle::ResizeLeftRight,
                        Direction::Horizontal => CursorStyle::ResizeUpDown,
                    };
                    if self.win.get_mouse_down(MouseButton::Left) {
                        next_state = Some(MouseState::DraggingSizer(sizer));
                    }
                } else if let Some(handle) = self.ws.get_dock_handle_with_header_at_pos(mouse_pos) {
                    cursor = CursorStyle::ClosedHand;
                    if self.win.get_mouse_down(MouseButton::Left) {
                        next_state = Some(MouseState::PreDraggingDock(handle, mouse_pos));
                    }
                } else {
                    cursor = CursorStyle::Arrow;
                    if self.win.get_mouse_down(MouseButton::Left) {
                        next_state = Some(MouseState::DraggingNothing);
                    }
                }
            }

            MouseState::DraggingNothing => {
                cursor = CursorStyle::Arrow;
                if !self.win.get_mouse_down(MouseButton::Left) {
                    next_state = Some(MouseState::Default);
                }
            }

            MouseState::DraggingSizer(SizerPos(handle, index, direction)) => {
                if self.win.get_mouse_down(MouseButton::Left) {
                    cursor = match direction {
                        Direction::Vertical => CursorStyle::ResizeLeftRight,
                        Direction::Horizontal => CursorStyle::ResizeUpDown,
                    };
                    self.ws.set_sizer_at(handle, index, mouse_pos);
                } else {
                    next_state = Some(MouseState::Default);
                    cursor = CursorStyle::Arrow;
                    should_save_ws_state = true;
                }
            }

            MouseState::PreDraggingDock(handle, initial_mouse_pos) => {
                cursor = CursorStyle::Arrow;
                if self.win.get_mouse_down(MouseButton::Left) {
                    if (initial_mouse_pos.0 - mouse_pos.0).abs() >= 5.0 &&
                       (initial_mouse_pos.1 - mouse_pos.1).abs() >= 5.0 {
                        next_state = Some(MouseState::DraggingDock(handle));
                    }
                } else {
                    next_state = Some(MouseState::Default);
                }
            }

            MouseState::DraggingDock(handle) => {
                let mut move_target = self.ws.get_item_target_at_pos(mouse_pos);

                if let Some(target_handle) = self.ws.get_dock_handle_at_pos(mouse_pos) {
                    if target_handle == handle {
                        move_target = None;
                    }
                }

                if self.win.get_mouse_down(MouseButton::Left) {
                    cursor = match move_target {
                        Some(_) => CursorStyle::OpenHand,
                        None => CursorStyle::ClosedHand,
                    };
                    if let Some((_, rect)) = move_target {
                        if let Some(dh) = self.ws.get_dock_handle_at_pos(mouse_pos) {
                            self.overlay = Some((dh, rect));
                        } else {
                            self.overlay = None;
                        }
                    } else {
                        self.overlay = None;
                    }
                } else {
                    if let Some((target, _)) = move_target {
                        if !self.ws.already_at_place(&target, handle) {
                            self.ws.move_dock(handle, target);
                            self.save_cur_workspace_state();
                        }
                    }
                    next_state = Some(MouseState::Default);
                    cursor = CursorStyle::Arrow;
                    self.overlay = None;
                }
            }
        }

        self.win.set_cursor_style(cursor);
        if let Some(ns) = next_state {
            self.mouse_state = ns;
        }
        if should_save_ws_state {
            self.save_cur_workspace_state();
        }
    }
}
