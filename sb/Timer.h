//==============================================================================
//
//  Timer.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "Pooled.h"
#include <cstddef>
#include "Clock.h"
#include "Q1Link.h"
#include "Q2Link.h"
#include "SbTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace SessionBase
{
//  An instance of a timer running on a PSM.  See also TimerProtocol.h.
//
class Timer : public Pooled
{
   friend class ProtocolSM;
   friend class Q1Way< Timer >;
   friend class Q2Way< Timer >;
   friend class TimerRegistry;
   friend class TimerTrace;
public:
   //  Returns the timer's PSM.
   //
   ProtocolSM* Psm() const { return psm_; }

   //  Returns the offset to link_.
   //
   static ptrdiff_t LinkDiff();

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;
private:
   //  Starts a timer on PSM, owned by OWNER, identified by TID, that will
   //  expire in SECS, and repeatedly if REPEAT is true.  Private because
   //  applications create timers via ProtocolSM::StartTimer.
   //
   Timer(ProtocolSM& psm, Base& owner,
      TimerId tid, secs_t secs, bool repeat);

   //  Private because applications delete timers via ProtocolSM::StopTimer.
   //  Not subclassed.
   //
   ~Timer();

   //  Returns the timer's owner.
   //
   Base* Owner() const { return owner_; }

   //  Returns the timer's identifier.
   //
   TimerId Tid() const { return tid_; }

   //  Removes the timer from its PSM's timer queue.
   //
   void Exqueue();

   //  Removes the timer from the timer registry.
   //
   void Deregister();

   //  Used by the timer thread to send a message to the PSM associated with
   //  the timer when the timer expires.
   //
   void SendTimeout();

   //  Restarts a repetitive timer each time it expires.
   //
   void Restart();

   //  Overridden to remove the timer from the timer registry during error
   //  recovery.
   //
   virtual void Cleanup() override;

   //  Overridden to obtain a timer from its object pool.
   //
   static void* operator new(size_t size);

   //  Timer queue identifier.
   //
   typedef int QId;

   //  Nil timer queue identifier, which allows array index 0 to be used.
   //
   static const QId NilQId = -1;

   //  Maximum timer queue identifier (3600 seconds = 1 hour).  The first
   //  3601 slots in the timewheel are for timers expiring in 0 to 3600
   //  seconds, and the final slot is for long duration timers (over 3600
   //  seconds).
   //
   static const QId MaxQId = 3601;

   //  The PSM on which the timer is running.
   //
   ProtocolSM* psm_;

   //  The timer's owner.
   //
   Base* const owner_;

   //  The owner's identifier for the timer.
   //
   TimerId tid_;

   //  Set if the timer should repeatedly expire every duration_ seconds.
   //
   const bool repeat_;

   //  The queue where the timer resides in the timer registry.
   //
   QId qid_;

   //  The two-way queue link for the timer registry.
   //
   Q2Link link_;

   //  The length of the timer in seconds.
   //
   const secs_t duration_;

   //  How long until the timer expires (only used for long timers).
   //
   secs_t remaining_;
};
}
#endif