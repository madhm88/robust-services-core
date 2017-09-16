//==============================================================================
//
//  MsgPort.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef MSGPORT_H_INCLUDED
#define MSGPORT_H_INCLUDED

#include "ProtocolLayer.h"
#include <cstddef>
#include "GlobalAddress.h"
#include "LocalAddress.h"
#include "NbTypes.h"
#include "SbTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace SessionBase
{
//  A message port resides at the bottom of a protocol stack, with one or more
//  PSMs (ProtocolSMs) above it.  It manages the local (host) and remote (peer)
//  addresses that are exchanging messages.  When the stack receives an initial
//  message, the port saves the host and peer addresses and adds them to each
//  outgoing message.  But when the the stack sends the initial message, that
//  message must specify the peer's address and the host's IP address, IP port,
//  and factory.
//
class MsgPort : public ProtocolLayer
{
   friend class MsgPortPool;
   friend class PsmContext;
   friend class PsmFactory;
   friend class SsmContext;
public:
   //  Creates a port that will receive MSG and run in CTX.
   //
   MsgPort(const Message& msg, Context& ctx);

   //  Creates a port from UPPER.  Its local and remote addresses will not be
   //  known until a message is sent.  Note, however, that LocAddr().SbAddr()
   //  will be complete: the factory is that of UPPER, and the object address
   //  is also known.
   //
   explicit MsgPort(ProtocolLayer& upper);

   //  Releases the port's TCP socket, if any.
   //
   ~MsgPort();

   //  Returns true if the port has received a message.
   //
   bool HasRcvdMsg() const { return msgRcvd_; }

   //  Returns true if the port has sent a message.
   //
   bool HasSentMsg() const { return msgSent_; }

   //  Returns the port's address.
   //
   const GlobalAddress& LocAddr() const { return locAddr_; }

   //  Returns the peer's address.
   //
   const GlobalAddress& RemAddr() const { return remAddr_; }

   //  Returns the port's local address.  This would typically be saved in a
   //  user profile so that an input handler can forward subsequent messages
   //  from the user to the port by placing the address in MsgHeader.rxAddr.
   //
   const LocalAddress& ObjAddr() const { return locAddr_.sbAddr_; }

   //  Returns the port (if any) identified by locAddr.
   //
   static MsgPort* Find(const LocalAddress& locAddr);

   //  Overridden to return this port.
   //
   virtual MsgPort* Port() const override;

   //  Overridden to return the PSM at the top of the stack.
   //
   virtual ProtocolSM* UppermostPsm() const override;

   //  Returns the ports's factory.
   //
   virtual FactoryId GetFactory()
      const override { return locAddr_.sbAddr_.fid; }

   //  Overridden to modify the addresses in this port and PEER.  Returns
   //  the peer port on success.
   //
   virtual ProtocolLayer* JoinPeer
      (const LocalAddress& peer, GlobalAddress& peerPrevRemAddr) override;

   //  Overridden to modify the addresses in this port and PEER.
   //
   virtual bool DropPeer(const GlobalAddress& peerPrevRemAddr) override;

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;

   //  Overridden to obtain a port from its object pool.
   //
   static void* operator new(size_t size);
protected:
   //  Returns the route over which an outgoing message should be sent.
   //
   virtual Message::Route Route() const override;

   //  Overridden to handle deletion of the layer above this one.
   //
   virtual void AdjacentDeleted(bool upper) override;

   //  Overridden to relinquish any socket during error recovery.
   //
   virtual void Cleanup() override;
private:
   //> Highest valid port identifier.
   //
   static const PooledObjectId MaxId = 0x40000;  // 2^18 = 256K

   //  Overridden to create the layer above for an incoming message.
   //
   virtual ProtocolLayer* AllocUpper(const Message& msg) override;

   //  Overridden to receive MSG when a transaction begins.
   //
   virtual Event* ReceiveMsg(Message& msg) override;

   //  Overridden to send MSG.  If a message has neither been sent nor
   //  received, MSG must contain the source and destination addresses.
   //
   virtual bool SendMsg(Message& msg) override;

   //  Overridden to return MSG as is, which will then be passed to the
   //  port and sent.
   //
   virtual Message* WrapMsg(Message& msg) override;

   //  Performs initialization that is common to all constructors.
   //  MSG is the incoming message, if any.
   //
   void Initialize(const Message* msg);

   //  If A sends a message to B and then wishes to send another message
   //  before it has received a reply from B, it has yet to be informed
   //  of B's address.  A's subsequent message will therefore be lost
   //  unless B has implemented a scheme to map A to B.  Such a scheme
   //  is needed for interprocessor messages but is currently implemented
   //  inefficiently (see ProtocolSMPool::FindPeerPort).  To avoid this
   //  inefficiency in the intraprocessor case, this function immediately
   //  updates A's port with B's address when a message arrives at B.
   //
   void UpdatePeer() const;

   //  Finds the port whose peer is remAddr.  This is invoked as described
   //  under UpdatePeer, when a peer sends a subsequent message to a host
   //  port from which it has not yet received a reply.  The peer is still
   //  unaware of the host port's address, so the host port must be found
   //  by iterating over active ports to find the one that contains the
   //  peer's address (remAddr_).
   //
   static MsgPort* FindPeer(const GlobalAddress& remAddr);

   //  The address of this port.
   //
   GlobalAddress locAddr_;

   //  The address of the peer port.
   //
   GlobalAddress remAddr_;

   //  Set if the port has received a message.
   //
   bool msgRcvd_;

   //  Set if the port has sent a message.
   //
   bool msgSent_;
};
}
#endif