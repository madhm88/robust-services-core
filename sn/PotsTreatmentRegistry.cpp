//==============================================================================
//
//  PotsTreatmentRegistry.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "PotsTreatmentRegistry.h"
#include <ostream>
#include <string>
#include "Debug.h"
#include "Formatters.h"
#include "SysTypes.h"
#include "Tones.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace PotsBase
{
fn_name PotsTreatmentRegistry_ctor = "PotsTreatmentRegistry.ctor";

PotsTreatmentRegistry::PotsTreatmentRegistry()
{
   Debug::ft(PotsTreatmentRegistry_ctor);

   for(auto c = 0; c <= Cause::MaxInd; ++c) causeToQId_[c] = NIL_ID;
   treatmentqs_.Init
      (PotsTreatmentQueue::MaxQId, PotsTreatmentQueue::CellDiff(), MemProt);
}

//------------------------------------------------------------------------------

fn_name PotsTreatmentRegistry_dtor = "PotsTreatmentRegistry.dtor";

PotsTreatmentRegistry::~PotsTreatmentRegistry()
{
   Debug::ft(PotsTreatmentRegistry_dtor);
}

//------------------------------------------------------------------------------

fn_name PotsTreatmentRegistry_BindTreatmentQ =
   "PotsTreatmentRegistry.BindTreatmentQ";

bool PotsTreatmentRegistry::BindTreatmentQ(PotsTreatmentQueue& treatmentq)
{
   Debug::ft(PotsTreatmentRegistry_BindTreatmentQ);

   return treatmentqs_.Insert(treatmentq);
}

//------------------------------------------------------------------------------

fn_name PotsTreatmentRegistry_CauseToTreatmentQ =
   "PotsTreatmentRegistry.CauseToTreatmentQ";

PotsTreatmentQueue* PotsTreatmentRegistry::CauseToTreatmentQ
   (Cause::Ind cause) const
{
   Debug::ft(PotsTreatmentRegistry_CauseToTreatmentQ);

   if(cause > Cause::MaxInd)
   {
      Debug::SwErr(PotsTreatmentRegistry_CauseToTreatmentQ, cause, 0);
      return nullptr;
   }

   auto qid = causeToQId_[cause];
   return treatmentqs_.At(qid);
}

//------------------------------------------------------------------------------

void PotsTreatmentRegistry::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Protected::Display(stream, prefix, options);

   stream << prefix << "treatmentqs [QId]" << CRLF;
   treatmentqs_.Display(stream, prefix + spaces(2), options);

   stream << prefix << "causeToQId [Cause::Ind]" << CRLF;

   for(auto c = 0; c <= Cause::MaxInd; ++c)
   {
      auto lead = prefix + spaces(2);

      if(causeToQId_[c] != NIL_ID)
      {
         stream << lead << strIndex(c, 3);
         stream << int(causeToQId_[c]) << CRLF;
      }
   }
}

//------------------------------------------------------------------------------

fn_name PotsTreatmentRegistry_SetCauseToTreatmentQ =
   "PotsTreatmentRegistry.SetCauseToTreatmentQ";

void PotsTreatmentRegistry::SetCauseToTreatmentQ
   (Cause::Ind cause, PotsTreatmentQueue::QId qid)
{
   if(cause > Cause::MaxInd)
   {
      Debug::SwErr(PotsTreatmentRegistry_SetCauseToTreatmentQ, cause, 0);
      return;
   }

   causeToQId_[cause] = qid;
}

//------------------------------------------------------------------------------

fn_name PotsTreatmentRegistry_Startup = "PotsTreatmentRegistry.Startup";

