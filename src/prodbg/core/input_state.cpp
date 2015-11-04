#include "input_state.h"
#include "core/core.h"
#include "pd_keys.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static InputState s_inputState;
// TODO: Move to settings
static const float KeyRepeatDelay = 0.250f;
static const float KeyRepeatRate = 0.050f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InputState* InputState_getState() {
    return &s_inputState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InputState_update(float deltaTime) {
    InputState* state = &s_inputState;
    state->deltaTime = deltaTime;

    for (uint32_t i = 0; i < sizeof_array(state->keyDownDuration); ++i) {
        const bool keyDown = state->keysDown[i];
        const float keyDownDur = state->keyDownDuration[i];
        state->keyDownDuration[i] = keyDown ? (keyDownDur < 0.0f ? 0.0f : keyDownDur + deltaTime) : -1.0f;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InputState_isKeyDown(int key, uint32_t modifiers, int repeat) {
    InputState* state = &s_inputState;

    assert(key >= 0 && key < (int)sizeof_array(state->keyDownDuration));

    const float t = state->keyDownDuration[key];

    if (t == 0.0f && state->modifiers == modifiers)
        return true;

    if (repeat && t > KeyRepeatDelay) {
        float delay = KeyRepeatDelay, rate = KeyRepeatRate;
        if ((fmodf(t - delay, rate) > rate * 0.5f) != (fmodf(t - delay - state->deltaTime, rate) > rate * 0.5f)) {
            if (state->modifiers == modifiers)
                return true;
        }
    }

    return false;
}

