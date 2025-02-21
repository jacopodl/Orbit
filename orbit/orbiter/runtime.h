// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_RUNTIME_H_
#define ORBIT_ORBITER_RUNTIME_H_

#include <thread>

#include <orbit/orbiter/config.h>

#include <orbit/orbiter/fqueue.h>

namespace orbiter {
    constexpr unsigned int kVCoreDefault = 4;
    constexpr unsigned int kVCoreQueueLengthMax = 255;

    class VCore {
    public:
        VCore *next = nullptr;
        VCore **prev = nullptr;

        FiberQueue queue;

        VCore(): queue(kVCoreQueueLengthMax) {
        }
    };

    class OSThread {
        OSThread *next = nullptr;
        OSThread **prev = nullptr;

    public:
        std::thread thread;

        Fiber *last = nullptr;

        bool idle = true;

        void PushToQueue(OSThread **list) noexcept {
            if (*list == nullptr) {
                this->next = nullptr;
                this->prev = list;
            } else {
                this->next = (*list);

                if ((*list) != nullptr)
                    (*list)->prev = &this->next;

                this->prev = list;
            }

            *list = this;
        }

        void RemoveFromQueue() noexcept {
            if (this->prev != nullptr)
                *this->prev = this->next;
            if (this->next != nullptr)
                this->next->prev = this->prev;

            this->next = nullptr;
            this->prev = nullptr;
        }
    };

    class Orbiter {
        static Orbiter *orbiter_;

        std::mutex ost_lock;

        // OSThread variables
        OSThread *ost_active = nullptr; // Working OSThread
        OSThread *ost_idle = nullptr; // IDLE OSThread

        // VCores variables
        VCore *vcores_ = nullptr; // List of instantiated VCore
        VCore *vcores_active_ = nullptr; // Active VCore

        // Runtime Counters
        unsigned int ost_total = 0; // OSThread counter
        unsigned int ost_idle_count = 0; // OSThread counter (idle)
        unsigned int ost_max = 0; // Maximum OS thread allowed

        unsigned int vcores_count_ = 0;

        Orbiter() = default;

        OSThread *AllocOSThread() noexcept;

        void FreeOSThread(OSThread *ost) noexcept;

        void InitVCores(unsigned int n);

        void OSTActive2Idle(OSThread *ost) noexcept;

        void OSTIdle2Active(OSThread *ost) noexcept;

    public:
        Orbiter(const Orbiter &) = delete;

        Orbiter &operator=(const Orbiter &) = delete;

        Orbiter(Orbiter &&) = delete;

        Orbiter &operator=(Orbiter &&) = delete;

        ~Orbiter();

        bool Finalize() noexcept;

        static bool Initialize(const void *config) noexcept;

        static Orbiter *GetInstance() noexcept {
            return orbiter_;
        }
    };
}

#endif // !ORBIT_ORBITER_RUNTIME_H_
