// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Aliases.h"

/* About the event handler.
 *
 * vAmiga is an event triggered emulator. If an action has to be performed at
 * a specific DMA cycle (e.g., activating the Copper at a certain beam
 * position), the action is scheduled via the event handling API and executed
 * when the trigger cycle is reached.
 * The event handler is part of Agnus, because this component is in charge of
 * synchronize timing between components.
 * Scheduled events are stored in so called event slots. Each slot is either
 * empty or contains a single event and is bound to a specific component. E.g.,
 * there is slot for Copper events, a slot for the Blitter events, and a slot
 * for managing UART event.
 * From a theoretical point of view, each event slot represents a state machine
 * running in parallel to the ones in the other slots. Keep in mind that the
 * state machines do interact with each other in various ways (e.g., by
 * blocking the DMA bus). As a result, the slot ordering is important: If two
 * events trigger at the same cycle, the the slot with a smaller number is
 * always served first.
 * To optimize speed, the event slots are categorized into primary slots and
 * secondary slots. The primary slots are those that who store frequently
 * occurring events (CIA execution, DMA operations, etc.) and the secondary
 * slots are those who store events that only occurr occasionally (e.g., a
 * signal change on the serial port). Correspondingly, we call an event a
 * primary event if if it scheduled in a primary slot and a secondary event if
 * it is called in a secondary slot.
 * By default, the event handler only checks the primary event slots on a
 * regular basis. To make the event handler check all slots, a special event
 * has to be scheduled in the SEC_SLOT (which is a primary slot and therefore
 * always checked). Triggering this event works like a wakeup by telling the
 * event handler to check for secondary events as well. Hence, whenever an
 * event is schedules in a secondary slot, it has to be ensured that SEC_SLOT
 * contains a wakeup with a trigger cycle matching the smallest trigger cycle
 * of all secondary events.
 * Scheduling the wakeup event in SEC_SLOT is transparant for the callee. When
 * an event is scheduled, the event handler automatically checks if the
 * selected slot is primary or secondary and schedules the SEC_SLOT
 * automatically in the latter case.
 */

public:

// Returns true iff the specified slot contains any event
template<EventSlot s> bool hasEvent() const { return slot[s].id != (EventID)0; }

// Returns true iff the specified slot contains a specific event
template<EventSlot s> bool hasEvent(EventID id) const { return slot[s].id == id; }

// Returns true iff the specified slot contains a pending event
template<EventSlot s> bool isPending() const { return slot[s].triggerCycle != NEVER; }

// Returns true iff the specified slot contains a due event
template<EventSlot s> bool isDue(Cycle cycle) const { return cycle >= slot[s].triggerCycle; }


//
// Scheduling events
//

/* To schedule an event, an event slot, a trigger cycle, and an event id
 * need to be provided. The trigger cycle is measured in master cycles. It can
 * be specified in multiple ways:
 *
 *   Absolute (Abs):
 *   The trigger cycle is specified as an absolute value.
 *
 *   Immediate (Imm):
 *   The trigger cycle is the next DMA cycle.
 *
 *   Relative (Rel):
 *   The trigger cycle is specified relative to the current DMA clock.
 *
 *   Incremental (Inc):
 *   The trigger cycle is specified relative to the current slot value.
 *
 *   Positional (Pos):
 *   The trigger cycle is specified in form of a beam position.
*
 * Events can also be rescheduled or canceled:
 *
 *   Rescheduling means that the event ID in the selected event slot
 *   remains unchanged.
 *
 *   Canceling means that the slot is emptied by deleting the setting the
 *   event ID and the event data to zero and the trigger cycle to NEVER.
 */

public:

template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id)
{
    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;

    if (isSecondarySlot(s) && cycle < slot[SLOT_SEC].triggerCycle)
        slot[SLOT_SEC].triggerCycle = cycle;
}

template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id, i64 data)
{
    scheduleAbs<s>(cycle, id);
    slot[s].data = data;
}

