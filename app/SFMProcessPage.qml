import QtQuick 2.2
import QtQuick.Controls 1.1
import recon 1.0

Rectangle {
  id: root
  anchors.fill: parent
  color: "grey"

  //property ReconSFMContext context
  property ReconImageSet images

  signal done()

  ReconSFMContext {
    id: sfmContext
    images: root.images
  }

  Connections {
    target: sfmContext

    onFinished: {
      nextButton.enabled = true;
    }
  }

  Component.onCompleted: {
    sfmContext.start();
  }

  Button {
    id: nextButton
    text: "Next"
    anchors {
      right: parent.right
      bottom: parent.bottom
    }

    onClicked: root.done();
    enabled: false
  }
}
