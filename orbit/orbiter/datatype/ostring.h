// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_DATATYPE_OSTRING_H_
#define ORBIT_ORBITER_DATATYPE_OSTRING_H_

#include <orbit/orbiter/context.h>

#include <orbit/orbiter/datatype/oobject.h>

namespace orbiter::datatype {
    enum class StringKind {
        ASCII,
        UTF8_2,
        UTF8_3,
        UTF8_4
    };

    struct String {
        OROBJ_HEAD;

        /* Raw buffer */
        unsigned char *buffer;

        /* String mode */
        StringKind kind;

        /* Interned string */
        bool intern;

        /* Length in bytes */
        MSize length;

        /* Number of graphemes in string */
        MSize cp_length;

        /* String hash */
        MSize hash;
    };

    /**
     * @brief Set up additional features and properties for the specified type
     *
     * This function enriches the previously created type with various functionalities.
     * It typically performs the following tasks:
     * - Adds default methods to the type
     * - Adds required properties to the type
     *
     * This function is called immediately after the type's Init function to complete its setup.
     *
     * @param ctx Pointer to the Context in which the type is being set up
     * @param self Pointer to TypeInfo created by %type%Init call
     *
     * @return true if setup was successful, false otherwise
     */
    bool StringTypeSetup(Context *ctx, TypeInfo *self);

    /**
     * @brief Creates an exact copy of a String object in the String pool and return it
     *
     * @param ctx Pointer to the Context
     * @param string The C-string to convert to Orbit string
     * @param length The length of the C-string
     *
     * @return A pointer to an Orbit string object, otherwise nullptr
     */
    String *StringIntern(Context *ctx, const unsigned char *string, MSize length);

    /**
     * @brief Creates an exact copy of a String object in the String pool and return it
     *
     * @param ctx Pointer to the Context
     * @param string The C-string to convert to Orbit string
     *
     * @return A pointer to an Orbit string object, otherwise nullptr
     */
    inline String *StringIntern(Context *ctx, const char *string) {
        return StringIntern(ctx, (const unsigned char *) string, strlen(string));
    }

    /**
     * @brief Create new string
     *
     * It allows you to build an empty String object (container only) which must subsequently be filled by the applicant
     *
     * @warning: Buffer must be zero terminated, the value of length MUST NOT include the terminator character,
     * so the buffer must pass the following assertion: buffer[length] == '\0'.
     * Obviously the size of the allocated buffer must be sufficient to also contain the terminator character
     *
     * @param ctx Pointer to the Context
     * @param buffer Raw buffer containing the string (ownership of the buffer will be transferred to the created object)
     * @param length Length of the buffer
     * @param cp_length Number of unicode code point in the buffer
     * @param kind StringKind
     *
     * @return A pointer to an Orbit string object, otherwise nullptr
     */
    String *StringNew(Context *ctx, unsigned char *string, MSize length, MSize cp_length, StringKind kind);

    /**
     * @brief Create new string.
     *
     * @param ctx Pointer to the Context
     * @param string The unsigned C-string to convert to Orbit string.
     * @param length The length of the C-string.
     *
     * @return A pointer to an Orbit string object, otherwise nullptr.
     */
    String *StringNew(Context *ctx, const unsigned char *string, MSize length);

    /**
     * @brief Create new string
     *
     * @param ctx Pointer to the Context
     * @param string The C-string to convert to Orbit string
     * @param length The length of the unsigned C-string
     *
     * @return A pointer to an Orbit string object, otherwise nullptr
     */
    inline String *StringNew(Context *ctx, const char *string, MSize length) {
        return StringNew(ctx, (const unsigned char *) string, length);
    }

    /**
     * @brief Create new string
     *
     * @param string The C-string to convert to Orbit string
     *
     * @return A pointer to an Orbit string object, otherwise nullptr
     */
    inline String *StringNew(Context *ctx, const char *string) {
        return StringNew(ctx, (unsigned char *) string, strlen(string));
    }

    /**
     * @brief Create a new string object using the buffer parameter as an internal buffer
     *
     * The new string object becomes the owner of the buffer passed as a parameter
     *
     * @warning: Buffer must be zero terminated, the value of length MUST NOT include the terminator character,
     * so the buffer must pass the following assertion: buffer[length] == '\0'.
     * Obviously the size of the allocated buffer must be sufficient to also contain the terminator character
     *
     * @param ctx Pointer to the Context
     * @param buffer Raw buffer containing the string
     * (ownership of the buffer will be transferred to the created object).
     * @param length Length of the buffer.
     *
     * @return A pointer to an Orbit string object, otherwise nullptr.
     */
    String *StringNewHoldBuffer(Context *ctx, unsigned char *buffer, MSize length);

    /**
     * @brief Initialize and create the specified type
     *
     * This function creates a new TypeInfo object representing the specific type.
     * It sets up the basic structure and core properties of the type.
     *
     * @param ctx Pointer to the Context in which the type is being created
     *
     * @return Pointer to the newly created TypeInfo for the type, or nullptr if creation failed
     */
    TypeInfo *StringTypeInit(orbiter::Context *ctx);
}

#endif // !ORBIT_ORBITER_DATATYPE_OSTRING_H_
