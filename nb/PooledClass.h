//==============================================================================
//
//  PooledClass.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef POOLEDCLASS_H_INCLUDED
#define POOLEDCLASS_H_INCLUDED

#include "Class.h"
#include <cstddef>

namespace NodeBase
{
   class ObjectPool;
}

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Subclass of Class that supports PooledObjects.
//
class PooledClass : public Class
{
public:
   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;
protected:
   //  Protected because subclasses should be singletons.
   //
   PooledClass(ClassId cid, size_t size);

   //  Protected because this class is virtual.
   //
   virtual ~PooledClass();

   //  Sets pool_.  A subclass' Initialize function calls this to
   //  set its associated ObjectPool subclass.
   //    auto pool = Singleton< MyObjectPool >::Instance();
   //    auto obj1 = new MyPooledObject(...);
   //    auto obj2 = new MyPooledObject(...);
   //    SetPool(*pool);
   //    SetVptr(*obj1);
   //    SetTemplate(*obj1);
   //    SetQuasiSingleton(*obj2);
   //
   bool SetPool(ObjectPool& pool);

   //  Overridden to call pool_->DeqBlock.
   //
   virtual Object* New(size_t size) override;
private:
   //  The pool that manages this class's objects.
   //
   ObjectPool* pool_;
};
}
#endif