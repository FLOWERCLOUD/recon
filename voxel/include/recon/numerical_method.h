#pragma once

#include <QPointF>
#include <QList>

namespace recon {

QList<QPointF> first_order(const QList<QPointF>& data);
QList<QPointF> second_order(const QList<QPointF>& data);
QList<float> find_roots(const QList<QPointF>& data);
QList<float> local_maxima(const QList<QPointF>& data);

}
