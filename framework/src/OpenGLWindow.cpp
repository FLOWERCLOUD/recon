#include "OpenGLWindow.h"
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>

namespace recon {

OpenGLWindow::OpenGLWindow(QWindow* parent)
: QWindow(parent)
, m_UpdatePending(false)
, m_Context(NULL)
, m_Device(NULL)
{
  setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::~OpenGLWindow()
{
  if (m_Device) {
    delete m_Device;
  }
}

void OpenGLWindow::render(QPainter* painter)
{
  Q_UNUSED(painter);
}

void OpenGLWindow::render()
{
  if (!m_Device) {
    m_Device = new QOpenGLPaintDevice();
  }

  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_Device->setSize(size());

  QPainter painter(m_Device);
  render(&painter);
}

void OpenGLWindow::initialize()
{
}

void OpenGLWindow::renderLater()
{
  if (!m_UpdatePending) {
    m_UpdatePending = true;
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
  }
}

bool OpenGLWindow::event(QEvent* event)
{
  if (event->type() == QEvent::UpdateRequest) {
    m_UpdatePending = false;
    renderNow();
    return true;
  }

  return QWindow::event(event);
}

void OpenGLWindow::exposeEvent(QExposeEvent* event)
{
  Q_UNUSED(event);

  if (isExposed()) {
    renderNow();
  }
}

void OpenGLWindow::renderNow()
{
  if (!isExposed()) {
    return;
  }

  bool needsInitialize = false;

  if (!m_Context) {
    m_Context = new QOpenGLContext(this);
    m_Context->setFormat(requestedFormat());
    m_Context->create();

    needsInitialize = true;
  }

  m_Context->makeCurrent(this);

  if (needsInitialize) {
    initializeOpenGLFunctions();
    initialize();
  }

  render();

  m_Context->swapBuffers(this);
}

}
