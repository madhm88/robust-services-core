//==============================================================================
//
//  Factory.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "Factory.h"
#include <ostream>
#include <string>
#include "Algorithms.h"
#include "Debug.h"
#include "Dynamic.h"
#include "FactoryRegistry.h"
#include "Formatters.h"
#include "Singleton.h"
#include "Statistics.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace SessionBase
{
//  Statistics for each factory.
//
class FactoryStats : public Dynamic
{
public:
   FactoryStats();
   ~FactoryStats();

   CounterPtr       icMsgsIntra_;
   CounterPtr       icMsgsInter_;
   HighWatermarkPtr icMsgSize_;
   CounterPtr       ogMsgsIntra_;
   CounterPtr       ogMsgsInter_;
   HighWatermarkPtr ogMsgSize_;
   CounterPtr       contexts_;
   CounterPtr       msgsDeleted_;
   CounterPtr       ctxsDeleted_;
};

//------------------------------------------------------------------------------

fn_name FactoryStats_ctor = "FactoryStats.ctor";

FactoryStats::FactoryStats()
{
   Debug::ft(FactoryStats_ctor);

   icMsgsIntra_.reset(new Counter("incoming intraprocessor messages"));
   icMsgsInter_.reset(new Counter("incoming interprocessor messages"));
   icMsgSize_.reset(new HighWatermark("longest incoming message"));
   ogMsgsIntra_.reset(new Counter("outgoing intraprocessor messages"));
   ogMsgsInter_.reset(new Counter("outgoing interprocessor messages"));
   ogMsgSize_.reset(new HighWatermark("longest outgoing message"));
   contexts_.reset(new Counter("contexts created"));
   msgsDeleted_.reset(new Counter("retransmitted messages deleted"));
   ctxsDeleted_.reset(new Counter("contexts freed on request-cancel"));
}

//------------------------------------------------------------------------------

fn_name FactoryStats_dtor = "FactoryStats.dtor";

FactoryStats::~FactoryStats()
{
   Debug::ft(FactoryStats_dtor);
}

//==============================================================================

fn_name Factory_ctor = "Factory.ctor";

Factory::Factory(Id fid, ContextType type, ProtocolId prid, const char* name) :
   type_(type),
   faction_(PayloadFaction),
   prid_(prid),
   name_(name)
{
   Debug::ft(Factory_ctor);

   Debug::Assert(name_ != nullptr);

   fid_.SetId(fid);
   stats_.reset(new FactoryStats);
   for(auto i = 0; i <= Signal::MaxId; ++i) icSignals_[i] = false;
   for(auto i = 0; i <= Signal::MaxId; ++i) ogSignals_[i] = false;

   //  Add the factory to the global factory registry.
   //
   Singleton< FactoryRegistry >::Instance()->BindFactory(*this);
}

//------------------------------------------------------------------------------

fn_name Factory_dtor = "Factory.dtor";

Factory::~Factory()
{
   Debug::ft(Factory_dtor);

   Singleton< FactoryRegistry >::Instance()->UnbindFactory(*this);
}

//------------------------------------------------------------------------------

fn_name Factory_AddIncomingSignal = "Factory.AddIncomingSignal";

void Factory::AddIncomingSignal(SignalId sid)
{
   Debug::ft(Factory_AddIncomingSignal);

   if(!Signal::IsValidId(sid))
   {
      Debug::SwErr(Factory_AddIncomingSignal, sid, 0);
   }

   icSignals_[sid] = true;
}

//------------------------------------------------------------------------------

fn_name Factory_AddOutgoingSignal = "Factory.AddOutgoingSignal";

void Factory::AddOutgoingSignal(SignalId sid)
{
   Debug::ft(Factory_AddOutgoingSignal);

   if(!Signal::IsValidId(sid))
   {
      Debug::SwErr(Factory_AddOutgoingSignal, sid, 0);
   }

   ogSignals_[sid] = true;
}

//------------------------------------------------------------------------------

fn_name Factory_AllocContext = "Factory.AllocContext";

Context* Factory::AllocContext() const
{
   Debug::ft(Factory_AllocContext);

   //  This must be implemented by subclasses.
   //
   Debug::SwErr(Factory_AllocContext, Fid(), 0);
   return nullptr;
}

//------------------------------------------------------------------------------

fn_name Factory_AllocIcMsg = "Factory.AllocIcMsg";

Message* Factory::AllocIcMsg(SbIpBufferPtr& buff) const
{
   Debug::ft(Factory_AllocIcMsg);

   //  This must be implemented by a subclass if required.
   //
   Debug::SwErr(Factory_AllocIcMsg, Fid(), 0);
   return nullptr;
}

//------------------------------------------------------------------------------

fn_name Factory_AllocOgMsg = "Factory.AllocOgMsg";

Message* Factory::AllocOgMsg(SignalId sid) const
{
   Debug::ft(Factory_AllocOgMsg);

   //  This must be implemented by a subclass if required.
   //
   Debug::SwErr(Factory_AllocOgMsg, Fid(), 0);
   return nullptr;
}

//------------------------------------------------------------------------------

ptrdiff_t Factory::CellDiff()
{
   int local;
   auto fake = reinterpret_cast< const Factory* >(&local);
   return ptrdiff(&fake->fid_, fake);
}

//------------------------------------------------------------------------------

fn_name Factory_CreateText = "Factory.CreateText";

CliText* Factory::CreateText() const
{
   Debug::ft(Factory_CreateText);

   return nullptr;
}

//------------------------------------------------------------------------------

size_t Factory::DiscardedContextCount() const
{
   return stats_->ctxsDeleted_->Curr();
}

//------------------------------------------------------------------------------

size_t Factory::DiscardedMessageCount() const
{
   return stats_->msgsDeleted_->Curr();
}

//------------------------------------------------------------------------------

void Factory::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Protected::Display(stream, prefix, options);

   stream << prefix << "fid     : " << fid_.to_str() << CRLF;
   stream << prefix << "type    : " << int(type_) << CRLF;
   stream << prefix << "faction : " << int(faction_) << CRLF;
   stream << prefix << "prid    : " << prid_ << CRLF;
   stream << prefix << "name    : " << name_ << CRLF;
   stream << prefix << "icSignals : ";

   for(auto i = 0; i <= Signal::MaxId; ++i)
   {
      if(icSignals_[i]) stream << int(i) << SPACE;
   }

   stream << CRLF;
   stream << prefix << "ogSignals : ";

   for(auto i = 0; i <= Signal::MaxId; ++i)
   {
      if(ogSignals_[i]) stream << int(i) << SPACE;
   }

   stream << CRLF;
}

