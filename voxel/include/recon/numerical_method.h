#pragma once

#include <QPointF>
#include <QList>

namespace recon {

QList<QPointF> first_order(const QList<QPointF>& data);
QList<QPointF> second_order(const QList<QPointF>& data);

}
