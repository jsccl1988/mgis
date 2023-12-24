// Copyright (c) 2015 The Baidu Authors.
// All rights reserved.

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SMART_PTR_REF_COUNTED_H_
#define BASE_SMART_PTR_REF_COUNTED_H_

#include <cassert>
#include <iosfwd>

#include <algorithm>

#include "base/build/build_config.h"
#include "base/compiler_specific.h"
#include "base/platform.h"

namespace base {

namespace subtle {

class RefCountedBase {
 public:
  bool HasOneRef() const { return ref_count_ == 1; }

 protected:
  RefCountedBase()
      : ref_count_(0)
  #ifdef _DEBUG
      , in_dtor_(false)
  #endif
      {
  }

  ~RefCountedBase() {
  #ifdef _DEBUG
    assert(in_dtor_);
  #endif
  }


  void AddRef() const {
    // TODO(maruel): Add back once it doesn't assert 500 times/sec.
    // Current thread books the critical section "AddRelease"
    // without release it.
    // DFAKE_SCOPED_LOCK_THREAD_LOCKED(add_release_);
  #ifdef _DEBUG
    assert(!in_dtor_);
  #endif
    ++ref_count_;
  }

  // Returns true if the object should self-delete.
  bool Release() const {
    // TODO(maruel): Add back once it doesn't assert 500 times/sec.
    // Current thread books the critical section "AddRelease"
    // without release it.
    // DFAKE_SCOPED_LOCK_THREAD_LOCKED(add_release_);
  #ifdef _DEBUG
    assert(!in_dtor_);
  #endif
    if (--ref_count_ == 0) {
  #ifdef _DEBUG
      in_dtor_ = true;
  #endif
      return true;
    }
    return false;
  }

 private:
  mutable int ref_count_;
#ifdef _DEBUG
  mutable bool in_dtor_;
#endif

  DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
};

class RefCountedThreadSafeBase {
 public:
  bool HasOneRef() const {
    return (const_cast<RefCountedThreadSafeBase*>(this)->ref_count_ == 1);
  }

 protected:
  RefCountedThreadSafeBase() : ref_count_(0) {
#ifdef _DEBUG
    in_dtor_ = false;
#endif
  }
  ~RefCountedThreadSafeBase() {
#ifdef _DEBUG
    assert(in_dtor_);
#endif
  }

  void AddRef() const {
#ifdef _DEBUG
    assert(!in_dtor_);
#endif
    ::InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&ref_count_), 1);
  }

// Returns true if the object should self-delete.
  bool Release() const {
#ifdef _DEBUG
    assert(!in_dtor_);
#endif
    long new_value = InterlockedExchangeAdd(
      reinterpret_cast<volatile LONG*>(&ref_count_), -1) - 1;
    if (0 == new_value) {
#ifdef _DEBUG
      in_dtor_ = true;
#endif
      return true;
    }
    return false;
  }

 private:
  mutable int ref_count_;
#ifdef _DEBUG
  mutable bool in_dtor_;
#endif

  DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafeBase);
};

}  // namespace subtle

// Forward declaration.
template <class T, class Traits> class RefCounted;

// Default traits for RefCounted<T>.  Deletes the object when its ref
// count reaches 0.  Overload to delete it on a different crt etc.
template<class T>
struct DefaultRefCountedTraits {
  virtual void Destruct(const T* x) {
    // Delete through RefCounted to make child classes only need to be
    // friend with RefCounted instead of this struct, which is an
    // implementation detail.
    // (chenchunliang) here we use virtual table to record Destruct function
    // point so we can call the original function on multi-crt space,we add
    // virtual table to DefaultRefCountedTraits instead of to T so we
    // can avoid introduce virtual table to T if T do not need virtual table.
    RefCounted<T, DefaultRefCountedTraits>::DeleteInternal(x);
  }
};


//
// A base class for reference counted classes.  Otherwise, known as a cheap
// knock-off of WebKit's RefCounted<T> class.  To use this guy just extend your
// class from it like so:
//
//   class MyFoo : public base::RefCounted<MyFoo> {
//    ...
//    private:
//     friend class base::RefCounted<MyFoo>;
//     ~MyFoo();
//   };
//
// You should always make your destructor private, to avoid any code deleting
// the object accidently while there are references to it.
template <class T, class Traits = DefaultRefCountedTraits<T> >
class RefCounted : public subtle::RefCountedBase {
 public:
  RefCounted() {}

