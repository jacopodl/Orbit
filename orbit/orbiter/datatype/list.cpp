// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <cassert>
#include <shared_mutex>

#include <orbit/orbiter/datatype/tuple.h>
#include <orbit/orbiter/datatype/list.h>

using namespace orbiter::datatype;

bool ListCheckSize(List *list, const MSize count) {
    if (list->length + count < list->capacity)
        return true;

    MSize len = kListInitialCapacity;

    if (list->objects != nullptr) {
        const MSize grown = (list->capacity + 1) + ((list->capacity + 1) / 2); // 1.5x

        len = grown > list->capacity + count ? grown : list->capacity + count;
    }

    orbiter::memory::IsolateAllocator allocator(O_GET_TYPE(list)->isolate);

    auto **tmp = allocator.realloc(list->objects, len * sizeof(void *));
    if (tmp == nullptr)
        return false;

    list->objects = tmp;
    list->capacity = len;

    return true;
}

bool ListDtor(const List *self) {
    const orbiter::memory::IsolateAllocator allocator(O_GET_TYPE(self)->isolate);

    allocator.free(self->objects);

    return true;
}

void ListTrace(const List *self, GCTraceCallback callback, const MSize epoch) {
    for (auto i = 0; i < self->length; i++) {
        const auto obj = self->objects[i];

        if (O_IS_OBJECT(obj))
            callback(obj, epoch);
    }
}

bool orbiter::datatype::ListAppend(List *list, OObject *object) {
    std::unique_lock _(list->lock);

    if (!ListCheckSize(list, 1))
        return false;

    list->objects[list->length++] = object;

    return true;
}

bool orbiter::datatype::ListAppend(List *list, List *other) {
    std::unique_lock _(list->lock);

    if (other == nullptr)
        return true;

    std::shared_lock other_lock(other->lock, std::defer_lock);

    if (list != other)
        other_lock.lock();

    if (!ListCheckSize(list, other->length))
        return false;

    const auto src_length = other->length;
    for (MSize i = 0; i < src_length; i++)
        list->objects[list->length++] = other->objects[i];

    return true;
}

bool orbiter::datatype::ListExtend(List *list, OObject *other) {
    if (O_IS_TYPE(other, InstanceType::LIST))
        return ListAppend(list, other);

    if (O_IS_TYPE(other, InstanceType::TUPLE)) {
        std::unique_lock _(list->lock);

        const auto count = ((Tuple *) other)->length;
        auto **objects = ((Tuple *) other)->objects;

        if (!ListCheckSize(list, count))
            return false;

        for (auto i = 0; i < count; i++)
            list->objects[list->length + i] = objects[i];

        list->length += count;

        return true;
    }

    assert(false);
}

bool orbiter::datatype::ListInsert(List *list, OObject *object, MSSize index) {
    std::unique_lock _(list->lock);

    if (list->length == 0 || index >= (MSSize) list->length) {
        if (!ListCheckSize(list, 1))
            return false;

        list->objects[list->length++] = object;

        return true;
    }

    index = ((index % (MSSize) list->length) + (MSSize) list->length) % (MSSize) list->length;

    list->objects[index] = object;

    return true;
}

bool orbiter::datatype::ListPrepend(List *list, OObject *object) {
    std::unique_lock _(list->lock);

    if (!ListCheckSize(list, 1))
        return false;

    for (MSize i = list->length; i > 0; i--)
        list->objects[i] = list->objects[i - 1];

    list->objects[0] = object;

    list->length++;

    return true;
}

bool orbiter::datatype::ListTypeSetup(TypeInfo *self) {
    self->dtor = (DtorFn) ListDtor;
    self->trace = (TraceFn) ListTrace;

    return true;
}

HList orbiter::datatype::ListNew(Isolate *isolate, const MSize capacity) {
    auto *list = MakeObject<List>(isolate, InstanceType::LIST);
    if (list == nullptr)
        return {};

    new(&list->lock)std::shared_mutex;
    list->objects = nullptr;
    list->capacity = capacity;
    list->length = 0;

    if (capacity > 0) {
        memory::IsolateAllocator allocator(isolate);

        list->objects = allocator.alloc<OObject *>(capacity * sizeof(void *));
        if (list->objects == nullptr) {
            isolate->gc->RawFree((OObject *) list, false);

            return {};
        }
    }

    O_GC_TRACK_RETURN(isolate, list, true);
}

HOObject orbiter::datatype::ListGet(List *list, bool *success, MSSize index) {
    std::shared_lock _(list->lock);

    *success = false;

    if (list->length == 0 || index >= (MSSize) list->length)
        return {};

    index = ((index % (MSSize) list->length) + (MSSize) list->length) % (MSSize) list->length;

    *success = true;

    return HOObject(list->objects[index]);
}

HOType orbiter::datatype::ListTypeInit(Isolate *isolate) {
    auto list = MakeType(isolate, "List", InstanceType::LIST, sizeof(List) - sizeof(OObject), 0, 0);
    return list;
}

void orbiter::datatype::ListRemove(List *list, MSSize index) {
    std::unique_lock _(list->lock);

    if (list->length == 0 || index >= (MSSize) list->length)
        return;

    index = ((index % (MSSize) list->length) + (MSSize) list->length) % (MSSize) list->length;

    // Move items back
    for (auto i = index + 1; i < list->length; i++)
        list->objects[i - 1] = list->objects[i];

    list->length--;
}
