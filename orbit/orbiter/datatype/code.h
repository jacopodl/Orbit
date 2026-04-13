// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_DATATYPE_CODE_H_
#define ORBIT_ORBITER_DATATYPE_CODE_H_

#include <orbit/orbiter/datatype/oobject.h>

#include <orbit/orbiter/opcode.h>
#include <orbit/orbiter/datatype/list.h>
#include <orbit/orbiter/datatype/orstring.h>

#include <orbit/orbiter/native/ffi.h>

namespace orbiter::datatype {
    struct CleanupEntry {
        const unsigned char *m_start;
        const unsigned char *m_end;

        OPCode type;

        U16 slot;
    };

    struct ExportedSymbol {
        ORString *name;

        VariableFlags flags;

        U16 slot;
    };

    struct Code {
        OROBJ_HEAD;

        const unsigned char *m_code;

        const unsigned char *m_end;

        struct {
            CleanupEntry *entries;

            U16 length;
        } cleanup;

        struct {
            ExportedSymbol *symbols;

            U16 length;
        } exported;

        struct {
            native::NativeBinding *bindings;

            U16 length;
        } native;

        List *codes;

        List *static_resources;

        List *unknown_symbols;

        ORString *name;

        ORString *doc;

        union {
            U16 slots_count;
            U16 params_count;
        };

        U16 vars_count;

        U16 stack_size;

        MSize hash;

        void SetProps(ORString *name, ORString *doc) {
            this->name = O_INCREF(name);
            this->doc = O_INCREF(doc);
        }
    };

    using HCode = Handle<Code>;

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
    * @param self Pointer to TypeInfo created by %type%Init call
    *
    * @return true if setup was successful, false otherwise
    */
    bool CodeTypeSetup(TypeInfo *self);

    /**
     * @brief Creates and initializes a new instance of the Code object.
     *
     * This function allocates a new Code object, initializes its properties, and associates it with
     * the provided environment. It sets up the internal code buffer, symbols for resolution, and
     * associated resources. If the object creation fails, it ensures to return null without
     * modifying any of the parameters.
     *
     * @param isolate Pointer to the Isolate in which the Code object is being created.
     * @param m_code Pointer to the code buffer that will be assigned to the new Code object.
     * @param unknown_symbols Pointer to the list of unresolved or unknown symbols required by the code.
     * @param static_resources Pointer to the list of static resources required by the code.
     * @param m_size The size of the provided code buffer in bytes.
     * @param slots_count The number of generic slots defined in the Code object.
     * @param stack_size The size of the execution stack defined in the Code object.
     *
     * @return A handle to the newly created Code object, or null if the creation failed.
     */
    HCode CodeNew(Isolate *isolate, const unsigned char *m_code, List *unknown_symbols, List *static_resources,
                  U32 m_size, U16 slots_count, U16 stack_size);

    /**
     * @brief Initialize and create the specified type
     *
     * This function creates a new TypeInfo object representing the specific type.
     * It sets up the basic structure and core properties of the type.
     *
     * @param isolate Pointer to the Isolate in which the type is being created
     *
     * @return Handle to the newly created TypeInfo for the type, or an empty handle if creation failed
     */
    HOType CodeTypeInit(Isolate *isolate);
}

#endif // !ORBIT_ORBITER_DATATYPE_CODE_H_
