#pragma once

#include <string>

#include "core/mesh.h"

struct ObjLoadResult {
    Mesh mesh;
    bool ok = false;
    std::string error;
};

ObjLoadResult load_obj(const std::string& filename);
