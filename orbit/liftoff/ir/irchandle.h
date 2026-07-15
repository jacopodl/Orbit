// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_IRCHANDLE_H_
#define ORBIT_LIFTOFF_IR_IRCHANDLE_H_

#include <orbit/liftoff/ir/ircontext.h>

namespace liftoff::ir {
    class IRCHandle {
        IRContext *ir_;

    public:
        IRCHandle() noexcept : ir_(nullptr) {
        }

        explicit IRCHandle(IRContext *ir) noexcept : ir_(ir) {
        }

        IRCHandle(const IRCHandle &) = delete;

        IRCHandle(IRCHandle &&other) noexcept : ir_(other.ir_) {
            other.ir_ = nullptr;
        }

        ~IRCHandle() noexcept {
            this->reset();
        }

        IRCHandle &operator=(const IRCHandle &) = delete;

        IRCHandle &operator=(IRCHandle &&other) noexcept {
            if (this != &other) {
                this->reset();

                this->ir_ = other.ir_;

                other.ir_ = nullptr;
            }

            return *this;
        }

        explicit operator bool() const noexcept {
            return this->ir_ != nullptr;
        }

        std::remove_pointer_t<IRContext> &operator*() const {
            assert(this->ir_ != nullptr);

            return *this->ir_;
        }

        IRContext *operator->() const noexcept {
            return this->ir_;
        }

        [[nodiscard]] IRContext *get() const noexcept {
            return this->ir_;
        }

        IRContext *release() noexcept {
            auto *temp = this->ir_;

            this->ir_ = nullptr;

            return temp;
        }

        void reset() noexcept {
            if (this->ir_ != nullptr) {
                IRContext::Delete(this->ir_);

                this->ir_ = nullptr;
            }
        }
    };
}

#endif // !ORBIT_LIFTOFF_IR_IRCHANDLE_H_