  void AddRef() const {
    subtle::RefCountedBase::AddRef();
  }

  void Release() const {
    if (subtle::RefCountedBase::Release()) {
      traits_.Destruct(static_cast<const T*>(this));
    }
  }

 protected:
  ~RefCounted() {}

 private:
  friend struct DefaultRefCountedTraits<T>;
  static void DeleteInternal(const T* x) { delete x; }
  mutable Traits traits_;
  DISALLOW_COPY_AND_ASSIGN(RefCounted);
};

// Forward declaration.
template <class T, class Traits> class RefCountedThreadSafe;

// Default traits for RefCountedThreadSafe<T>.  Deletes the object when its ref
// count reaches 0.  Overload to delete it on a different thread etc.
template<class T>
struct DefaultRefCountedThreadSafeTraits {
  virtual void Destruct(const T* x) {
    // Delete through RefCountedThreadSafe to make child classes only need to be
    // friend with RefCountedThreadSafe instead of this struct, which is an
    // implementation detail.
    // (chenchunliang) here we use virtual table to record Destruct function
    // point so we can call the original function on multi-crt space,we add
    // virtual table to DefaultRefCountedThreadSafeTraits instead of to T so we
    // can avoid introduce virtual table to T if T do not need virtual table.
    RefCountedThreadSafe<T,
                         DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
  }
};

//
// A thread-safe variant of RefCounted<T>
//
//   class MyFoo : public base::RefCountedThreadSafe<MyFoo> {
//    ...
//   };
//
// If you're using the default trait, then you should add compile time
// asserts that no one else is deleting your object.  i.e.
//    private:
//     friend class base::RefCountedThreadSafe<MyFoo>;
//     ~MyFoo();
template <class T, class Traits = DefaultRefCountedThreadSafeTraits<T> >
class RefCountedThreadSafe : public subtle::RefCountedThreadSafeBase {
 public:
  RefCountedThreadSafe() {
  }

  void AddRef() const {
    subtle::RefCountedThreadSafeBase::AddRef();
  }

  void Release() const {
    if (subtle::RefCountedThreadSafeBase::Release()) {
      traits_.Destruct(static_cast<const T*>(this));
    }
  }

 protected:
  ~RefCountedThreadSafe() {}

 private:
  friend struct DefaultRefCountedThreadSafeTraits<T>;
  static void DeleteInternal(const T* x) { delete x; }
  mutable Traits traits_;
  DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafe);
};

//
// A thread-safe wrapper for some piece of data so we can place other
// things in scoped_refptrs<>.
//
template<class T>
class RefCountedData
    : public base::RefCountedThreadSafe< base::RefCountedData<T> > {
 public:
  RefCountedData() : data() {}
  RefCountedData(const T& in_value) : data(in_value) {}

  T data;

 private:
  friend class base::RefCountedThreadSafe<base::RefCountedData<T> >;
  ~RefCountedData() {}
};

}  // namespace base

namespace base {
//
// A smart pointer class for reference counted objects.  Use this class instead
// of calling AddRef and Release manually on a reference counted object to
// avoid common memory leaks caused by forgetting to Release an object
// reference.  Sample usage:
//
//   class MyFoo : public RefCounted<MyFoo> {
//    ...
//   };
//
//   void some_function() {
//     ScopedRefPtr<MyFoo> foo = new MyFoo();
//     foo->Method(param);
//     // |foo| is released when this function returns
//   }
//
//   void some_other_function() {
//     ScopedRefPtr<MyFoo> foo = new MyFoo();
//     ...
//     foo = NULL;  // explicitly releases |foo|
//     ...
//     if (foo)
//       foo->Method(param);
//   }
//
// The above examples show how ScopedRefPtr<T> acts like a pointer to T.
// Given two ScopedRefPtr<T> classes, it is also possible to exchange
// references between the two objects, like so:
//
//   {
//     ScopedRefPtr<MyFoo> a = new MyFoo();
//     ScopedRefPtr<MyFoo> b;
//
//     b.swap(a);
//     // now, |b| references the MyFoo object, and |a| references NULL.
//   }
//
// To make both |a| and |b| in the above example reference the same MyFoo
// object, simply use the assignment operator:
//
//   {
//     ScopedRefPtr<MyFoo> a = new MyFoo();
//     ScopedRefPtr<MyFoo> b;
//
//     b = a;
//     // now, |a| and |b| each own a reference to the same MyFoo object.
//   }
//
template <class T>
class ScopedRefPtr {
 public:
  typedef T element_type;