void PotsTreatmentRegistry::Startup(RestartLevel level)
{
   Debug::ft(PotsTreatmentRegistry_Startup);

   //  Here queues are defined statically, but a production system would
   //  allow them to be provisioned dynamically, along with the cause to
   //  treatment queue mappings.
   //
   if(level < RestartReload) return;

   auto IdleQId = PotsTreatmentQueue::IdleQId;
   new PotsTreatmentQueue(IdleQId);
   new PotsIdleTreatment(IdleQId);

   auto DisconnectQId = PotsTreatmentQueue::DisconnectQId;
   new PotsTreatmentQueue(DisconnectQId);
   new PotsToneTreatment(DisconnectQId, Tone::Silence, 10);
   new PotsToneTreatment(DisconnectQId, Tone::Reorder, 20);
   new PotsToneTreatment(DisconnectQId, Tone::ReceiverOffHook, 40);
   new PotsLockoutTreatment(DisconnectQId);

   auto BusyQId = PotsTreatmentQueue::BusyQId;
   new PotsTreatmentQueue(BusyQId);
   new PotsToneTreatment(BusyQId, Tone::Busy, 20);
   new PotsToneTreatment(BusyQId, Tone::Reorder, 20);
   new PotsToneTreatment(BusyQId, Tone::ReceiverOffHook, 40);
   new PotsLockoutTreatment(BusyQId);

   auto ErrorQId = PotsTreatmentQueue::ErrorQId;
   new PotsTreatmentQueue(ErrorQId);
   new PotsToneTreatment(ErrorQId, Tone::Reorder, 20);
   new PotsToneTreatment(ErrorQId, Tone::ReceiverOffHook, 40);
   new PotsLockoutTreatment(ErrorQId);

   auto ConfQId = PotsTreatmentQueue::ConfQId;
   new PotsTreatmentQueue(ConfQId);
   new PotsToneTreatment(ConfQId, Tone::Confirmation, 5);
   new PotsToneTreatment(ConfQId, Tone::Silence, 10);
   new PotsToneTreatment(ConfQId, Tone::Reorder, 20);
   new PotsToneTreatment(ConfQId, Tone::ReceiverOffHook, 40);
   new PotsLockoutTreatment(ConfQId);

   SetCauseToTreatmentQ(Cause::UnallocatedNumber, DisconnectQId);
   SetCauseToTreatmentQ(Cause::Confirmation, ConfQId);
   SetCauseToTreatmentQ(Cause::AddressTimeout, ErrorQId);
   SetCauseToTreatmentQ(Cause::NormalCallClearing, DisconnectQId);
   SetCauseToTreatmentQ(Cause::UserBusy, BusyQId);
   SetCauseToTreatmentQ(Cause::AlertingTimeout, DisconnectQId);
   SetCauseToTreatmentQ(Cause::AnswerTimeout, ErrorQId);
   SetCauseToTreatmentQ(Cause::ExchangeRoutingError, IdleQId);
   SetCauseToTreatmentQ(Cause::DestinationOutOfOrder, DisconnectQId);
   SetCauseToTreatmentQ(Cause::InvalidAddress, ErrorQId);
   SetCauseToTreatmentQ(Cause::FacilityRejected, ErrorQId);
   SetCauseToTreatmentQ(Cause::TemporaryFailure, IdleQId);
   SetCauseToTreatmentQ(Cause::OutgoingCallsBarred, ErrorQId);
   SetCauseToTreatmentQ(Cause::IncomingCallsBarred, DisconnectQId);
   SetCauseToTreatmentQ(Cause::CallRedirected, ErrorQId);
   SetCauseToTreatmentQ(Cause::ExcessiveRedirection, ErrorQId);
   SetCauseToTreatmentQ(Cause::MessageInvalidForState, IdleQId);
   SetCauseToTreatmentQ(Cause::ParameterAbsent, IdleQId);
   SetCauseToTreatmentQ(Cause::ProtocolTimeout, IdleQId);
   SetCauseToTreatmentQ(Cause::ResetCircuit, IdleQId);
}

//------------------------------------------------------------------------------

PotsTreatmentQueue* PotsTreatmentRegistry::TreatmentQ
   (PotsTreatmentQueue::QId qid) const
{
   return treatmentqs_.At(qid);
}

//------------------------------------------------------------------------------

fn_name PotsTreatmentRegistry_UnbindTreatmentQ =
   "PotsTreatmentRegistry.UnbindTreatmentQ";

void PotsTreatmentRegistry::UnbindTreatmentQ(PotsTreatmentQueue& treatmentq)
{
   Debug::ft(PotsTreatmentRegistry_UnbindTreatmentQ);

   treatmentqs_.Erase(treatmentq);
}
}