template<EventSlot s> void scheduleImm(EventID id)
{
    scheduleAbs<s>(0, id);
}

template<EventSlot s> void scheduleImm(EventID id, i64 data)
{
    scheduleAbs<s>(0, id);
    slot[s].data = data;
}

template<EventSlot s> void scheduleRel(Cycle cycle, EventID id)
{
    scheduleAbs<s>(clock + cycle, id);
}

template<EventSlot s> void scheduleRel(Cycle cycle, EventID id, i64 data)
{
    scheduleAbs<s>(clock + cycle, id);
    slot[s].data = data;
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id)
{
    scheduleAbs<s>(slot[s].triggerCycle + cycle, id);
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id, i64 data)
{
    scheduleAbs<s>(slot[s].triggerCycle + cycle, id);
    slot[s].data = data;
}

template<EventSlot s> void schedulePos(i16 vpos, i16 hpos, EventID id)
{
    scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id);
}

template<EventSlot s> void schedulePos(i16 vpos, i16 hpos, EventID id, i64 data)
{
    scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id, data);
}

template<EventSlot s> void rescheduleAbs(Cycle cycle)
{
    slot[s].triggerCycle = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
     if (isSecondarySlot(s) && cycle < slot[SLOT_SEC].triggerCycle)
         slot[SLOT_SEC].triggerCycle = cycle;
}

template<EventSlot s> void rescheduleInc(Cycle cycle)
{
    rescheduleAbs<s>(slot[s].triggerCycle + cycle);
}

template<EventSlot s> void rescheduleRel(Cycle cycle)
{
    rescheduleAbs<s>(clock + cycle);
}

template<EventSlot s> void reschedulePos(i16 vpos, i16 hpos)
{
    rescheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ));
}

template<EventSlot s> void cancel()
{
    slot[s].id = (EventID)0;
    slot[s].data = 0;
    slot[s].triggerCycle = NEVER;
}


//
// Scheduling specific events
//

// Schedules the next BPL event relative to a given DMA cycle.
void scheduleNextBplEvent(i16 hpos);

// Schedules the next BPL event relative to the currently emulated DMA cycle.
void scheduleNextBplEvent() { scheduleNextBplEvent(pos.h); }

// Schedules the earliest BPL event that occurs at or after the given DMA cycle.
void scheduleBplEventForCycle(i16 hpos);

// Updates the scheduled BPL event according to the current event table.
void updateBplEvent() { scheduleBplEventForCycle(pos.h); }

// Schedules the next DAS event relative to a given DMA cycle.
void scheduleNextDasEvent(i16 hpos);

// Schedules the next DAS event relative to the currently emulated DMA cycle.
void scheduleNextDasEvent() { scheduleNextDasEvent(pos.h); }

// Schedules the earliest DAS event that occurs at or after the given DMA cycle.
void scheduleDasEventForCycle(i16 hpos);

// Updates the scheduled DAS event according to the current event table.
void updateDasEvent() { scheduleDasEventForCycle(pos.h); }

// Schedules the next register change event
void scheduleNextREGEvent();


//
// Processing events
//

private:

/* Executes the event handler up to a given master cycle.
 * This method is called inside Agnus::executeUntil().
 */
void executeEventsUntil(Cycle cycle);

// Event handlers for specific slots
template <int nr> void serviceCIAEvent();
void serviceREGEvent(Cycle until);
void serviceBPLEvent();
template <int nr> void serviceBPLEventHires();
template <int nr> void serviceBPLEventLores();
void serviceBPLEventLores();
void serviceDASEvent();
void serviceRASEvent();

public:

void serviceINSEvent();

//
// Debugging
//

private:

/* Performs some debugging checks. Won't be executed in release build.
 * The provided slot must be a slot in the primary event table.
 */
bool checkScheduledEvent(EventSlot s);
bool checkTriggeredEvent(EventSlot s);

