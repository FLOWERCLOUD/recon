import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
  id: thisPage
  anchors.fill: parent

  signal done()

  ImageGridView {
    anchors {
      left: parent.left
      right: parent.right
      top: parent.top
      bottom: nextButton.top
    }
  }

  Button {
    id: nextButton
    text: "Next"
    anchors {
      right: parent.right
      bottom: parent.bottom
    }

    onClicked: thisPage.done();
  }
}
