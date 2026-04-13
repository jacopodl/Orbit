// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <shared_mutex>
#include <orbit/orbiter/datatype/orstring.h>

#include <orbit/orbiter/datatype/dict.h>

using namespace orbiter::datatype;

bool DictDtor(Dict *self) {
    self->dict.Finalize(nullptr);
    self->dict.~ORHMap();

    self->lock.~AsyncRWLock();

    return true;
}

void DictTrace(const Dict *self, const GCTraceCallback callback, const MSize epoch) {
    for (const auto *cursor = self->dict.iter_begin; cursor != nullptr; cursor = cursor->iter_next) {
        callback((OObject *) cursor->key, epoch);

        callback(cursor->value, epoch);
    }
}

bool orbiter::datatype::DictInsert(Dict *dict, OObject *key, OObject *value) {
    std::unique_lock _(dict->lock);

    ORHEntry *entry;

    dict->dict.Lookup(key, &entry);
    if (entry != nullptr) {
        entry->value = value;

        return true;
    }

    entry = dict->dict.AllocHEntry();
    if (entry == nullptr)
        return false;

    entry->key = key;
    entry->value = value;

    if (!dict->dict.Insert(entry)) {
        dict->dict.FreeHEntry(entry);

        return false;
    }

    return true;
}

bool orbiter::datatype::DictInsert(Dict *dict, const char *key, OObject *value) {
    auto okey = ORStringNew(O_GET_ISOLATE(dict), key);

    if (okey)
        return DictInsert(dict, (OObject *) okey.get(), value);

    return false;
}

bool orbiter::datatype::DictLookup(Dict *dict, OObject *key, HOObject &out_value) {
    std::shared_lock _(dict->lock);

    ORHEntry *entry;

    if (dict->dict.Lookup(key, &entry)) {
        out_value = Handle(entry->value);

        return true;
    }

    return false;
}

bool orbiter::datatype::DictLookup(Dict *dict, const char *key, HOObject &out_value) {
    auto okey = ORStringNew(O_GET_ISOLATE(dict), key);

    if (okey)
        return DictLookup(dict, (OObject *) okey.get(), out_value);

    return false;
}

bool orbiter::datatype::DictRemove(Dict *dict, OObject *key) {
    std::unique_lock _(dict->lock);

    ORHEntry *out;

    if (!dict->dict.Remove(key, &out))
        return false;

    dict->dict.FreeHEntry(out);

    return true;
}

bool orbiter::datatype::DictRemove(Dict *dict, const char *key) {
    const auto okey = ORStringNew(O_GET_ISOLATE(dict), key);
    if (okey)
        return DictRemove(dict, (OObject *) okey.get());

    return false;
}

bool orbiter::datatype::DictTypeSetup(TypeInfo *self) {
    self->dtor = (DtorFn) DictDtor;
    self->trace = (TraceFn) DictTrace;

    return true;
}

HDict orbiter::datatype::DictNew(Isolate *isolate, const U32 size) {
    auto *dict = MakeObject<Dict>(isolate, InstanceType::DICT);

    if (dict != nullptr) {
        new(&dict->dict)ORHMap(isolate);

        const auto ok = size > 0 ? dict->dict.Initialize(size) : dict->dict.Initialize();
        if (!ok) {
            isolate->gc->RawFree((OObject *) dict, false);

            return {};
        }

        new(&dict->lock)sync::AsyncRWLock();
    }

    O_GC_TRACK_RETURN(isolate, dict, true);
}

HOType orbiter::datatype::DictTypeInit(Isolate *isolate) {
    auto dict = MakeType(isolate, "Dict", InstanceType::DICT, sizeof(Dict) - sizeof(OObject), 0, 0);
    return dict;
}
