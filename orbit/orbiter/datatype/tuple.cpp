// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/datatype/dict.h>

#include <orbit/orbiter/datatype/tuple.h>

using namespace orbiter::datatype;

bool TupleDtor(const Tuple *self) {
    const orbiter::memory::IsolateAllocator allocator(O_GET_ISOLATE(self));

    allocator.free(self->objects);

    return true;
}

void TupleTrace(const Tuple *self, const GCTraceCallback callback, const MSize epoch) {
    for (int i = 0; i < self->length; i++) {
        const auto obj = self->objects[i];

        if (O_IS_OBJECT(obj))
            callback(obj, epoch);
    }
}

bool orbiter::datatype::TupleAppend(Tuple *tuple, OObject *item) {
    if (tuple->length == tuple->capacity)
        return false;

    tuple->objects[tuple->length] = item;

    tuple->length += 1;

    return true;
}

bool orbiter::datatype::TupleTypeSetup(TypeInfo *self) {
    self->dtor = (DtorFn) TupleDtor;
    self->trace = (TraceFn) TupleTrace;

    return true;
}

HOType orbiter::datatype::TupleTypeInit(Isolate *isolate) {
    auto tuple = MakeType(isolate, "Tuple", InstanceType::TUPLE, sizeof(Tuple) - sizeof(OObject), 0, 0);
    return tuple;
}

HTuple orbiter::datatype::TupleNew(Isolate *isolate, const MSize count) {
    auto *tuple = MakeObject<Tuple>(isolate, InstanceType::TUPLE);

    if (tuple != nullptr) {
        memory::IsolateAllocator allocator(isolate);

        tuple->objects = allocator.alloc<OObject *>(count * sizeof(void *));
        if (tuple->objects == nullptr) {
            isolate->gc->RawFree((OObject *) tuple, false);

            return {};
        }

        tuple->capacity = count;
        tuple->length = 0;
        tuple->hash = 0;
    }

    O_GC_TRACK_RETURN(isolate, tuple, true);
}

HTuple orbiter::datatype::TupleNew(OObject *object) {
    if (O_IS_SMI(object))
        return {};

    auto *isolate = O_GET_ISOLATE(object);

    if (O_IS_TYPE(object, InstanceType::DICT)) {
        auto list = (HList) DictKeys((Dict *) object);
        return TupleNewFromList(list);
    }

    if (O_IS_TYPE(object, InstanceType::TUPLE)) {
        const auto *other = (Tuple *) object;
        auto tuple = TupleNew(isolate, other->length);

        for (auto i = 0; i < other->length; i++)
            tuple->objects[i] = other->objects[i];

        tuple->length = other->length;

        return tuple;
    }

    if (O_IS_TYPE(object, InstanceType::LIST)) {
        assert(false);
    }

    return {};
}

HTuple orbiter::datatype::TupleNewFromList(HList &list) {
    auto *isolate = O_GET_ISOLATE(list);

    auto *tuple = MakeObject<Tuple>(isolate, InstanceType::TUPLE);

    if (tuple != nullptr) {
        tuple->objects = list->objects;
        tuple->capacity = list->capacity;
        tuple->length = list->length;
        tuple->hash = 0;

        list->objects = nullptr;
        list->capacity = 0;
        list->length = 0;

        list.reset();
    }

    O_GC_TRACK_RETURN(isolate, tuple, true);
}