//------------------------------------------------------------------------------

fn_name Factory_DisplayStats = "Factory.DisplayStats";

void Factory::DisplayStats(ostream& stream) const
{
   Debug::ft(Factory_DisplayStats);

   stream << spaces(2) << name_ << SPACE << strIndex(Fid(), 0, false) << CRLF;

   stats_->icMsgsIntra_->DisplayStat(stream);
   stats_->icMsgsInter_->DisplayStat(stream);
   stats_->icMsgSize_->DisplayStat(stream);
   stats_->ogMsgsIntra_->DisplayStat(stream);
   stats_->ogMsgsInter_->DisplayStat(stream);
   stats_->ogMsgSize_->DisplayStat(stream);
   stats_->contexts_->DisplayStat(stream);
   stats_->msgsDeleted_->DisplayStat(stream);
   stats_->ctxsDeleted_->DisplayStat(stream);
}

//------------------------------------------------------------------------------

fn_name Factory_IncrContexts = "Factory.IncrContexts";

void Factory::IncrContexts() const
{
   Debug::ft(Factory_IncrContexts);

   stats_->contexts_->Incr();
}

//------------------------------------------------------------------------------

fn_name Factory_InjectMsg = "Factory.InjectMsg";

bool Factory::InjectMsg(Message& msg) const
{
   Debug::ft(Factory_InjectMsg);

   //  This must be implemented by a subclass if required.
   //
   Debug::SwErr(Factory_InjectMsg, Fid(), 0);
   return false;
}

