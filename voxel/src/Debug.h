#pragma once

#include "AABB.h"
#include <vectormath.h>
#include <QDebug>

QDebug& operator<<(QDebug&, const vectormath::aos::vec3&);
QDebug& operator<<(QDebug&, const recon::AABB&);
