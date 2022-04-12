/*
 * Copyright (C) 2019 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#ifndef KRAKENBRIDGE_BINDINGS_QJS_CONVERTER_IMPL_H_
#define KRAKENBRIDGE_BINDINGS_QJS_CONVERTER_IMPL_H_

#include <type_traits>
#include "atomic_string.h"
#include "converter.h"
#include "core/dom/document.h"
#include "core/dom/events/event.h"
#include "core/dom/events/event_target.h"
#include "core/dom/ng/node_list.h"
#include "core/fileapi/blob_part.h"
#include "core/fileapi/blob_property_bag.h"
#include "core/html/html_element.h"
#include "idl_type.h"
#include "js_event_listener.h"
#include "native_string_utils.h"
#include "qjs_add_event_listener_options.h"
#include "qjs_element_attributes.h"
#include "qjs_error_event_init.h"
#include "qjs_event_init.h"
#include "qjs_event_listener_options.h"
#include "qjs_node.h"
#include "qjs_scroll_to_options.h"

namespace kraken {

template <typename T>
struct is_shared_ptr : std::false_type {};
template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

// Optional value for pointer value.
template <typename T>
struct Converter<IDLOptional<T>, std::enable_if_t<std::is_pointer<typename Converter<T>::ImplType>::value>>
    : public ConverterBase<IDLOptional<T>> {
  using ImplType = typename Converter<T>::ImplType;

  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception) {
    if (JS_IsUndefined(value)) {
      return nullptr;
    }
    return Converter<T>::FromValue(ctx, value, exception);
  }

  static JSValue ToValue(JSContext* ctx, typename Converter<T>::ImplType value) {
    if (value == nullptr) {
      return JS_UNDEFINED;
    }

    return Converter<T>::ToValue(ctx, value);
  }
};

// Nullable value for pointer value
template <typename T>
struct Converter<IDLNullable<T>, std::enable_if_t<std::is_pointer<typename Converter<T>::ImplType>::value>>
    : public ConverterBase<IDLNullable<T>> {
  using ImplType = typename Converter<T>::ImplType;

  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsNull(value)) {
      return nullptr;
    }
    return Converter<T>::FromValue(ctx, value, exception_state);
  }

  static JSValue ToValue(JSContext* ctx, typename Converter<T>::ImplType value) {
    if (value == nullptr) {
      return JS_NULL;
    }

    return Converter<T>::ToValue(ctx, value);
  }
};

template <typename T>
struct Converter<IDLOptional<T>, std::enable_if_t<is_shared_ptr<typename Converter<T>::ImplType>::value>>
    : public ConverterBase<IDLOptional<T>> {
  using ImplType = typename Converter<T>::ImplType;

  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception) {
    if (JS_IsUndefined(value)) {
      return nullptr;
    }
    return Converter<T>::FromValue(ctx, value, exception);
  }

  static JSValue ToValue(JSContext* ctx, typename Converter<T>::ImplType value) {
    if (value == nullptr) {
      return JS_UNDEFINED;
    }

    return Converter<T>::ToValue(ctx, value);
  }
};

// Optional value for arithmetic value
template <typename T>
struct Converter<IDLOptional<T>, std::enable_if_t<std::is_arithmetic<typename Converter<T>::ImplType>::value>>
    : public ConverterBase<IDLOptional<T>> {
  using ImplType = typename Converter<T>::ImplType;

  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception) {
    if (JS_IsUndefined(value)) {
      return 0;
    }
    return Converter<T>::FromValue(ctx, value, exception);
  }

  static JSValue ToValue(JSContext* ctx, typename Converter<T>::ImplType value) {
    return Converter<T>::ToValue(ctx, value);
  }
};

// Any
template <>
struct Converter<IDLAny> : public ConverterBase<IDLAny> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return ScriptValue(ctx, value);
  }

  static JSValue ToValue(JSContext* ctx, ScriptValue value) { return value.QJSValue(); }
};

template <>
struct Converter<IDLOptional<IDLAny>> : public ConverterBase<IDLOptional<IDLAny>> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return ScriptValue(ctx, value);
  }

  static JSValue ToValue(JSContext* ctx, typename Converter<IDLAny>::ImplType value) {
    return Converter<IDLAny>::ToValue(ctx, std::move(value));
  }
};

template <>
struct Converter<IDLNullable<IDLAny>> : public ConverterBase<IDLNullable<IDLAny>> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsNull(value)) {
      return ScriptValue::Empty(ctx);
    }

    assert(!JS_IsException(value));
    return ScriptValue(ctx, value);
  }
};

// Boolean
template <>
struct Converter<IDLBoolean> : public ConverterBase<IDLBoolean> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return JS_ToBool(ctx, value);
  };

  static JSValue ToValue(JSContext* ctx, bool value) { return JS_NewBool(ctx, value); };
};

// Uint32
template <>
struct Converter<IDLUint32> : public ConverterBase<IDLUint32> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    uint32_t v;
    JS_ToUint32(ctx, &v, value);
    return v;
  }

  static JSValue ToValue(JSContext* ctx, uint32_t v) { return JS_NewUint32(ctx, v); }
};

// Int32
template <>
struct Converter<IDLInt32> : public ConverterBase<IDLInt32> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    int32_t v;
    JS_ToInt32(ctx, &v, value);
    return v;
  }
  static JSValue ToValue(JSContext* ctx, uint32_t v) { return JS_NewInt32(ctx, v); }
};

// Int64
template <>
struct Converter<IDLInt64> : public ConverterBase<IDLInt64> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    int64_t v;
    JS_ToInt64(ctx, &v, value);
    return v;
  }
  static JSValue ToValue(JSContext* ctx, uint32_t v) { return JS_NewInt64(ctx, v); }
};

template <>
struct Converter<IDLDouble> : public ConverterBase<IDLDouble> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    double v;
    JS_ToFloat64(ctx, &v, value);
    return v;
  }

  static JSValue ToValue(JSContext* ctx, double v) { return JS_NewFloat64(ctx, v); }
};

template <>
struct Converter<IDLDOMString> : public ConverterBase<IDLDOMString> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return AtomicString(ctx, value);
  }

  static JSValue ToValue(JSContext* ctx, const AtomicString& value) { return value.ToQuickJS(ctx); }
  static JSValue ToValue(JSContext* ctx, NativeString* str) {
    return JS_NewUnicodeString(ctx, str->string(), str->length());
  }
  static JSValue ToValue(JSContext* ctx, std::unique_ptr<NativeString> str) {
    return JS_NewUnicodeString(ctx, str->string(), str->length());
  }
  static JSValue ToValue(JSContext* ctx, uint16_t* bytes, size_t length) {
    return JS_NewUnicodeString(ctx, bytes, length);
  }
  static JSValue ToValue(JSContext* ctx, const std::string& str) { return JS_NewString(ctx, str.c_str()); }
};

template <>
struct Converter<IDLOptional<IDLDOMString>> : public ConverterBase<IDLDOMString> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsUndefined(value))
      return AtomicString::Empty(ctx);
    return Converter<IDLDOMString>::FromValue(ctx, value, exception_state);
  }

  static JSValue ToValue(JSContext* ctx, uint16_t* bytes, size_t length) {
    return Converter<IDLDOMString>::ToValue(ctx, bytes, length);
  }
  static JSValue ToValue(JSContext* ctx, const std::string& str) { return Converter<IDLDOMString>::ToValue(ctx, str); }
  static JSValue ToValue(JSContext* ctx, typename Converter<IDLDOMString>::ImplType value) {
    return Converter<IDLDOMString>::ToValue(ctx, std::move(value));
  }
};

template <>
struct Converter<IDLNullable<IDLDOMString>> : public ConverterBase<IDLDOMString> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsNull(value))
      return AtomicString::Empty(ctx);
    return Converter<IDLDOMString>::FromValue(ctx, value, exception_state);
  }

  static JSValue ToValue(JSContext* ctx, const std::string& value) { return AtomicString(ctx, value).ToQuickJS(ctx); }
  static JSValue ToValue(JSContext* ctx, const AtomicString& value) { return value.ToQuickJS(ctx); }
};

template <typename T>
struct Converter<IDLSequence<T>> : public ConverterBase<IDLSequence<T>> {
  using ImplType = typename IDLSequence<typename Converter<T>::ImplType>::ImplType;

  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    assert(JS_IsArray(ctx, value));

    ImplType v;
    uint32_t length = Converter<IDLUint32>::FromValue(ctx, JS_GetPropertyStr(ctx, value, "length"), exception_state);

    v.reserve(length);

    for (uint32_t i = 0; i < length; i++) {
      auto&& item = Converter<T>::FromValue(ctx, JS_GetPropertyUint32(ctx, value, i), exception_state);
      if (exception_state.HasException()) {
        return {};
      }

      v.emplace_back(item);
    }

    return v;
  }
};

template <typename T>
struct Converter<IDLOptional<IDLSequence<T>>> : public ConverterBase<IDLSequence<T>> {
  using ImplType = typename IDLSequence<typename Converter<T>::ImplType>::ImplType;
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsUndefined(value)) {
      return {};
    }

    return Converter<IDLSequence<T>>::FromValue(ctx, value, exception_state);
  }
};

template <typename T>
struct Converter<IDLNullable<IDLSequence<T>>> : public ConverterBase<IDLSequence<T>> {
  using ImplType = typename IDLSequence<typename Converter<T>::ImplType>::ImplType;
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsNull(value)) {
      return {};
    }

    return Converter<IDLSequence<T>>::FromValue(ctx, value, exception_state);
  }
};

template <>
struct Converter<IDLCallback> : public ConverterBase<IDLCallback> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    if (!JS_IsFunction(ctx, value)) {
      return nullptr;
    }

    return QJSFunction::Create(ctx, value);
  }
};

template <>
struct Converter<BlobPart> : public ConverterBase<BlobPart> {
  using ImplType = BlobPart::ImplType;
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return BlobPart::Create(ctx, value, exception_state);
  }

  static JSValue ToValue(JSContext* ctx, BlobPart* data) { return data->ToQuickJS(ctx); }
};

template <>
struct Converter<BlobPropertyBag> : public ConverterBase<BlobPropertyBag> {
  using ImplType = BlobPropertyBag::ImplType;
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return BlobPropertyBag::Create(ctx, value, exception_state);
  }
};

// EventListener
// template <>
// struct Converter<EventListener> : public ConverterBase<EventListener> {
//  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
//    assert(!JS_IsException(value));
//    if (!JS_IsFunction(ctx, value)) {
//      return nullptr;
//    }
//
//    return EventListener::Create(ctx, value);
//  }
//};
//
// template <>
// struct Converter<IDLNullable<EventListener>> : public ConverterBase<EventListener> {
//  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
//    assert(!JS_IsException(value));
//    if (JS_IsNull(value)) {
//      return nullptr;
//    }
//
//    return Converter<EventListener>::FromValue(ctx, value, exception_state);
//  }
//};

template <>
struct Converter<EventTarget> : public ConverterBase<EventTarget> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return toScriptWrappable<EventTarget>(value);
  }

  static JSValue ToValue(JSContext* ctx, ImplType value) { return value->ToQuickJS(); }
};

template <>
struct Converter<IDLNullable<EventTarget>> : public ConverterBase<EventTarget> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsNull(value)) {
      return nullptr;
    }

    assert(!JS_IsException(value));
    return Converter<EventTarget>::FromValue(ctx, value, exception_state);
  }

  static JSValue ToValue(JSContext* ctx, ImplType value) {
    if (value == nullptr)
      return JS_NULL;
    return Converter<EventTarget>::ToValue(ctx, value);
  }
};

template <>
struct Converter<Event> : public ConverterBase<Event> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return toScriptWrappable<Event>(value);
  }

  static JSValue ToValue(JSContext* ctx, ImplType value) {
    return reinterpret_cast<ScriptWrappable*>(value)->ToQuickJS();
  }
};

template <>
struct Converter<JSEventListener> : public ConverterBase<JSEventListener> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return JSEventListener::CreateOrNull(QJSFunction::Create(ctx, value));
  }
};

template <>
struct Converter<IDLNullable<JSEventListener>> : public ConverterBase<JSEventListener> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    if (JS_IsNull(value)) {
      return nullptr;
    }

    assert(!JS_IsException(value));
    return Converter<JSEventListener>::FromValue(ctx, value, exception_state);
  }
};

template <>
struct Converter<EventInit> : public ConverterBase<EventInit> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return EventInit::Create(ctx, value, exception_state);
  }
};

template <>
struct Converter<ErrorEventInit> : public ConverterBase<ErrorEventInit> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return ErrorEventInit::Create(ctx, value, exception_state);
  }
};

template <>
struct Converter<AddEventListenerOptions> : public ConverterBase<AddEventListenerOptions> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return AddEventListenerOptions::Create(ctx, value, exception_state);
  };
};

template <>
struct Converter<EventListenerOptions> : public ConverterBase<EventListenerOptions> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return EventListenerOptions::Create(ctx, value, exception_state);
  }
};

#define DEFINE_SCRIPT_WRAPPABLE_CONVERTER(class_name)                                           \
  template <>                                                                                   \
  struct Converter<class_name> : public ConverterBase<class_name> {                             \
    static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) { \
      assert(!JS_IsException(value));                                                           \
      return toScriptWrappable<class_name>(value);                                              \
    }                                                                                           \
    static JSValue ToValue(JSContext* ctx, ImplType value) { return value->ToQuickJS(); }       \
  };

DEFINE_SCRIPT_WRAPPABLE_CONVERTER(Node);
DEFINE_SCRIPT_WRAPPABLE_CONVERTER(Document);
DEFINE_SCRIPT_WRAPPABLE_CONVERTER(HTMLElement);

template <>
struct Converter<NodeList> : public ConverterBase<NodeList> {
  static JSValue ToValue(JSContext* ctx, ImplType value) { return value->ToQuickJS(); }
};

template <>
struct Converter<ScrollToOptions> : ConverterBase<ScrollToOptions> {
  static ImplType FromValue(JSContext* ctx, JSValue value, ExceptionState& exception_state) {
    assert(!JS_IsException(value));
    return ScrollToOptions::Create(ctx, value, exception_state);
  }
};

template <>
struct Converter<ElementAttributes> : ConverterBase<ElementAttributes> {
  static JSValue ToValue(JSContext* ctx, ImplType value) { return value->ToQuickJS(); }
};

};  // namespace kraken

#endif  // KRAKENBRIDGE_BINDINGS_QJS_CONVERTER_IMPL_H_
