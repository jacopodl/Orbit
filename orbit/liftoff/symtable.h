// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_SYMTABLE_H_
#define ORBIT_LIFTOFF_SYMTABLE_H_

#include <cassert>

#include <orbit/orbiter/isolate.h>

#include <orbit/orbiter/datatype/hashmap.h>
#include <orbit/orbiter/datatype/orstring.h>

namespace liftoff {
    constexpr int kSTMapSubscopeCapacity = 6;

    using STHEntry = orbiter::datatype::HEntry<orbiter::datatype::ORString *, struct Symbol *>;
    using STHMap = orbiter::datatype::HashMap<
        orbiter::datatype::ORString *,
        Symbol *,
        orbiter::datatype::ORStringEqual,
        orbiter::datatype::ORStringHash
    >;

    enum class AccessModifier {
        PRIVATE,
        PROTECTED,
        PUBLIC
    };

    enum class ScopeType {
        CLASS,
        FUNCTION,
        MODULE,
        NATIVE_FUNC,
        TRAIT
    };

    enum class StorageLocation {
        AUTO,

        CLOSURE,
        GLOBAL,
        MODULE,
        SLOTS,
        STACK
    };

    enum class SymbolFlags : U8 {
        INITIALIZED = 1,

        ANON = 1 << 1,
        CONST = 1 << 2,
        SELF = 1 << 3,
        SYNTHETIC = 1 << 4,
        UPVALUE = 1 << 5
    };

    enum class SymbolType {
        CLASS,
        ECONST,
        FUNC,
        LABEL,
        METHOD,
        MODULE,
        NATIVE_CONST,
        NATIVE_FUNC,
        NATIVE_VAR,
        PARAMETER,
        TRAIT,
        VARIABLE,
        UNKNOWN
    };

    class SubScope {
        STHMap symbols;

        SubScope *child = nullptr;

        SubScope *next_sibling = nullptr;

        SubScope *parent = nullptr;

        struct {
            MSize start;
            MSize end;
        } offset{};

        MSize nesting = 0;

        explicit SubScope(orbiter::Isolate *isolate) : symbols(isolate) {
        }

        friend class Scope;

        friend class SymbolTable;
    };

    class Scope {
        SubScope scope;

        SubScope *active = nullptr;

        explicit Scope(orbiter::Isolate *isolate, const MSize line_start) : scope(isolate) {
            this->line.start = line_start;
        }

        static Scope *New(orbiter::Isolate *isolate, MSize line_start) noexcept;

        friend class SymbolTable;

    public:
        ScopeType type = ScopeType::MODULE;

        Scope *back = nullptr;

        struct {
            MSize start;
            MSize end;
        } line{};
    };

    struct Symbol {
        Symbol *next;

        Scope *decl_scope;

        Scope *defining_scope;

        orbiter::datatype::ORString *name;

        MSize decl_offset;

        AccessModifier access;

        StorageLocation location;

        SymbolFlags flags;

        SymbolType type;

        unsigned short offset;

        unsigned short nesting;
    };

    enum class SymbolTableError {
        OK = 0,
        MEMORY_ERROR,
        SCOPE_NOT_FOUND,
        SYMBOL_ALREADY_EXISTS,
        SYMBOL_NOT_FOUND
    };

    class SymbolTable {
        orbiter::Isolate *isolate = nullptr;

        unsigned short *c_offset = nullptr;

        explicit SymbolTable(orbiter::Isolate *isolate) : isolate(isolate) {
        }

        ~SymbolTable() noexcept;

        [[nodiscard]] Symbol *SymbolNew(orbiter::datatype::ORString *name, SymbolType type, StorageLocation location,
                                        MSize offset) noexcept;

        void ScopeDel(Scope *target) const noexcept;

        void SubScopeDel(SubScope *sub_scope, bool r_memory) const noexcept;

        void SymbolDel(Symbol *symbol) const noexcept;

    public:
        /**
         * @brief Represents the current scope in a symbol table.
         *
         * This pointer is used to manage and navigate the active scope within the symbol table.
         * It is initially set to `nullptr`.
         */
        Scope *scope = nullptr;

        /// @brief Represents the status of operations performed on a symbol table.
        SymbolTableError status = SymbolTableError::OK;

        /**
         * @brief Create a new nested scope at a specified offset.
         * This method is used during parsing and creates and enters a nested scope.
         *
         * @param offset The offset used for scope creation.
         * @return True if the nested scope was successfully created.
         */
        bool CreateNestedScope(MSize offset) noexcept;

        /**
         * @brief Enter an existing nested scope using the specified offset.
         * This method enters a nested scope but does not create it.
         *
         * @param offset The offset used for entering the scope.
         * @return True if the scope was successfully entered, false if not found.
         */
        [[nodiscard]] bool EnterNestedScope(MSize offset) const noexcept;

        /**
         * @brief Enters a new scope with the given name.
         *
         * @param name The name of the new scope.
         * @return True if the scope was successfully entered, false otherwise.
         */
        bool EnterScope(orbiter::datatype::ORString *name) noexcept;

