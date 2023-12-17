// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_FRAMEWORK_TASK_H_
#define BASE_FRAMEWORK_TASK_H_

#include "base/base_export.h"
#include "base/callback/callback.h"

namespace base {
const size_t kDeadTask = 0xDEAD7A53;
// RunnableMethodTraits --------------------------------------------------------
//
// This traits-class is used by RunnableMethod to manage the lifetime of the
// callee object.  By default, it is assumed that the callee supports AddRef
// and Release methods.  A particular class can specialize this template to
// define other lifetime management.  For example, if the callee is known to
// live longer than the RunnableMethod object, then a RunnableMethodTraits
// struct could be defined with empty RetainCallee and ReleaseCallee methods.

template <class T>
struct RunnableMethodTraits {
  RunnableMethodTraits() {}

  ~RunnableMethodTraits() {}

  void RetainCallee(T* obj) { obj->AddRef(); }

  void ReleaseCallee(T* obj) { obj->Release(); }
};

// Convenience macro for declaring a RunnableMethodTraits that disables
// refcounting of a class.  This is useful if you know that the callee
// will outlive the RunnableMethod object and thus do not need the ref counts.
//
// The invocation of DISABLE_RUNNABLE_METHOD_REFCOUNT should be done at the
// global namespace scope.  Example:
//
//   namespace foo {
//   class Bar {
//     ...
//   };
//   }  // namespace foo
//
//   DISABLE_RUNNABLE_METHOD_REFCOUNT(foo::Bar);
//
// This is different from DISALLOW_COPY_AND_ASSIGN which is declared inside the
// class.
#define DISABLE_RUNNABLE_METHOD_REFCOUNT(TypeName) \
  template <>                                      \
  struct RunnableMethodTraits<TypeName> {          \
    void RetainCallee(TypeName* manager) {}        \
    void ReleaseCallee(TypeName* manager) {}       \
  }

// RunnableMethod and RunnableFunction -----------------------------------------
//
// Runnable methods are a type of task that call a function on an object when
// they are run. We implement both an object and a set of NewRunnableMethod and
// NewRunnableFunction functions for convenience. These functions are
// overloaded and will infer the template types, simplifying calling code.
//
// The template definitions all use the following names:
// T                - the class type of the object you're supplying
//                    this is not needed for the Static version of the call
// Method/Function  - the signature of a pointer to the method or function you
//                    want to call
// Param            - the parameter(s) to the method, possibly packed as a tuple
// A                - the first parameter (if any) to the method
// B                - the second parameter (if any) to the method
//
// Put these all together and you get an object that can call a method whose
// signature is:
//   R T::MyFunction([A[, B]])
//
// Usage:
// PostTask(NewRunnableMethod(object, &Object::method[, a[, b]])
// PostTask(NewRunnableFunction(&function[, a[, b]])

// RunnableMethod and NewRunnableMethod implementation -------------------------

}  // namespace base

#endif  // BASE_FRAMEWORK_TASK_H_
