//==============================================================================
//
//  Pooled.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef POOLED_H_INCLUDED
#define POOLED_H_INCLUDED

#include "Object.h"
#include <cstddef>
#include <cstdint>
#include "Q1Link.h"
#include "SysTypes.h"

namespace NodeBase
{
   class ObjectPool;
}

//------------------------------------------------------------------------------

namespace NodeBase
{
//  A pooled object is allocated from an ObjectPool created during system
//  initialization rather than from the heap.
//
class Pooled : public Object
{
   friend class ObjectPool;
public:
   //  Virtual to allow subclassing.
   //
   virtual ~Pooled() { }

   //  Returns true if the object is marked corrupt.
   //
   bool IsCorrupt() const { return corrupt_; }

   //  Returns true if the object is invalid.
   //
   bool IsInvalid() const { return !assigned_; }

   //  Returns the offset to link_.
   //
   static ptrdiff_t LinkDiff();

   //  Overridden to claim blocks that this object owns.  If the object is
   //  marked corrupt, it simply returns; otherwise, it surrounds a call
   //  to Base::ClaimBlocks by setting and clearing the corrupt_ flag.
   //
   virtual void ClaimBlocks() override;

   //  Clears the object's orphaned_ field so that the object pool audit
   //  will not reclaim it.  May be overridden, but the base class version
   //  must be invoked.
   //
   virtual void Claim() override;

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden to return the type of memory used by subclasses.
   //
   virtual MemoryType MemType() const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;

   //  Overridden to return a block to its object pool.
   //
   static void operator delete(void* addr);
protected:
   //  Protected because this class is virtual.
   //
   Pooled();
private:
   //  Override vector operator new to prohibit array allocation.
   //
   static void* operator new[](size_t size, MemoryType type);

   //  Overridden to prohibit copying.
   //
   Pooled(const Pooled& that);
   void operator=(const Pooled& that);

   //  Link for queueing the object.
   //
   Q1Link link_;

   //  True if allocated for an object; false if on free queue.
   //
   bool assigned_;

   //  Zero for a block that is in use.  Incremented each time through the
   //  audit; if it reaches a threshold, the block is deemed to be orphaned
   //  and is recovered.
   //
   uint8_t orphaned_;

   //  Used by audits to avoid invoking functions on a corrupt block.  The
   //  audit sets this flag before it invokes any function on the object.
   //  If the object's function traps, the flag is still set when the audit
   //  resumes execution, so it knows that the block is corrupt and simply
   //  recovers it instead of invoking its function again.  If the function
   //  returns successfully, the audit immediately clears the flag.
   //
   bool corrupt_;

   //  Used by audits to avoid double logging.
   //
   bool logged_;
};
}
#endif