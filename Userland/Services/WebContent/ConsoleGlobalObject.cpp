/*
 * Copyright (c) 2021-2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ConsoleGlobalObject.h"
#include <LibJS/Runtime/Completion.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/HTML/Window.h>

namespace WebContent {

ConsoleGlobalObject::ConsoleGlobalObject(JS::Realm& realm, Web::HTML::Window& parent_object)
    : GlobalObject(realm)
    , m_window_object(&parent_object)
{
}

void ConsoleGlobalObject::initialize(JS::Realm& realm)
{
    Base::initialize(realm);

    define_native_accessor(realm, "$0", $0_getter, nullptr, 0);
}

void ConsoleGlobalObject::visit_edges(Visitor& visitor)
{
    Base::visit_edges(visitor);
    visitor.visit(m_window_object);
}

JS::ThrowCompletionOr<JS::Object*> ConsoleGlobalObject::internal_get_prototype_of() const
{
    return m_window_object->internal_get_prototype_of();
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_set_prototype_of(JS::Object* prototype)
{
    return m_window_object->internal_set_prototype_of(prototype);
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_is_extensible() const
{
    return m_window_object->internal_is_extensible();
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_prevent_extensions()
{
    return m_window_object->internal_prevent_extensions();
}

JS::ThrowCompletionOr<Optional<JS::PropertyDescriptor>> ConsoleGlobalObject::internal_get_own_property(JS::PropertyKey const& property_name) const
{
    if (auto result = TRY(m_window_object->internal_get_own_property(property_name)); result.has_value())
        return result;

    return Base::internal_get_own_property(property_name);
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_define_own_property(JS::PropertyKey const& property_name, JS::PropertyDescriptor const& descriptor)
{
    return m_window_object->internal_define_own_property(property_name, descriptor);
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_has_property(JS::PropertyKey const& property_name) const
{
    return TRY(Object::internal_has_property(property_name)) || TRY(m_window_object->internal_has_property(property_name));
}

JS::ThrowCompletionOr<JS::Value> ConsoleGlobalObject::internal_get(JS::PropertyKey const& property_name, JS::Value receiver) const
{
    if (TRY(m_window_object->has_own_property(property_name)))
        return m_window_object->internal_get(property_name, (receiver == this) ? m_window_object : receiver);

    return Base::internal_get(property_name, receiver);
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_set(JS::PropertyKey const& property_name, JS::Value value, JS::Value receiver)
{
    return m_window_object->internal_set(property_name, value, (receiver == this) ? m_window_object : receiver);
}

JS::ThrowCompletionOr<bool> ConsoleGlobalObject::internal_delete(JS::PropertyKey const& property_name)
{
    return m_window_object->internal_delete(property_name);
}

JS::ThrowCompletionOr<JS::MarkedVector<JS::Value>> ConsoleGlobalObject::internal_own_property_keys() const
{
    return m_window_object->internal_own_property_keys();
}

static JS::ThrowCompletionOr<ConsoleGlobalObject*> get_console(JS::VM& vm)
{
    if (!is<ConsoleGlobalObject>(vm.get_global_object()))
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAnObjectOfType, "ConsoleGlobalObject");

    return static_cast<ConsoleGlobalObject*>(&vm.get_global_object());
}

JS_DEFINE_NATIVE_FUNCTION(ConsoleGlobalObject::$0_getter)
{
    auto* console_global_object = TRY(get_console(vm));
    auto& window = *console_global_object->m_window_object;
    auto* inspected_node = window.associated_document().inspected_node();
    if (!inspected_node)
        return JS::js_undefined();

    return inspected_node;
}

}
