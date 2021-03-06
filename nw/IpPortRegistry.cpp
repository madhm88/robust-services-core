//==============================================================================
//
//  IpPortRegistry.cpp
//
//  Copyright (C) 2017  Greg Utas
//
//  This file is part of the Robust Services Core (RSC).
//
//  RSC is free software: you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software
//  Foundation, either version 3 of the License, or (at your option) any later
//  version.
//
//  RSC is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
//  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
//  details.
//
//  You should have received a copy of the GNU General Public License along
//  with RSC.  If not, see <http://www.gnu.org/licenses/>.
//
#include "IpPortRegistry.h"
#include "CfgStrParm.h"
#include "StatisticsGroup.h"
#include <ostream>
#include <string>
#include "Algorithms.h"
#include "CfgParmRegistry.h"
#include "Debug.h"
#include "Formatters.h"
#include "FunctionGuard.h"
#include "IpPort.h"
#include "IpService.h"
#include "NwCliParms.h"
#include "Restart.h"
#include "Singleton.h"
#include "SysIpL3Addr.h"
#include "SysTypes.h"

using namespace NodeBase;
using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace NetworkBase
{
//  Configuration parameter for the host IP address.
//
class HostAddrCfg : public CfgStrParm
{
public:
   explicit HostAddrCfg(ProtectedStr* field);
   ~HostAddrCfg();
   SysIpL2Addr Address() const { return addr_; }
protected:
   void SetCurr() override;
private:
   bool SetNext(c_string input) override;

   //  Kept in synch with the string version of the element's address.
   //
   SysIpL2Addr addr_;
};

//------------------------------------------------------------------------------

fn_name HostAddrCfg_ctor = "HostAddrCfg.ctor";

HostAddrCfg::HostAddrCfg(ProtectedStr* field) : CfgStrParm("ElementDefaultAddr",
   "127.0.0.1", field, "element's default IP address (n.n.n.n)")
{
   Debug::ft(HostAddrCfg_ctor);
}

//------------------------------------------------------------------------------

fn_name HostAddrCfg_dtor = "HostAddrCfg.dtor";

HostAddrCfg::~HostAddrCfg()
{
   Debug::ftnt(HostAddrCfg_dtor);
}

//------------------------------------------------------------------------------

fn_name HostAddrCfg_SetCurr = "HostAddrCfg.SetCurr";

void HostAddrCfg::SetCurr()
{
   Debug::ft(HostAddrCfg_SetCurr);

   FunctionGuard guard(Guard_MemUnprotect);
   CfgStrParm::SetCurr();
   addr_ = SysIpL2Addr(GetCurr());
}

//------------------------------------------------------------------------------

fn_name HostAddrCfg_SetNext = "HostAddrCfg.SetNext";

bool HostAddrCfg::SetNext(c_string input)
{
   Debug::ft(HostAddrCfg_SetNext);

   SysIpL2Addr addr(input);
   if(!addr.IsValid()) return false;
   return CfgStrParm::SetNext(input);
}

//==============================================================================

class IpPortStatsGroup : public StatisticsGroup
{
public:
   IpPortStatsGroup();
   ~IpPortStatsGroup();
   void DisplayStats
      (ostream& stream, id_t id, const Flags& options) const override;
};

//------------------------------------------------------------------------------

fn_name IpPortStatsGroup_ctor = "IpPortStatsGroup.ctor";

IpPortStatsGroup::IpPortStatsGroup() : StatisticsGroup("IpPorts [ipport_t]")
{
   Debug::ft(IpPortStatsGroup_ctor);
}

//------------------------------------------------------------------------------

fn_name IpPortStatsGroup_dtor = "IpPortStatsGroup.dtor";

IpPortStatsGroup::~IpPortStatsGroup()
{
   Debug::ftnt(IpPortStatsGroup_dtor);
}

//------------------------------------------------------------------------------

fn_name IpPortStatsGroup_DisplayStats = "IpPortStatsGroup.DisplayStats";

void IpPortStatsGroup::DisplayStats
   (ostream& stream, id_t id, const Flags& options) const
{
   Debug::ft(IpPortStatsGroup_DisplayStats);

   StatisticsGroup::DisplayStats(stream, id, options);

   auto reg = Singleton< IpPortRegistry >::Instance();

   if(id == 0)
   {
      auto& ports = reg->Ports();

      for(auto p = ports.First(); p != nullptr; ports.Next(p))
      {
         p->DisplayStats(stream, options);
      }
   }
   else
   {
      auto p = reg->GetPort(id);

      if(p == nullptr)
      {
         stream << spaces(2) << NoIpPortExpl << CRLF;
         return;
      }

      p->DisplayStats(stream, options);
   }
}

//==============================================================================

fn_name IpPortRegistry_ctor = "IpPortRegistry.ctor";

IpPortRegistry::IpPortRegistry() : hostAddrStr_("127.0.0.1")
{
   Debug::ft(IpPortRegistry_ctor);

   portq_.Init(IpPort::LinkDiff());
   hostAddrCfg_.reset(new HostAddrCfg(&hostAddrStr_));
   Singleton< CfgParmRegistry >::Instance()->BindParm(*hostAddrCfg_);
   statsGroup_.reset(new IpPortStatsGroup);
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_dtor = "IpPortRegistry.dtor";

IpPortRegistry::~IpPortRegistry()
{
   Debug::ftnt(IpPortRegistry_dtor);

   Debug::SwLog(IpPortRegistry_dtor, UnexpectedInvocation, 0);
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_BindPort = "IpPortRegistry.BindPort";

bool IpPortRegistry::BindPort(IpPort& port)
{
   Debug::ft(IpPortRegistry_BindPort);

   //  Sort entries by port number.  Generate a log and reject overbinding.
   //
   auto pid = port.GetPort();

   IpPort* prev = nullptr;
   IpPort* curr = portq_.First();

   //  Skip lower-numbered ports.
   //
   while(curr != nullptr)
   {
      if(curr->GetPort() >= pid) break;
      prev = curr;
      portq_.Next(curr);
   }

   auto newpro = port.GetService()->Protocol();

   while(curr != nullptr)
   {
      if(curr->GetPort() > pid) break;

      //  CURR is already using this port number.  This is only allowed if
      //  CURR supports UDP or TCP and the new port supports the other.
      //
      auto oldpro = curr->GetService()->Protocol();

      if((newpro == IpAny) || (oldpro == IpAny) || (newpro == oldpro))
      {
         Debug::SwLog(IpPortRegistry_BindPort,
            "port already in use", pack3(newpro, oldpro, pid));
         return false;
      }

      prev = curr;
      portq_.Next(curr);
   }

   portq_.Insert(prev, port);
   return true;
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_CanBypassStack = "IpPortRegistry.CanBypassStack";

bool IpPortRegistry::CanBypassStack
   (const SysIpL3Addr& srce, const SysIpL3Addr& dest) const
{
   Debug::ft(IpPortRegistry_CanBypassStack);

   if((dest.GetIpV4Addr() != srce.GetIpV4Addr()) &&
      (dest.GetIpV4Addr() != SysIpL2Addr::LoopbackAddr().GetIpV4Addr()))
   {
      auto host = HostAddress();
      if(dest.GetIpV4Addr() != host.GetIpV4Addr()) return false;
   }

   auto port = dest.GetPort();

   return ((port == NilIpPort) || (GetPort(dest.GetPort()) != nullptr));
}

//------------------------------------------------------------------------------

void IpPortRegistry::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Protected::Display(stream, prefix, options);

   stream << prefix << "HostAddr    : " << hostAddr_.to_str() << CRLF;
   stream << prefix << "HostAddrStr : " << hostAddrStr_ << CRLF;
   stream << prefix << "hostAddrCfg : " << strObj(hostAddrCfg_.get()) << CRLF;
   stream << prefix << "statsGroup  : " << strObj(statsGroup_.get()) << CRLF;
   stream << prefix << "portq : " << CRLF;
   portq_.Display(stream, prefix + spaces(2), options);
}

//------------------------------------------------------------------------------

IpPort* IpPortRegistry::GetPort(ipport_t port, IpProtocol protocol) const
{
   for(auto p = portq_.First(); p != nullptr; portq_.Next(p))
   {
      if(p->GetPort() == port)
      {
         if(protocol == IpAny) return p;
         if(p->GetService()->Protocol() == protocol) return p;
      }
   }

   return nullptr;
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_HostAddress = "IpPortRegistry.HostAddress";

SysIpL2Addr IpPortRegistry::HostAddress()
{
   Debug::ft(IpPortRegistry_HostAddress);

   auto reg = Singleton< IpPortRegistry >::Extant();
   if(reg == nullptr) return SysIpL2Addr::LoopbackAddr();

   if(reg->hostAddr_.IsValid()) return reg->hostAddr_;

   string name;
   string service;
   IpProtocol proto;

   if(SysIpL2Addr::HostName(name))
   {
      SysIpL3Addr host(name, service, proto);

      FunctionGuard guard(Guard_MemUnprotect);
      reg->hostAddr_ = host;
      return host;
   }

   return reg->hostAddrCfg_->Address();
}

//------------------------------------------------------------------------------

void IpPortRegistry::Patch(sel_t selector, void* arguments)
{
   Protected::Patch(selector, arguments);
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_Shutdown = "IpPortRegistry.Shutdown";

void IpPortRegistry::Shutdown(RestartLevel level)
{
   Debug::ft(IpPortRegistry_Shutdown);

   for(auto p = portq_.First(); p != nullptr; portq_.Next(p))
   {
      p->Shutdown(level);
   }

   FunctionGuard guard(Guard_MemUnprotect);
   Restart::Release(statsGroup_);
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_Startup = "IpPortRegistry.Startup";

void IpPortRegistry::Startup(RestartLevel level)
{
   Debug::ft(IpPortRegistry_Startup);

   //  The host address is frequently required and may be expensive to
   //  look up.  It is therefore cached during initialization, after
   //  which a restart is required to change it.
   //
   FunctionGuard guard(Guard_MemUnprotect);
   HostAddress();
   if(statsGroup_ == nullptr) statsGroup_.reset(new IpPortStatsGroup);
   guard.Release();

   for(auto p = portq_.First(); p != nullptr; portq_.Next(p))
   {
      p->Startup(level);
   }
}

//------------------------------------------------------------------------------

fn_name IpPortRegistry_UnbindPort = "IpPortRegistry.UnbindPort";

void IpPortRegistry::UnbindPort(IpPort& port)
{
   Debug::ftnt(IpPortRegistry_UnbindPort);

   portq_.Exq(port);
}
}
