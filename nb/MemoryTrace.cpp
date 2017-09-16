//==============================================================================
//
//  MemoryTrace.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "MemoryTrace.h"
#include <ostream>
#include <string>
#include "Formatters.h"
#include "Singleton.h"
#include "ToolTypes.h"
#include "TraceBuffer.h"
#include "TraceDump.h"

using std::ostream;

//------------------------------------------------------------------------------

namespace NodeBase
{
MemoryTrace::MemoryTrace(Id rid, const void* addr, MemoryType type,
   size_t size) : TimedRecord(sizeof(MemoryTrace), MemoryTracer),
   addr_(addr),
   type_(type),
   size_(size)
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

bool MemoryTrace::Display(ostream& stream)
{
   if(!TimedRecord::Display(stream)) return false;

   stream << spaces(TraceDump::EvtToObj) << addr_ << TraceDump::Tab();
   stream << "type=" << TypeString(type_) << spaces(3);
   stream << "size=" << size_;

   if(rid_ == Free) return true;

   //  If there is no record of this memory being freed, flag it.
   //
   auto buff = Singleton< TraceBuffer >::Instance();
   auto mask = Flags(1 << MemoryTracer);
   TraceRecord* rec  = this;

   for(buff->Next(rec, mask); rec != nullptr; buff->Next(rec, mask))
   {
      auto curr = static_cast< MemoryTrace* >(rec);

      if((curr->addr_ == addr_) && (curr->rid_ == Free)) return true;
   }

   stream << " NOT FREED";
   return true;
}

//------------------------------------------------------------------------------

fixed_string AllocEventStr = " +mem";
fixed_string FreeEventStr  = " -mem";

const char* MemoryTrace::EventString() const
{
   switch(rid_)
   {
   case Alloc: return AllocEventStr;
   case Free: return FreeEventStr;
   }

   return ERROR_STR;
}

//------------------------------------------------------------------------------

fixed_string MemTypeStrings[MemoryType_N + 1] =
{
   ERROR_STR,
   "temp",
   "dyn ",
   "prot",
   "perm",
   "imm ",
   ERROR_STR
};

const char* MemoryTrace::TypeString(MemoryType type)
{
   if((type >= 0) && (type < MemoryType_N)) return MemTypeStrings[type];
   return MemTypeStrings[MemoryType_N];
}
}