//------------------------------------------------------------------------------

fn_name Factory_IsLegalIcSignal = "Factory.IsLegalIcSignal";

bool Factory::IsLegalIcSignal(SignalId sid) const
{
   Debug::ft(Factory_IsLegalIcSignal);

   if(!Signal::IsValidId(sid)) return false;
   return icSignals_[sid];
}

//------------------------------------------------------------------------------

fn_name Factory_IsLegalOgSignal = "Factory.IsLegalOgSignal";

bool Factory::IsLegalOgSignal(SignalId sid) const
{
   Debug::ft(Factory_IsLegalOgSignal);

   if(!Signal::IsValidId(sid)) return false;
   return ogSignals_[sid];
}

//------------------------------------------------------------------------------

void Factory::Patch(sel_t selector, void* arguments)
{
   Protected::Patch(selector, arguments);
}

//------------------------------------------------------------------------------

fn_name Factory_ReallocOgMsg = "Factory.ReallocOgMsg";

Message* Factory::ReallocOgMsg(SbIpBufferPtr& buff) const
{
   Debug::ft(Factory_ReallocOgMsg);

   //  This must be implemented by a subclass if required.
   //
   Debug::SwErr(Factory_ReallocOgMsg, Fid(), 0);
   return nullptr;
}

//------------------------------------------------------------------------------

fn_name Factory_ReceiveMsg = "Factory.ReceiveMsg";

Factory::Rc Factory::ReceiveMsg
   (Message& msg, bool atIoLevel, TransTrace* tt, Context*& ctx)
{
   Debug::ft(Factory_ReceiveMsg);

   //  This must be implemented by a subclass if required.
   //
   Debug::SwErr(Factory_ReceiveMsg, Fid(), 0);
   return FactoryNotReceiving;
}

//------------------------------------------------------------------------------

fn_name Factory_RecordDeletion = "Factory.RecordDeletion";

void Factory::RecordDeletion(bool context) const
{
   Debug::ft(Factory_RecordDeletion);

   if(context)
      stats_->ctxsDeleted_->Incr();
   else
      stats_->msgsDeleted_->Incr();
}

//------------------------------------------------------------------------------

fn_name Factory_RecordMsg = "Factory.RecordMsg";

void Factory::RecordMsg(bool incoming, bool inter, size_t size) const
{
   Debug::ft(Factory_RecordMsg);

   if(incoming)
   {
      if(inter)
         stats_->icMsgsInter_->Incr();
      else
         stats_->icMsgsIntra_->Incr();

      stats_->icMsgSize_->Update(size);
   }
   else
   {
      if(inter)
         stats_->ogMsgsInter_->Incr();
      else
         stats_->ogMsgsIntra_->Incr();

      stats_->ogMsgSize_->Update(size);
   }
}

//------------------------------------------------------------------------------

fn_name Factory_ScreenIcMsgs = "Factory.ScreenIcMsgs";

bool Factory::ScreenIcMsgs(Q1Way< Message >& msgq)
{
   Debug::ft(Factory_ScreenIcMsgs);

   return true;
}

//------------------------------------------------------------------------------

fn_name Factory_Shutdown = "Factory.Shutdown";

void Factory::Shutdown(RestartLevel level)
{
   Debug::ft(Factory_Shutdown);

   if(level < RestartCold) return;

   stats_.release();
}

//------------------------------------------------------------------------------

fn_name Factory_Startup = "Factory.Startup";

void Factory::Startup(RestartLevel level)
{
   Debug::ft(Factory_Startup);

   if(stats_ == nullptr) stats_.reset(new FactoryStats);
}
}