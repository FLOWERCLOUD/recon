#include "numerical_method.h"

namespace recon {

QList<QPointF> first_order(const QList<QPointF>& data)
{
  QList<QPointF> d_data;
  d_data.reserve(data.size());

  for (int i = 0, n = data.size(); i < n; ++i) {
    int i1 = (i > 0 ? i-1 : 0);
    int i2 = (i < n-1 ? i+1 : i);
    float y = (data[i2].y() - data[i1].y()) / (data[i2].x() - data[i1].x());
    float x = data[i].x();
    d_data.append(QPointF(x, y));
  }

  return d_data;
}

QList<QPointF> second_order(const QList<QPointF>& data)
{
  QList<QPointF> d2_data;
  d2_data.reserve(data.size());

  for (int i = 0, n = data.size(); i < n; ++i) {
    int i1 = (i > 0 ? i-1 : 0);
    int i2 = (i < n-1 ? i+1 : i);

    float dx2 = data[i2].x() - data[i].x();
    float dx1 = data[i].x() - data[i1].x();
    float dx = data[i2].x() - data[i1].x();

    float y = dx1 * data[i2].y() - dx * data[i].y() + dx2 * data[i1].y();
    y /= 0.5f * dx2 * dx1 * dx;

    float x = data[i].x();
    d2_data.append(QPointF(x, y));
  }

  return d2_data;
}

QList<float> find_roots(const QList<QPointF>& data)
{
}

QList<float> local_maxima(const QList<QPointF>& data)
{
}

}
