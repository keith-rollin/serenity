/*
 * Copyright (c) 2023, Caoimhe Byrne <caoimhebyrne06@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Message.h"
#include <AK/MemoryStream.h>
#include <AK/Stream.h>
#include <AK/String.h>

namespace SpiceAgent {

ErrorOr<AnnounceCapabilitiesMessage> AnnounceCapabilitiesMessage::read_from_stream(AK::Stream& stream)
{
    // If this message is a capabilities request, we don't have to parse anything else.
    auto is_requesting = TRY(stream.read_value<u32>()) == 1;
    if (is_requesting) {
        return AnnounceCapabilitiesMessage(is_requesting);
    }

    return Error::from_string_literal("Unexpected non-requesting announce capabilities message received!");
}

ErrorOr<void> AnnounceCapabilitiesMessage::write_to_stream(AK::Stream& stream)
{
    TRY(stream.write_value<u32>(is_request()));

    // Each bit in this u32 indicates if a certain capability is enabled or not.
    u32 capabilities_bits = 0;
    for (auto capability : capabilities()) {
        // FIXME: At the moment, we only support up to 32 capabilities as the Spice protocol
        //        only contains 17 capabilities.
        auto capability_value = to_underlying(capability);
        VERIFY(capability_value < 32);

        capabilities_bits |= 1 << capability_value;
    }

    TRY(stream.write_value(capabilities_bits));

    return {};
}

ErrorOr<String> AnnounceCapabilitiesMessage::debug_description()
{
    StringBuilder builder;
    TRY(builder.try_append("AnnounceCapabilities { "sv));
    TRY(builder.try_appendff("is_request = {}, ", is_request()));
    TRY(builder.try_appendff("capabilities.size() = {}", capabilities().size()));
    TRY(builder.try_append(" }"sv));
    return builder.to_string();
}

ErrorOr<ClipboardGrabMessage> ClipboardGrabMessage::read_from_stream(AK::Stream& stream)
{
    auto types = Vector<ClipboardDataType>();
    while (!stream.is_eof()) {
        auto value = TRY(stream.read_value<u32>());
        if (value >= to_underlying(ClipboardDataType::__End)) {
            return Error::from_string_literal("Unsupported clipboard type");
        }

        types.append(static_cast<ClipboardDataType>(value));
    }

    return ClipboardGrabMessage(types);
}

ErrorOr<void> ClipboardGrabMessage::write_to_stream(AK::Stream& stream)
{
    for (auto type : types()) {
        TRY(stream.write_value(type));
    }

    return {};
}

ErrorOr<String> ClipboardGrabMessage::debug_description()
{
    StringBuilder builder;
    TRY(builder.try_append("ClipboardGrabMessage { "sv));
    TRY(builder.try_appendff("types = {}", types()));
    TRY(builder.try_append(" }"sv));
    return builder.to_string();
}

ErrorOr<ClipboardRequestMessage> ClipboardRequestMessage::read_from_stream(AK::Stream& stream)
{
    auto value = TRY(stream.read_value<u32>());
    if (value >= to_underlying(ClipboardDataType::__End)) {
        return Error::from_string_literal("Unsupported clipboard type");
    }

    auto type = static_cast<ClipboardDataType>(value);
    return ClipboardRequestMessage(type);
}

ErrorOr<void> ClipboardRequestMessage::write_to_stream(AK::Stream& stream)
{
    TRY(stream.write_value(data_type()));
    return {};
}

ErrorOr<String> ClipboardRequestMessage::debug_description()
{
    StringBuilder builder;
    TRY(builder.try_append("ClipboardRequest { "sv));
    TRY(builder.try_appendff("data_type = {}", data_type()));
    TRY(builder.try_append(" }"sv));
    return builder.to_string();
}

ErrorOr<ClipboardMessage> ClipboardMessage::read_from_stream(AK::Stream& stream)
{
    auto value = TRY(stream.read_value<u32>());
    if (value >= to_underlying(ClipboardDataType::__End)) {
        return Error::from_string_literal("Unsupported clipboard type");
    }

    auto type = static_cast<ClipboardDataType>(value);
    auto contents = TRY(stream.read_until_eof());
    return ClipboardMessage(type, contents);
}

ErrorOr<void> ClipboardMessage::write_to_stream(AK::Stream& stream)
{
    TRY(stream.write_value(data_type()));
    TRY(stream.write_until_depleted(contents()));

    return {};
}

ErrorOr<String> ClipboardMessage::debug_description()
{
    StringBuilder builder;
    TRY(builder.try_append("Clipboard { "sv));
    TRY(builder.try_appendff("data_type = {}, ", data_type()));
    TRY(builder.try_appendff("contents.size() = {}", contents().size()));
    TRY(builder.try_append(" }"sv));
    return builder.to_string();
}

}