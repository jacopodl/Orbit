// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/runtime.h>

using namespace orbiter;

Orbiter *Orbiter::orbiter_ = nullptr;

OSThread *Orbiter::AllocOSThread() noexcept {
    auto *ost = new(std::nothrow) OSThread();
    if (ost != nullptr)
        this->ost_total += 1;

    return ost;
}

void Orbiter::FreeOSThread(OSThread *ost) noexcept {
    if (ost != nullptr) {
        assert(ost->thread.get_id() != std::this_thread::get_id());
        assert(!ost->thread.joinable());

        delete ost;

        this->ost_total -= 1;
    }
}

void Orbiter::InitVCores(unsigned int n) {
    if (n == 0) {
        n = std::thread::hardware_concurrency();
        if (n == 0)
            n = kVCoreDefault;
    }

    this->vcores_ = new VCore[n]();
    this->vcores_count_ = n;
}

void Orbiter::OSTActive2Idle(OSThread *ost) noexcept {
    if (ost->idle)
        return;

    std::unique_lock _(ost_lock);

    // TODO: VCore release!

    ost->idle = true;

    ost->RemoveFromQueue();
    ost->PushToQueue(&this->ost_idle);

    this->ost_idle_count += 1;
}

void Orbiter::OSTIdle2Active(OSThread *ost) noexcept {
    if (!ost->idle)
        return;

    std::unique_lock _(ost_lock);

    ost->idle = false;

    ost->RemoveFromQueue();
    ost->PushToQueue(&this->ost_active);

    this->ost_idle_count -= 1;
}

// *********************************************************************************************************************
// PUBLIC
// *********************************************************************************************************************

Orbiter::~Orbiter() {
    // TODO: fill this!

    delete[] this->vcores_;
}

bool Orbiter::Finalize() noexcept {
    // TODO: fill this!
    return false;
}

bool Orbiter::Initialize(const void *config) noexcept {
    if (orbiter_ != nullptr)
        return true;

    orbiter_ = new(std::nothrow) Orbiter();
    if (orbiter_ != nullptr) {
        orbiter_->InitVCores(kVCoreDefault); // TODO: from config!

        return true;
    }

    return false;
}
