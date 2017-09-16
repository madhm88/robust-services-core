//==============================================================================
//
//  SysIpL3Addr.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "SysIpL3Addr.h"
#include <iosfwd>
#include <sstream>
#include "Debug.h"
#include "SysSocket.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace NodeBase
{
fn_name SysIpL3Addr_ctor1 = "SysIpL3Addr.ctor";

SysIpL3Addr::SysIpL3Addr() :
   port_(NilIpPort),
   proto_(IpAny),
   socket_(nullptr)
{
   Debug::ft(SysIpL3Addr_ctor1);
}

//------------------------------------------------------------------------------

fn_name SysIpL3Addr_ctor2 = "SysIpL3Addr.ctor(IPv4addr)";

SysIpL3Addr::SysIpL3Addr(ipv4addr_t v4Addr, ipport_t port,
   IpProtocol proto, SysSocket* socket) : SysIpL2Addr(v4Addr),
   port_(port),
   proto_(proto),
   socket_(socket)
{
   Debug::ft(SysIpL3Addr_ctor2);

   if(socket_ != nullptr) proto_ = socket_->Protocol();
}

//------------------------------------------------------------------------------

fn_name SysIpL3Addr_ctor3 = "SysIpL3Addr.ctor(L2addr)";

SysIpL3Addr::SysIpL3Addr(const SysIpL2Addr& l2Addr, ipport_t port,
   IpProtocol proto, SysSocket* socket) : SysIpL2Addr(l2Addr),
   port_(port),
   proto_(proto),
   socket_(socket)
{
   Debug::ft(SysIpL3Addr_ctor3);

   if(socket_ != nullptr) proto_ = socket_->Protocol();
}

//------------------------------------------------------------------------------

fn_name SysIpL3Addr_ctor4 = "SysIpL3Addr.ctor(copy)";

SysIpL3Addr::SysIpL3Addr(const SysIpL3Addr& that) : SysIpL2Addr(that),
   port_(that.port_),
   proto_(that.proto_),
   socket_(that.socket_)
{
   Debug::ft(SysIpL3Addr_ctor4);
}

//------------------------------------------------------------------------------

fn_name SysIpL3Addr_dtor = "SysIpL3Addr.dtor";

SysIpL3Addr::~SysIpL3Addr()
{
   Debug::ft(SysIpL3Addr_dtor);
}

//------------------------------------------------------------------------------

void SysIpL3Addr::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   SysIpL2Addr::Display(stream, prefix, options);

   stream << prefix << "port   : " << port_ << CRLF;
   stream << prefix << "proto  : " << proto_ << CRLF;
   stream << prefix << "socket : " << socket_ << CRLF;
}

//------------------------------------------------------------------------------

fn_name SysIpL3Addr_opAssign = "SysIpL3Addr.operator=(copy)";

SysIpL3Addr& SysIpL3Addr::operator=(const SysIpL3Addr& that)
{
   Debug::ft(SysIpL3Addr_opAssign);

   if(&that == this) return *this;
   SysIpL2Addr::operator=(that);
   this->port_ = that.port_;
   this->socket_ = that.socket_;
   return *this;
}

//------------------------------------------------------------------------------

bool SysIpL3Addr::operator==(const SysIpL3Addr& that) const
{
   return ((port_ == that.port_) && (GetIpV4Addr() == that.GetIpV4Addr()));
}

//------------------------------------------------------------------------------

bool SysIpL3Addr::operator!=(const SysIpL3Addr& that) const
{
   return !(*this == that);
}

//------------------------------------------------------------------------------

fn_name SysIpL3Addr_SetSocket = "SysIpL3Addr.SetSocket";

void SysIpL3Addr::SetSocket(SysSocket* socket)
{
   Debug::ft(SysIpL3Addr_SetSocket);

   socket_ = socket;
   if(socket_ != nullptr) proto_ = socket->Protocol();
}

//------------------------------------------------------------------------------

string SysIpL3Addr::to_str() const
{
   std::ostringstream stream;

   stream << SysIpL2Addr::to_str() << ": " << port_;
   return stream.str();
}

//------------------------------------------------------------------------------

string SysIpL3Addr::to_string() const
{
   std::ostringstream stream;

   stream << to_str();

   if((proto_ != IpAny) || (socket_ != nullptr))
   {
      stream << " [" << proto_ << ", " << socket_ << ']';
   }

   return stream.str();
}
}