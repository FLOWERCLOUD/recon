import QtQuick 2.2
import recon 1.0

Rectangle {
  id: root

  property real cellWidth: 100
  property real cellHeight: 100
  readonly property int imageCount: {
    if (gridview.model) {
      gridview.model.count;
    } else {
      0;
    }
  }

  Component {
    id: imageCellDelegate

    Item {
      width: gridview.cellWidth;
      height: gridview.cellHeight;

      Image {
        anchors.fill: parent
        anchors.margins: 2
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectFit
        source: sourcePath
      }
    }
  }

  ImageListModel {
    id: imageListModel
  }

  GridView {
    id: gridview
    anchors {
      fill: parent
      margins: 5
    }
    cellWidth: parent.cellWidth
    cellHeight: parent.cellHeight
    model: imageListModel
    delegate: imageCellDelegate

    DropArea {
      anchors.fill: parent

      onDropped: {
        if (drop.hasUrls) {
          drop.urls.forEach(function(url, index, array){
            if (/\.jpg$/i.test(url.toString())) {
              imageListModel.addImageSource(url);
            }
          })
          drop.accept(Qt.CopyAction);
        }
        exited();
      }

      onEntered: {
        root.border.width = 5;
      }

      onExited: {
        root.border.width = 0;
      }
    }
  }

  clip: true
  border {
    width: 0
    color: "blue"
  }

  Behavior on border.width {
    PropertyAnimation {
    }
  }
}