  ScopedRefPtr() : ptr_(NULL) {
  }

  ScopedRefPtr(T* p) : ptr_(p) {
    if (ptr_)
      AddRef(ptr_);
  }

  ScopedRefPtr(const ScopedRefPtr<T>& r) : ptr_(r.ptr_) {
    if (ptr_)
      AddRef(ptr_);
  }

  template <class U>
  ScopedRefPtr(const ScopedRefPtr<U>& r) : ptr_(r.get()) {
    if (ptr_)
      AddRef(ptr_);
  }

  ~ScopedRefPtr() {
    if (ptr_)
      Release(ptr_);
  }

  T* get() const { return ptr_; }

  T& operator*() const {
    assert(ptr_ != NULL);
    return *ptr_;
  }

  T* operator->() const {
    assert(ptr_ != NULL);
    return ptr_;
  }

  ScopedRefPtr<T>& operator=(T* p) {
    // AddRef first so that self assignment should work
    if (p)
      AddRef(p);
    T* old_ptr = ptr_;
    ptr_ = p;
    if (old_ptr)
      Release(old_ptr);
    return *this;
  }

  ScopedRefPtr<T>& operator=(const ScopedRefPtr<T>& r) {
    return *this = r.ptr_;
  }

  template <class U>
  ScopedRefPtr<T>& operator=(const ScopedRefPtr<U>& r) {
    return *this = r.get();
  }

  void swap(T** pp) {
    T* p = ptr_;
    ptr_ = *pp;
    *pp = p;
  }

  void swap(ScopedRefPtr<T>& r) {
    swap(&r.ptr_);
  }

 private:
  // Allow ScopedRefPtr<T> to be used in boolean expressions, but not
  // implicitly convertible to a real bool (which is dangerous).
  //
  // Note that this trick is only safe when the == and != operators
  // are declared explicitly, as otherwise "refptr1 == refptr2"
  // will compile but do the wrong thing (i.e., convert to Testable
  // and then do the comparison).
  typedef T* ScopedRefPtr::*Testable;

 public:
  operator Testable() const { return ptr_ ? &ScopedRefPtr::ptr_ : nullptr; }

  template <class U>
  bool operator==(const ScopedRefPtr<U>& rhs) const {
    return ptr_ == rhs.get();
  }

  template <class U>
  bool operator!=(const ScopedRefPtr<U>& rhs) const {
    return !operator==(rhs);
  }

  template <class U>
  bool operator<(const ScopedRefPtr<U>& rhs) const {
    return ptr_ < rhs.get();
  }

 protected:
  T* ptr_;

 private:
  // Non-inline helpers to allow:
  //     class Opaque;
  //     extern template class ScopedRefPtr<Opaque>;
  // Otherwise the compiler will complain that Opaque is an incomplete type.
  static void AddRef(T* ptr);
  static void Release(T* ptr);
};

template <class T>
void ScopedRefPtr<T>::AddRef(T* ptr) {
  ptr->AddRef();
}

template <class T>
void ScopedRefPtr<T>::Release(T* ptr) {
  ptr->Release();
}

// Handy utility for creating a ScopedRefPtr<T> out of a T* explicitly without
// having to retype all the template arguments
template <class T>
ScopedRefPtr<T> make_scoped_refptr(T* t) {
  return ScopedRefPtr<T>(t);
}

// Temporary operator overloads to facilitate the transition. See
// https://crbug.com/110610.
template <class T, class U>
bool operator==(const ScopedRefPtr<T>& lhs, const U* rhs) {
  return lhs.get() == rhs;
}

template <class T, class U>
bool operator==(const T* lhs, const ScopedRefPtr<U>& rhs) {
  return lhs == rhs.get();
}

template <class T, class U>
bool operator!=(const ScopedRefPtr<T>& lhs, const U* rhs) {
  return !operator==(lhs, rhs);
}

template <class T, class U>
bool operator!=(const T* lhs, const ScopedRefPtr<U>& rhs) {
  return !operator==(lhs, rhs);
}

template <class T>
std::ostream& operator<<(std::ostream& out, const ScopedRefPtr<T>& p) {
  return out << p.get();
}
}  // namespace base

#endif  // BASE_SMART_PTR_REF_COUNTED_H_
