// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_SCOPED_NSOBJECT_H_
#define FLUTTER_FML_PLATFORM_DARWIN_SCOPED_NSOBJECT_H_

// Include NSObject.h directly because Foundation.h pulls in many dependencies.
// (Approx 100k lines of code versus 1.5k for NSObject.h). scoped_nsobject gets
// singled out because it is most typically included from other header files.
#import <Foundation/NSObject.h>

// #include "flutter/fml/compiler_specific.h"
// #include "flutter/fml/macros.h"

@class NSAutoreleasePool;

namespace fml {

// scoped_nsobject<> is patterned after scoped_ptr<>, but maintains ownership
// of an NSObject subclass object.  Style deviations here are solely for
// compatibility with scoped_ptr<>'s interface, with which everyone is already
// familiar.
//
// scoped_nsobject<> takes ownership of an object (in the constructor or in
// reset()) by taking over the caller's existing ownership claim.  The caller
// must own the object it gives to scoped_nsobject<>, and relinquishes an
// ownership claim to that object.  scoped_nsobject<> does not call -retain,
// callers have to call this manually if appropriate.
//
// scoped_nsprotocol<> has the same behavior as scoped_nsobject, but can be used
// with protocols.
//
// scoped_nsobject<> is not to be used for NSAutoreleasePools. For
// NSAutoreleasePools use ScopedNSAutoreleasePool from
// scoped_nsautorelease_pool.h instead.
// We check for bad uses of scoped_nsobject and NSAutoreleasePool at compile
// time with a template specialization (see below).

class scoped_nsprotocol_arc_memory_management {
 public:
  static id Retain(__unsafe_unretained id object) __attribute((ns_returns_not_retained));
  static id Autorelease(__unsafe_unretained id object) __attribute((ns_returns_not_retained));
  static void Release(__unsafe_unretained id object);
  static id InvalidValue() __attribute((ns_returns_not_retained)) { return nil; }
};

template <typename NST>
using scoped_nsprotocol_memory_management = scoped_nsprotocol_arc_memory_management;

template <typename NST>
class scoped_nsprotocol {
  using Memory = scoped_nsprotocol_memory_management<NST>;

 public:
  explicit scoped_nsprotocol(__unsafe_unretained NST object = Memory::InvalidValue()) : object_(object) {}

  scoped_nsprotocol(const scoped_nsprotocol<NST>& that) : object_(Memory::Retain(that.object_)) {}

  template <typename NSU>
  scoped_nsprotocol(const scoped_nsprotocol<NSU>& that) : object_(Memory::Retain(that.get())) {}

  ~scoped_nsprotocol() { Memory::Release(object_); }

  scoped_nsprotocol& operator=(const scoped_nsprotocol<NST>& that) {
    reset(Memory::Retain(that.get()));
    return *this;
  }

  void reset(__unsafe_unretained NST object = Memory::InvalidValue()) {
    // We intentionally do not check that object != object_ as the caller must
    // either already have an ownership claim over whatever it passes to this
    // method, or call it with the |RETAIN| policy which will have ensured that
    // the object is retained once more when reaching this point.
    Memory::Release(object_);
    object_ = object;
  }

  bool operator==(NST that) const { return object_ == that; }
  bool operator!=(NST that) const { return object_ != that; }

  operator NST() const { return object_; }

  NST get() const { return object_; }

  void swap(scoped_nsprotocol& that) {
    NST temp = that.object_;
    that.object_ = object_;
    object_ = temp;
  }

  // scoped_nsprotocol<>::release() is like scoped_ptr<>::release.  It is NOT a
  // wrapper for [object_ release].  To force a scoped_nsprotocol<> to call
  // [object_ release], use scoped_nsprotocol<>::reset().
  [[nodiscard]] NST release() __attribute((ns_returns_not_retained)) {
    NST temp = object_;
    object_ = Memory::InvalidValue();
    return temp;
  }

  // Shift reference to the autorelease pool to be released later.
  NST autorelease() __attribute((ns_returns_not_retained)) {
    return Memory::Autorelease(release());
  }

 private:
  NST object_;
};

// Free functions
template <class C>
void swap(scoped_nsprotocol<C>& p1, scoped_nsprotocol<C>& p2) {
  p1.swap(p2);
}

template <class C>
bool operator==(C p1, const scoped_nsprotocol<C>& p2) {
  return p1 == p2.get();
}

template <class C>
bool operator!=(C p1, const scoped_nsprotocol<C>& p2) {
  return p1 != p2.get();
}

template <typename NST>
class scoped_nsobject : public scoped_nsprotocol<NST*> {
  using Memory = scoped_nsprotocol_memory_management<NST*>;

 public:
  explicit scoped_nsobject(__unsafe_unretained NST* object = Memory::InvalidValue())
      : scoped_nsprotocol<NST*>(object) {}

  scoped_nsobject(const scoped_nsobject<NST>& that) : scoped_nsprotocol<NST*>(that) {}

  template <typename NSU>
  scoped_nsobject(const scoped_nsobject<NSU>& that) : scoped_nsprotocol<NST*>(that) {}

  scoped_nsobject& operator=(const scoped_nsobject<NST>& that) {
    scoped_nsprotocol<NST*>::operator=(that);
    return *this;
  }
};

// Specialization to make scoped_nsobject<id> work.
template <>
class scoped_nsobject<id> : public scoped_nsprotocol<id> {
  using Memory = scoped_nsprotocol_memory_management<id>;

 public:
  explicit scoped_nsobject(__unsafe_unretained id object = Memory::InvalidValue()) : scoped_nsprotocol<id>(object) {}

  scoped_nsobject(const scoped_nsobject<id>& that) : scoped_nsprotocol<id>(that) {}

  template <typename NSU>
  scoped_nsobject(const scoped_nsobject<NSU>& that) : scoped_nsprotocol<id>(that) {}

  scoped_nsobject& operator=(const scoped_nsobject<id>& that) {
    scoped_nsprotocol<id>::operator=(that);
    return *this;
  }
};

#if !__has_feature(objc_arc)

// Do not use scoped_nsobject for NSAutoreleasePools, use
// ScopedNSAutoreleasePool instead. This is a compile time check. See details
// at top of header.
template <>
class scoped_nsobject<NSAutoreleasePool> {
 private:
  explicit scoped_nsobject(NSAutoreleasePool* object = nil);
  // FML_DISALLOW_COPY_AND_ASSIGN(scoped_nsobject);
};

#endif  // !__has_feature(objc_arc)

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_SCOPED_NSOBJECT_H_
