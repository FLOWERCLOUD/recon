#pragma once

#include <QWindow>
#include <QOpenGLFunctions>
class QOpenGLContext;
class QOpenGLPaintDevice;

namespace recon {

class OpenGLWindow : public QWindow, protected QOpenGLFunctions {
  Q_OBJECT
public:
  explicit OpenGLWindow(QWindow* parent = 0);
  ~OpenGLWindow();

  virtual void render(QPainter* painter);
  virtual void render();

  virtual void initialize();

public slots:
  void renderLater();
  void renderNow();

protected:
  virtual bool event(QEvent* event) Q_DECL_OVERRIDE;
  virtual void exposeEvent(QExposeEvent* event) Q_DECL_OVERRIDE;

private:
  bool m_UpdatePending;
  QOpenGLContext* m_Context;
  QOpenGLPaintDevice* m_Device;
};

}
