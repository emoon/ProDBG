#pragma once

#include <stdint.h>

// This is the C wrapper API for the UI plugins in ProDBG even if it is usable from C/C++ this
// API is mainly to be seen as a "low-level" API that a nicer API is wrapped on top of it.

typedef uint64_t PUHandle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Describes an 2d area where x,y is the upper left corner and width,height expands to lower right
typedef struct PURect {
	int x; /// x position
	int y; /// y position
	int width; /// width of the rect 
	int height; /// height of the rect 
} PURect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PUColor {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} PUColor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI {
	PUHandle (*button)(void* priv, const char* name, int name_len, void (*callback)(void* user_data));

	// paint functions

	void (*fill_rect)(void* priv, PURect rect, PUColor color);

	// this holds private data for the UI system and needs to be sent to each function call 
	void* priv;
	
} PDUI;