        Symbol *Declare(orbiter::datatype::ORString *name, SymbolType type, StorageLocation location,
                        MSize offset) noexcept;

        Symbol *Declare(const char *name, SymbolType type, StorageLocation location, MSize offset) noexcept;

        /**
         * @brief Declares a new symbol with the specified name, type, and offset.
         *
         * @param name The name of the symbol.
         * @param type The type of the symbol.
         * @param offset The offset of the symbol in source code.
         * @return A pointer to the newly declared symbol.
         */
        Symbol *Declare(orbiter::datatype::ORString *name, const SymbolType type, const MSize offset) noexcept {
            return this->Declare(name, type, StorageLocation::AUTO, offset);
        }

        /**
         * @brief Declares a new symbol with the specified name, type, and offset.
         *
         * @param name The name of the symbol (as a C-string).
         * @param type The type of the symbol.
         * @param offset The offset of the symbol in source code.
         * @return A pointer to the newly declared symbol.
         */
        Symbol *Declare(const char *name, const SymbolType type, const MSize offset) noexcept {
            return this->Declare(name, type, StorageLocation::AUTO, offset);
        }

        /**
         * @brief Declares a new symbol scope with the specified details.
         *
         * @param name The name of the symbol.
         * @param type The type of the symbol.
         * @param offset The offset of the symbol in source code.
         * @param line_start The start line of the symbol scope.
         * @return A pointer to the newly declared symbol.
         */
        Symbol *DeclareSymbolScope(orbiter::datatype::ORString *name, SymbolType type, MSize offset,
                                   MSize line_start) noexcept;

        /**
         * @brief Declares a new symbol scope with the specified details.
         *
         * @param name The name of the symbol (as a C-string).
         * @param type The type of the symbol.
         * @param offset The offset of the symbol in source code.
         * @param line_start The start line of the symbol scope.
         * @return A pointer to the newly declared symbol.
         */
        Symbol *DeclareSymbolScope(const char *name, SymbolType type, MSize offset, MSize line_start) noexcept;

        /**
         * @brief Looks up a symbol by its name in the symbol table, starting from the current scope.
         *
         * This method searches for a symbol with the given name, starting in the active scope and
         * climbing up through the hierarchy of scopes until it is found or the global scope is reached.
         * If the `class_prop` parameter is true, it adjusts the search behavior to include class or trait
         * scopes.
         *
         * @param name The name of the symbol to look for.
         * @param offset The current instruction offset, used to resolve scoped definitions based on execution order.
         * @return A pointer to the discovered Symbol if found, or `nullptr` otherwise.
         */
        Symbol *Lookup(orbiter::datatype::ORString *name, MSize offset) noexcept;

        /**
         * @brief Looks up a symbol with the specified name and offset.
         *
         * @param name The name of the symbol (as a C-string).
         * @param offset The offset of the symbol in source code.
         * @return A pointer to the found symbol or nullptr if not found.
         */
        Symbol *Lookup(const char *name, MSize offset) noexcept;

        /**
         * @brief Looks up a symbol with the specified name and offset, inserting it if not found.
         *
         * @param name The name of the symbol.
         * @param offset The offset of the symbol in source code.
         * @return A pointer to the found or newly inserted symbol.
         */
        Symbol *LookupInsert(orbiter::datatype::ORString *name, MSize offset) noexcept;

        /**
         * @brief Looks up a symbol with the specified name and offset, inserting it if not found.
         *
         * @param name The name of the symbol (as a C-string).
         * @param offset The offset of the symbol in source code.
         * @return A pointer to the found or newly inserted symbol.
         */
        Symbol *LookupInsert(const char *name, MSize offset) noexcept;

        Symbol *LookupMember(orbiter::datatype::ORString *name);

        Symbol *LookupMember(const char *name) noexcept;

        /**
         * @brief Creates a new symbol table with the specified isolate.
         *
         * @param isolate Isolate associated with the symbol table.
         * @return A pointer to the newly created symbol table.
         */
        static SymbolTable *New(orbiter::Isolate *isolate) noexcept;

        /**
         * @brief Leave the nested scope using the specified offset as end position.
         * @note This method is designed to be used during parsing.
         *
         * @param offset The offset used for scope declaration.
         */
        void LeaveNestedScope(MSize offset) const noexcept;

        /**
         * @brief Leave the nested scope.
         * @note This method is designed to step back during various compiler analyses.
         */
        void LeaveNestedScope() const noexcept;

        /**
         * @brief Leave the current scope, effectively ending its lifetime.
         * @note This method is designed to be used during parsing.
         */
        void LeaveScope(MSize offset, MSize line_end) noexcept;

        /**
         * @brief Leave the current scope, effectively ending its lifetime.
         * @note This method is designed to step back during various compiler analyses.
         */
        void LeaveScope() noexcept;
    };
}

ENUMBITMASK_ENABLE(liftoff::SymbolFlags);

#endif // !ORBIT_LIFTOFF_SYMTABLE_H_
