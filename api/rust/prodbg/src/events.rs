///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: This file should be auto-generated

pub const ACTION_NONE: i32 = 0;
pub const ACTION_STOP: i32 = 1;
pub const ACTION_BREAK: i32 = 2;
pub const ACTION_RUN: i32 = 3;
pub const ACTION_STEP: i32 = 4;
pub const ACTION_STEP_OUT: i32 = 5;
pub const ACTION_STEP_OVER: i32 = 6;

// Events

pub const EVENT_NONE: i32 = 0;
pub const EVENT_GET_LOCALS: i32 = 1;
pub const EVENT_SET_LOCALS: i32 = 2;
pub const EVENT_GET_CALLSTACK: i32 = 3;
pub const EVENT_SET_CALLSTACK: i32 = 4;
pub const EVENT_GET_WATCH: i32 = 5;
pub const EVENT_SET_WATCH: i32 = 6;
pub const EVENT_GET_REGISTERS: i32 = 7;
pub const EVENT_SET_REGISTERS: i32 = 8;
pub const EVENT_GET_MEMORY: i32 = 9;
pub const EVENT_SET_MEMORY: i32 = 10;
pub const EVENT_GET_TTY: i32 = 11;
pub const EVENT_SET_TTY: i32 = 12;
pub const EVENT_GET_EXCEPTION_LOCATION: i32 = 13;
pub const EVENT_SET_EXCEPTION_LOCATION: i32 = 14;
pub const EVENT_GET_DISASSEMBLY: i32 = 15;
pub const EVENT_SET_DISASSEMBLY: i32 = 16;
pub const EVENT_GET_STATUS: i32 = 17;
pub const EVENT_SET_STATUS: i32 = 18;
pub const EVENT_SET_THREADS: i32 = 19;
pub const EVENT_GET_THREADS: i32 = 20;
pub const EVENT_SELECT_THREAD: i32 = 21;
pub const EVENT_SELECT_FRAME: i32 = 22;
pub const EVENT_GET_SOURCE_FILES: i32 = 23;
pub const EVENT_SET_SOURCE_FILES: i32 = 24;

pub const EVENT_SET_SOURCE_CODE_FILE: i32 = 25;

pub const EVENT_SET_BREAKPOINT: i32 = 26;
pub const EVENT_REPLY_BREAKPOINT: i32 = 27;

pub const EVENT_DELETE_BREAKPOINT: i32 = 28;
pub const EVENT_SET_EXECUTABLE: i32 = 29;
pub const EVENT_ACTION: i32 = 30;
pub const EVENT_ATTACH_TO_PROCESS: i32 = 31;
pub const EVENT_ATTACH_TO_REMOTE_SESSION: i32 = 32;

pub const EVENT_EXECUTE_CONSOLE: i32 = 33;
pub const EVENT_GET_CONSOLE: i32 = 34;

pub const EVENT_MENU_EVENT: i32 = 35;

pub const PDEVENT_TOGGLE_BREAKPOINT_CURRENT_LINE: i32 = 36;

pub const PDEVENT_UPDATE_MEMORY: i32 = 37;
pub const PDEVENT_UPDATE_REGISTER: i32 = 38;
pub const PDEVENT_UPDATE_PC: i32 = 39;

