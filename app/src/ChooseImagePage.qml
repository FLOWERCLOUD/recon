import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
  id: root
  anchors.fill: parent

  property real cellWidth: cellSizeSlider.value
  property real cellHeight: cellSizeSlider.value

  signal done(var imagePaths)

  ListModel {
    id: imageListModel
  }

  ScrollView {
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      bottom: cellSizeSlider.top
    }

    GridView {
      id: gridView
      //anchors {
      //  fill: parent
      //  margins: 5
      //}

      cellWidth: root.cellWidth
      cellHeight: root.cellHeight

      model: imageListModel

      delegate: Component {
        Item {
          width: root.cellWidth
          height: root.cellHeight
          Image {
            anchors.fill: parent
            anchors.margins: 2
            asynchronous: true
            fillMode: Image.PreserveAspectFit
            source: model.sourcePath
            sourceSize.width: 512
            sourceSize.height: 512
          }
        }
      }

      DropArea {
        anchors.fill: parent
        onDropped: {
          if (drop.hasUrls) {
            var flag = false;
            drop.urls.forEach(function(url, index, array){
              var data = { 'sourcePath': url };
              imageListModel.append(data);
              flag = true;
            })
            if (flag) {
              drop.accept(Qt.CopyAction);
            }
          }
        }
      } // END of DropArea
    } // END of gridView
  } // END of ScrollView

  Text {
    id: imageCountText
    text: "%1 images".arg(imageListModel.count)
    width: 100
    horizontalAlignment: Text.AlignHCenter
    anchors {
      left: parent.left
      bottom: parent.bottom
    }
  }

  Slider {
    id: cellSizeSlider
    anchors {
      left: imageCountText.right
      right: nextButton.left
      bottom: parent.bottom
      leftMargin: 5
      rightMargin: 5
    }
    maximumValue: 512
    minimumValue: 64
    value: 128
  }

  Button {
    id: nextButton
    text: qsTr("Next")
    anchors {
      right: parent.right
      bottom: parent.bottom
    }
    onClicked: root.notifyDone();
    enabled: imageListModel.count > 5
  }

  function getImagePaths() {
    var n = imageListModel.count;
    var images = new Array(n);
    for (var i = 0; i < n; ++i) {
      var element = imageListModel.get(i);
      images[i] = element.sourcePath;
    }
    return images;
  }

  function notifyDone() {
    root.done(getImagePaths());
  }

}
