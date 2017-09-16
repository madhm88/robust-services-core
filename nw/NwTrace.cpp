//==============================================================================
//
//  NwTrace.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "NwTrace.h"
#include <ostream>
#include <string>
#include "Formatters.h"
#include "ToolTypes.h"
#include "TraceDump.h"

using std::ostream;

//------------------------------------------------------------------------------

namespace NodeBase
{
NwTrace::NwTrace(Id rid, const SysSocket* socket, word data) :
   TimedRecord(sizeof(NwTrace), NetworkTracer),
   socket_(socket),
   data_(data),
   port_(NilIpPort)
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

NwTrace::NwTrace(Id rid, const SysSocket* socket, word data, ipport_t port) :
   TimedRecord(sizeof(NwTrace), NetworkTracer),
   socket_(socket),
   data_(data),
   port_(port)
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

NwTrace::NwTrace(Id rid, const SysSocket* socket, word data, ipport_t port,
   const SysIpL3Addr& peer) : TimedRecord(sizeof(NwTrace), NetworkTracer),
   socket_(socket),
   data_(data),
   port_(port),
   peer_(peer)
{
   rid_ = rid;
}

//------------------------------------------------------------------------------

bool NwTrace::Display(ostream& stream)
{
   if(!TimedRecord::Display(stream)) return false;

   stream << spaces(TraceDump::EvtToObj);

   if(socket_ != nullptr)
      stream << socket_;
   else
      stream << spaces(NIBBLES_PER_POINTER);

   stream << TraceDump::Tab();

   switch(rid_)
   {
   case Acquire:
   case Release:
   case Register:
   case Deregister:
   case Queue:
   case Dispatch:
   case Purge:
   case Delete:
      stream << "state=" << data_;
      break;
   case Connect:
      stream << "peer=" << peer_.to_str() << " rc=" << data_;
      break;
   case Listen:
      stream << "port=" << port_ << " backlog=" << data_;
      break;
   case Poll:
      stream << "ready=" << data_;
      break;
   case Accept:
      stream << "port=" << port_ << " peer=" << peer_.to_str();
      break;
   case Recv:
      stream << "rcvd=" << data_;
      break;
   case RecvFrom:
      stream << "rcvd=" << data_;
      stream << " port=" << port_ << " peer=" << peer_.to_str();
      break;
   case Send:
      stream << "sent=" << data_;
      break;
   case SendTo:
      stream << "sent=" << data_;
      stream << " port=" << port_ << " peer=" << peer_.to_str();
      break;
   case Disconnect:
      break;
   case Close:
      stream << "hard=" << (data_ == 0);
      break;
   }

   return true;
}

//------------------------------------------------------------------------------

fixed_string AcquireEventStr    = " +app";
fixed_string ReleaseEventStr    = " -app";
fixed_string RegisterEventStr   = " +iot";
fixed_string DeregisterEventStr = " -iot";
fixed_string ConnectEventStr    = " conn";
fixed_string ListenEventStr     = " lstn";
fixed_string PollEventStr       = " poll";
fixed_string AcceptEventStr     = " acpt";
fixed_string RecvEventStr       = " recv";
fixed_string RecvFromEventStr   = "urecv";
fixed_string QueueEventStr      = "queue";
fixed_string DispatchEventStr   = " disp";
fixed_string SendEventStr       = " send";
fixed_string SendToEventStr     = "usend";
fixed_string DisconnectEventStr = " disc";
fixed_string CloseEventStr      = "close";
fixed_string PurgeEventStr      = "purge";
fixed_string DeleteEventStr     = "-sock";

const char* NwTrace::EventString() const
{
   switch(rid_)
   {
   case Acquire: return AcquireEventStr;
   case Release: return ReleaseEventStr;
   case Register: return RegisterEventStr;
   case Deregister: return DeregisterEventStr;
   case Connect: return ConnectEventStr;
   case Listen: return ListenEventStr;
   case Poll: return PollEventStr;
   case Accept: return AcceptEventStr;
   case Recv: return RecvEventStr;
   case RecvFrom: return RecvFromEventStr;
   case Queue: return QueueEventStr;
   case Dispatch: return DispatchEventStr;
   case Send: return SendEventStr;
   case SendTo: return SendToEventStr;
   case Disconnect: return DisconnectEventStr;
   case Close: return CloseEventStr;
   case Purge: return PurgeEventStr;
   case Delete: return DeleteEventStr;
   }

   return ERROR_STR;
}
}