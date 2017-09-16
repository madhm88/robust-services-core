//==============================================================================
//
//  PotsStatistics.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "PotsStatistics.h"
#include <memory>
#include <ostream>
#include <string>
#include "Debug.h"
#include "Formatters.h"
#include "Singleton.h"
#include "Statistics.h"
#include "SysTypes.h"

using std::ostream;

//------------------------------------------------------------------------------

namespace PotsBase
{
fixed_string StatExplStrings[PotsStatistics::MaxId + 1] =
{
   "originations attempted",
   "originations abandoned",
   "terminations attempted",
   "terminations abandoned",
   "calls alerted",
   "calls answered",
   "calls resumed",
   "proxy calls attempted",
   "proxy calls answered"
};

//------------------------------------------------------------------------------

fn_name PotsStatistics_ctor = "PotsStatistics.ctor";

PotsStatistics::PotsStatistics() : StatisticsGroup("POTS Calls")
{
   Debug::ft(PotsStatistics_ctor);

   for(auto i = 0; i <= MaxId; ++i)
   {
      basicCalls_[i].reset(new Counter(StatExplStrings[i]));
   }

   for(auto i = 0; i <= Cause::MaxInd; ++i)
   {
      treatments_[i].reset(new Counter(Cause::strInd(i)));
   }
}

//------------------------------------------------------------------------------

fn_name PotsStatistics_dtor = "PotsStatistics.dtor";

PotsStatistics::~PotsStatistics()
{
   Debug::ft(PotsStatistics_dtor);
}

//------------------------------------------------------------------------------

fn_name PotsStatistics_DisplayStats = "PotsStatistics.DisplayStats";

void PotsStatistics::DisplayStats(ostream& stream, id_t id) const
{
   Debug::ft(PotsStatistics_DisplayStats);

   StatisticsGroup::DisplayStats(stream, id);

   stream << spaces(2) << "Basic Calls" << CRLF;

   for(auto i = 0; i <= MaxId; ++i)
   {
      basicCalls_[i]->DisplayStat(stream);
   }

   stream << spaces(2) << "Treatments (by Cause)" << CRLF;

   for(auto i = 0; i <= Cause::MaxInd; ++i)
   {
      treatments_[i]->DisplayStat(stream);
   }
}

//------------------------------------------------------------------------------

fn_name PotsStatistics_Incr = "PotsStatistics.Incr";

void PotsStatistics::Incr(Id id)
{
   Debug::ft(PotsStatistics_Incr);

   if(id <= MaxId)
   {
      Singleton< PotsStatistics >::Instance()->basicCalls_[id]->Incr();
      return;
   }

   Debug::SwErr(PotsStatistics_Incr, id, 0);
}

//------------------------------------------------------------------------------

fn_name PotsStatistics_IncrCause = "PotsStatistics.IncrCause";

void PotsStatistics::IncrCause(Cause::Ind cause)
{
   Debug::ft(PotsStatistics_IncrCause);

   if(cause <= Cause::MaxInd)
   {
      Singleton< PotsStatistics >::Instance()->treatments_[cause]->Incr();
      return;
   }

   Debug::SwErr(PotsStatistics_IncrCause, cause, 0);
}
}