/*
 * Copyright (C) 2021-present The Kraken authors. All rights reserved.
 */

#ifndef KRAKENBRIDGE_BINDINGS_QJS_ATOMIC_STRING_H_
#define KRAKENBRIDGE_BINDINGS_QJS_ATOMIC_STRING_H_

#include <quickjs/quickjs.h>
#include <memory>
#include "foundation/macros.h"
#include "foundation/native_string.h"
#include "native_string_utils.h"
#include "qjs_engine_patch.h"

namespace kraken {

// An AtomicString instance represents a string, and multiple AtomicString
// instances can share their string storage if the strings are
// identical. Comparing two AtomicString instances is much faster than comparing
// two String instances because we just check string storage identity.
class AtomicString {
 public:
  enum class StringKind { kIsLowerCase, kIsUpperCase, kIsMixed };

  static AtomicString Empty(JSContext* ctx);
  static AtomicString From(JSContext* ctx, NativeString* native_string);

  AtomicString() = default;
  AtomicString(JSContext* ctx, const std::string& string);
  AtomicString(JSContext* ctx, JSValue value);
  ~AtomicString() { JS_FreeAtomRT(runtime_, atom_); };

  // Return the undefined string value from atom key.
  JSValue ToQuickJS(JSContext* ctx) const { return JS_AtomToValue(ctx, atom_); };

  bool IsNull() const;
  bool IsEmpty() const;

  JSAtom Impl() const { return atom_; }

  [[nodiscard]] std::string ToStdString() const;
  [[nodiscard]] std::unique_ptr<NativeString> ToNativeString() const;

  AtomicString ToUpperIfNecessary() const;
  const AtomicString ToUpperSlow() const;

  const AtomicString ToLowerIfNecessary() const;
  const AtomicString ToLowerSlow() const;

  // Copy assignment
  AtomicString(AtomicString const& value);
  AtomicString& operator=(const AtomicString& other);

  // Move assignment
  AtomicString(AtomicString&& value) noexcept;
  AtomicString& operator=(AtomicString&& value) noexcept;

  bool operator==(const AtomicString& other) const { return other.atom_ == this->atom_; }
  bool operator!=(const AtomicString& other) const { return other.atom_ != this->atom_; };

 protected:
  JSContext* ctx_{nullptr};
  JSRuntime* runtime_{nullptr};
  JSAtom atom_{JS_ATOM_NULL};
  mutable JSAtom atom_upper_{JS_ATOM_NULL};
  mutable JSAtom atom_lower_{JS_ATOM_NULL};
  StringKind kind_;
};

}  // namespace kraken

#endif  // KRAKENBRIDGE_BINDINGS_QJS_ATOMIC_STRING_H_