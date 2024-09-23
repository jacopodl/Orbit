// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <cassert>

#include <orbit/orbiter/datatype/stringbuilder.h>

#include <orbit/orbiter/datatype/ostring.h>

using namespace orbiter::datatype;

#define STR_BUF(str) ((str)->buffer)
#define STR_LEN(str) ((str)->length)

String *MkStringContainer(orbiter::Context *ctx, MSize len, bool mkbuf) {
    auto str = MakeObject<String>(ctx, InstanceType::STRING);

    if (str != nullptr) {
        str->buffer = nullptr;

        if (mkbuf) {
            // +1 is '\0'
            str->buffer = (unsigned char *) orbiter::memory::Alloc(len + 1);
            if (str->buffer == nullptr) {
                Release(str);
                return nullptr;
            }

            // Set terminator
            STR_BUF(str)[(len + 1) - 1] = 0x00;
        }

        str->kind = StringKind::ASCII;
        str->intern = false;
        STR_LEN(str) = len;
        str->cp_length = 0;
        str->hash = 0;
    }

    return str;
}

bool StringInitKind(String *string) {
    StringKind kind = StringKind::ASCII;
    MSize index = 0;
    MSize cp_length = 0;

    string->cp_length = 0;

    while (index < string->length) {
        if (!CheckUnicodeCharSequence(&kind, &cp_length, nullptr,0, string->buffer[index], index)) {
            // TODO: error!
            return false;
        }

        if (kind > string->kind)
            string->kind = kind;

        if (++index == cp_length)
            string->cp_length++;
    }

    return true;
}

bool orbiter::datatype::StringTypeSetup(Context *ctx, TypeInfo *self) {
    return true;
}

String *orbiter::datatype::StringIntern(Context *ctx, const unsigned char *string, MSize length) {
    // TODO: IMPL THIS!
    return StringNew(ctx, string, length);
}

String *orbiter::datatype::StringNew(Context *ctx, unsigned char *buffer, MSize length, MSize cp_length, StringKind kind) {
    assert(buffer[length] == '\0');

    auto *str = MkStringContainer(ctx, length, false);
    if (str != nullptr) {
        str->buffer = buffer;
        str->cp_length = cp_length;
        str->kind = kind;
    }

    return str;
}

String *orbiter::datatype::StringNew(Context *ctx, const unsigned char *string, MSize length) {
    StringBuilder builder{};
    StringKind kind;
    MSize len;
    MSize cp_len;

    builder.Write(string, length, 0);

    auto *buffer = builder.BuildString(nullptr, &len, &cp_len, &kind);
    if (buffer == nullptr && len != 0) {
        assert(false);
        // TODO: ERROR!
    }

    // TODO: do not release buffer of size zero!

    auto *str = StringNew(ctx, buffer, len, cp_len, kind);
    if (str != nullptr)
        builder.Release();

    return str;
}

String *orbiter::datatype::StringNewHoldBuffer(Context *ctx, unsigned char *string, MSize length) {
    assert(string[length] == '\0');

    auto *str = MkStringContainer(ctx, length, false);

    if (str != nullptr) {
        str->buffer = string;

        if (!StringInitKind(str)) {
            str->buffer = nullptr;

            Release(str);

            return nullptr;
        }
    }

    return str;
}

TypeInfo *orbiter::datatype::StringTypeInit(orbiter::Context *ctx) {
    auto *string = MakeType(InstanceType::STRING, sizeof(String) - sizeof(OObject), 0, 0);
    return string;
}
