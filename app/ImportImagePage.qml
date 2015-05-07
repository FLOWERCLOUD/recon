import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
  id: thisPage
  anchors.fill: parent

  signal done()

  ImageGridView {
    id: imageView
    anchors {
      left: parent.left
      right: parent.right
      top: parent.top
      bottom: nextButton.top
    }

    cellWidth: cellSizeSlider.value
    cellHeight: cellSizeSlider.value
  }

  Text {
    id: imageCountText
    text: "%1 images".arg(imageView.imageCount)
    width: 100
    horizontalAlignment: Text.AlignHCenter
    anchors {
      left: parent.left
      right: cellSizeSlider.left
      bottom: parent.bottom
    }
  }

  Slider {
    id: cellSizeSlider
    anchors {
      left: imageCountText.right
      right: nextButton.left
      bottom: parent.bottom
    }
    maximumValue: 512
    minimumValue: 64
    value: 128
  }

  Button {
    id: nextButton
    text: "Next"
    anchors {
      right: parent.right
      bottom: parent.bottom
    }

    onClicked: thisPage.done();
    enabled: imageView.imageCount > 5
  }
}
