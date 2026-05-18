// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <new>

#include <orbit/orbiter/datatype/list.h>
#include <orbit/orbiter/datatype/orstring.h>

#include <orbit/orbiter/import/importer.h>

using namespace orbiter::datatype;
using namespace orbiter::import;

// *********************************************************************************************************************
// PUBLIC API
// *********************************************************************************************************************

bool Importer::Initialize() {
    this->roots_ = ListNew(this->isolate_);

    return (bool) this->roots_;
}

bool Importer::AddRoot(const char *path) {
    const auto root = ORStringNew(this->isolate_, path);
    if (!root)
        return false;

    return this->AddRoot(root.get());
}

bool Importer::AddRoot(ORString *path) const {
    return ListAppend(this->roots_.get(), (OObject *) path);
}

