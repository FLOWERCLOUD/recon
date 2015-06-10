#pragma once

#include <vectormath.h>

namespace vectormath {
namespace utils {

using vectormath::aos::scalar;
using vectormath::aos::scalar4;
using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::mat3;
using vectormath::aos::mat4;
using vectormath::aos::quat;
using vectormath::aos::dualquat;

}
}

#include "utils/AABox.h"
#include "utils/Plane.h"
#include "utils/Frustum.h"
#include "utils/Transform.h"
