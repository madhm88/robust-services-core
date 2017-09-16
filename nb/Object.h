//==============================================================================
//
//  Object.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include "Base.h"
#include <cstddef>
#include <cstdint>
#include "SysTypes.h"

//------------------------------------------------------------------------------

namespace NodeBase
{
//  This extends Base to provide support for
//  o patching (Patch and patchArea_)
//  o memory types (MemType and operators new and delete)
//  o association with a Class (most everything else)
//
class Object : public Base
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~Object() { }

   //  Selector for the Patch function.
   //
   typedef uint8_t sel_t;

   //  Allows a new function to be "added" without changing the interface.
   //  For lab use and delivery of bug fixes only.  A non-trivial subclass
   //  must override this by invoking the same function on its base class.
   //  SELECTOR is used in a switch statement to select the "function" being
   //  invoked.  ARGUMENTS is cast as whatever struct supplies arguments
   //  and/or returned values for the specific SELECTOR being used.
   //
   virtual void Patch(sel_t selector, void* arguments) { }

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   ////////////////////////////////////////////////////////////////////////////
   //
   //  The rest of the public interface is only used by objects that are
   //  supported by a subclass of Class (see Class.h).

   typedef id_t ClassId;         // identifies a Class
   typedef uint32_t InstanceId;  // identifies an object within a Class
   typedef uint32_t ObjectId;    // ClassId (12 bits) + InstanceId (20 bits)

   static const ClassId MaxClassId = (1 << 12) - 1;

   static const size_t MaxInstanceIdLog2 = 20;
   static const InstanceId MaxInstanceId = (1 << MaxInstanceIdLog2) - 1;

   //  Returns the Class singleton associated with this object.  Must be
   //  overridden by subclasses that have an associated Class.  The default
   //  version returns nullptr.
   //
   virtual Class* GetClass() const;

   //  Called to initialize member data when using the Object Template
   //  technique.  The default version does nothing and is overridden by
   //  classes that use Object Template but that also have data members
   //  whose initialization depends on information that is only available
   //  at run time.
   //
   virtual void PostInitialize() { }

   //  Returns the class to which the object belongs.  Returns NIL_ID
   //  if the object is not associated with a Class.
   //
   ClassId GetClassId() const;

   //  Returns the object's instance identifier within its Class.  Must be
   //  overridden by subclasses that support instance identifiers.  The
   //  default version returns NIL_ID.
   //
   virtual InstanceId GetInstanceId() const;

   //  Returns the object's identifier, which is a combination of its class
   //  and instance identifiers.  Returns NIL_ID if the object is not
   //  associated with a Class or does not support instance identifiers.
   //
   ObjectId GetObjectId() const;

   //  Given object identifier OID, returns object's instance identifier in
   //  ID and its associated Class in CLS.  Returns false if the object has
   //  no instance identifier or Class.
   //
   static bool GetClassInstanceId(ObjectId oid, Class*& cls, InstanceId& iid);

   //  Overridden to return the type of memory used by the object.
   //
   virtual MemoryType MemType() const override;

   //  Overridden to support memory types.
   //
   static void* operator new(size_t size, MemoryType type);
   static void* operator new[](size_t size, MemoryType type);
   static void operator delete(void* addr);
   static void operator delete[](void* addr);
   static void operator delete(void* addr, MemoryType type);
   static void operator delete[](void* addr, MemoryType type);
protected:
   //  Protected because this class is virtual.
   //
   Object();

   //  Morphs the object to the class associated with TARGET by changing
   //  its vptr.
   //
   void MorphTo(Class& target);
private:
   //  If a Patch function requires additional memory, this provides enough
   //  space for a pointer to dynamically allocated memory.
   //
   uintptr_t patchArea_;
};
}
#endif