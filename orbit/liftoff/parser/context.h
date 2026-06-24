// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_PARSER_CONTEXT_H_
#define ORBIT_LIFTOFF_PARSER_CONTEXT_H_

#include <orbit/liftoff/parser/parser.h>

namespace liftoff::parser {
    enum class ContextType {
        CDTOR,
        CLASS,
        FUNC,
        LOOP,
        MODULE,
        TRAIT,
        SWITCH
    };

    class Context {
        Context *back_;
        Parser *parser_;

        ContextType type_;

    public:
        int anon_count = 0;

        [[nodiscard]] bool Check(const ContextType type) const noexcept {
            return this->type_ == type;
        }

        [[nodiscard]] bool CheckBack(const ContextType type) const noexcept {
            return this->back_ != nullptr && this->back_->type_ == type;
        }

        [[nodiscard]] bool CheckExt(const ContextType type) const noexcept {
            auto cursor = this;

            while (cursor != nullptr) {
                if (cursor->type_ == type)
                    return true;

                cursor = cursor->back_;
            }

            return false;
        }

        /// Walk outward looking for a context of @p type, passing transparently
        /// through control-flow blocks (`loop`/`switch`) but stopping at the
        /// first real scope boundary. Returns true only if @p type is reached
        /// before any non-control scope.
        [[nodiscard]] bool CheckEnclosingScope(const ContextType type) const noexcept {
            auto cursor = this;

            while (cursor != nullptr) {
                if (cursor->type_ == type)
                    return true;

                // Any non-control context is a hard scope boundary.
                if (!IsControl(cursor->type_))
                    return false;

                cursor = cursor->back_;
            }

            return false;
        }

        /// Control-flow contexts are transparent to enclosing-scope lookups: a
        /// `loop`/`switch` does not open a new declaration scope, so a search for
        /// the enclosing function/class/module must look straight through them.
        [[nodiscard]] static constexpr bool IsControl(const ContextType type) noexcept {
            return type == ContextType::LOOP || type == ContextType::SWITCH;
        }

        explicit Context(Parser *parser, const ContextType type) : back_(parser->context_), parser_(parser), type_(type) {
            parser->context_ = this;
        }

        ~Context() {
            this->parser_->context_ = this->back_;
        }
    };
}

#endif // !ORBIT_LIFTOFF_PARSER_CONTEXT_H_
