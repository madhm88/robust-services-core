//==============================================================================
//
//  Initiator.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef INITIATOR_H_INCLUDED
#define INITIATOR_H_INCLUDED

#include "Protected.h"
#include <cstddef>
#include <cstdint>
#include "EventHandler.h"
#include "Q1Link.h"
#include "SbTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace SessionBase
{
//  An Initiator requests the creation of a ServiceSM which then modifies the
//  behavior of a root service.  A modifier registers an Initiator with each
//  trigger (usually one) that it uses in order to observe its root service's
//  behavior and create its own ServiceSM at the appropriate time.
//
class Initiator : public Protected
{
   friend class Q1Way< Initiator >;
public:
   //  Each initiator specifies a priority when it registers with its trigger.
   //  The trigger queues initiators in descending order of priority, meaning
   //  that an initiator with a higher priority will get the chance to request
   //  the creation of its modifier before an initiator with a lower priority.
   //  This is important in resolving service interactions.  All priorities
   //  must be defined in the interface that defines the associated trigger.
   //
   typedef uint8_t Priority;

   //  Returns the service associated with the initiator.
   //
   ServiceId Sid() const { return sid_; }

   //  Returns the initiator's priority.
   //
   Priority GetPriority() const { return prio_; }

   //  Invokes the initiator's ProcessEvent function.
   //
   EventHandler::Rc InvokeHandler
      (const ServiceSM& parentSsm, Event& icEvent, Event*& ogEvent) const;

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
protected:
   //  Sets the corresponding member variables.  Adds the initiator to
   //  the trigger identified by AID and TID, which must already exist.
   //  SID is the initiator's service, and PRIO is its priority with
   //  respect to other services that use the same trigger.  Protected
   //  because this class is virtual.
   //
   Initiator(ServiceId sid, ServiceId aid, TriggerId tid, Priority prio);

   //  Removes the initiator from its trigger.  Protected to restrict
   //  deletion.
   //
   virtual ~Initiator();
private:
   //  Overridden to prohibit copying.
   //
   Initiator(const Initiator& that);
   void operator=(const Initiator& that);

   //  The initiator's event handler, which receives either an SAP or SNP,
   //  depending on the trigger with which it has registered.  It can either
   //  pass icEvent onwards or create an InitiationReqEvent and return it in
   //  ogEvent to request the creation of its ServiceSM.  The default version
   //  generates a log and returns EventHandler::Pass and must be overridden.
   //
   virtual EventHandler::Rc ProcessEvent
      (const ServiceSM& parentSsm, Event& icEvent, Event*& ogEvent) const;

   //  Returns the trigger where the initiator is located.
   //
   Trigger* GetTrigger() const;

   //  Used by InvokeHandler to clean up when an error is detected
   //  during event processing.
   //
   EventHandler::Rc EventError(Event*& evt, EventHandler::Rc rc) const;

   //  The service associated with the initiator.
   //
   ServiceId sid_;

   //  The service associated with the trigger (the initiator's ancestor).
   //
   ServiceId aid_;

   //  The trigger associated with the initiator.
   //
   TriggerId tid_;

   //  The initiator's priority.
   //
   const Priority prio_;

   //  The next initiator in the trigger's queue of initiators.
   //
   Q1Link link_;
};
}
#endif