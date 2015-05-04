#include <Recon/Document.h>
#include <Recon/NVMLoader.h>
#include <Recon/SFMWorker.h>
#include "OpenGLWindow.h"

#include <QtCore>
#include <QtDebug>
#include <QGuiApplication>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>

class BundleWindow : public recon::OpenGLWindow {
public:
  BundleWindow();

  void initialize() override;
  void render() override;

protected:
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  int m_CameraIndex;
  GLuint m_FeatureVBO;
  GLint m_ModelViewMatrixUniform;
  GLint m_CalibrationMatrixUniform;

  QOpenGLShaderProgram *m_Program;
  recon::Document m_Document;
};

BundleWindow::BundleWindow()
: m_Document("data/e100vs/workspace")
{
  m_CameraIndex = 0;

  recon::SFMWorker* sfmworker = new recon::SFMWorker(&document);
  sfmworker.start();
}

void BundleWindow::keyPressEvent(QKeyEvent* event)
{
  switch (event->key()) {
    case Qt::Key_Right:
      m_CameraIndex += 1;
      renderLater();
      break;
    case Qt::Key_Left:
      m_CameraIndex -= 1;
      renderLater();
      break;
  }
}

void BundleWindow::initialize()
{
  qDebug() << "CWD = " << QDir::currentPath();
  qDebug() << "DOC BASEPATH = " << m_Document.basePath();

  // Load bundle
  recon::NVMLoader("data/e100vs/bundle.nvm").load(&m_Document);
  m_Document.save();

  const QVector<recon::Camera>& cameras = m_Document.cameras();
  const QVector<recon::Feature>& features = m_Document.features();
  qDebug() << cameras.size() << " cameras";
  qDebug() << features.size() << " points";

  const char* version = (const char*) glGetString(GL_VERSION);
  qDebug() << "OpenGL Version = " << version;

  // Copy to vertex buffer
  glGenBuffers(1, &m_FeatureVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_FeatureVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(recon::Feature) * features.size(), features.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Load Shader
  m_Program = new QOpenGLShaderProgram(this);
  {
    QFile file("shader/bundle_feature.vert");
    file.open(QIODevice::ReadOnly);
    if (!m_Program->addShaderFromSourceCode(QOpenGLShader::Vertex, file.readAll())) {
      qDebug() << "Failed to compile vertex shader:\n" << m_Program->log();
    }
    file.close();
  }
  {
    QFile file("shader/bundle_feature.frag");
    file.open(QIODevice::ReadOnly);
    if (!m_Program->addShaderFromSourceCode(QOpenGLShader::Fragment, file.readAll())) {
      qDebug() << "Failed to compile fragment shader:\n" << m_Program->log();
    }
    file.close();
  }
  m_Program->bindAttributeLocation("in_Position", 0);
  m_Program->bindAttributeLocation("in_Color", 1);
  if (!m_Program->link()) {
    qDebug() << "Failed to link program:\n" << m_Program->log();
  }

  m_ModelViewMatrixUniform = m_Program->uniformLocation("u_ModelViewMatrix");
  m_CalibrationMatrixUniform = m_Program->uniformLocation("u_CalibrationMatrix");
}

void BundleWindow::render()
{
  using vectormath::aos::vec3;
  using vectormath::aos::vec4;
  using vectormath::aos::mat3;
  using vectormath::aos::mat4;
  using vectormath::aos::load_vec3;
  using vectormath::aos::load_mat3;
  using vectormath::aos::store_mat3;
  using vectormath::aos::store_mat4;

  glViewport(0, 0, width(), height());

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const QVector<recon::Camera>& cameras = m_Document.cameras();
  const QVector<recon::Feature>& features = m_Document.features();

  while (m_CameraIndex < 0)
    m_CameraIndex += cameras.size();
  m_CameraIndex = m_CameraIndex % cameras.size();

  const recon::Camera& cam = cameras[m_CameraIndex];

  m_Program->bind();
  {
    GLfloat buf[4][4];

    mat3 rot = load_mat3(cam.extrinsic);
    vec3 trans = load_vec3(cam.extrinsic+9);
    mat4 modelview = make_mat4(rot, trans);
    store_mat4((GLfloat*)buf, modelview);

    m_Program->setUniformValue(m_ModelViewMatrixUniform, buf);
  }
  {
    GLfloat buf[3][3];
    mat3 m1 = load_mat3(cam.intrinsic);

    store_mat3((GLfloat*)buf, m1);
    m_Program->setUniformValue(m_CalibrationMatrixUniform, buf);
  }

  glEnable(GL_PROGRAM_POINT_SIZE);

  glBindBuffer(GL_ARRAY_BUFFER, m_FeatureVBO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(recon::Feature), (void*)offsetof(recon::Feature, pos));
  glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(recon::Feature), (void*)offsetof(recon::Feature, color));
  glDrawArrays(GL_POINTS, 0, features.size());

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  m_Program->release();
}

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  QSurfaceFormat format;
  format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  format.setRedBufferSize(8);
  format.setGreenBufferSize(8);
  format.setBlueBufferSize(8);
  format.setAlphaBufferSize(8);
  format.setDepthBufferSize(24);

  BundleWindow window;
  window.setFormat(format);
  window.resize(800, 600);
  window.show();

  return app.exec();

  return 0;
}
