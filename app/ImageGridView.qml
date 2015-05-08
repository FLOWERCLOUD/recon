import QtQuick 2.2
import recon 1.0

Rectangle {
  id: root

  property real cellWidth: 100
  property real cellHeight: 100
  readonly property int imageCount: root.document.imageCount
  //property ImageListModel model: ImageListModel {}

  property ReconDocument document: ReconDocument {
    baseUrl: "file:tmp"
  }

  Connections {
    target: root.document

    onImageAdded: {
      console.log("image added: " + url);
      var data = { 'sourcePath': url.toString() };
      imageListModel.append(data);
    }
  }

  ListModel {
    id: imageListModel
  }

  Component {
    id: imageCellDelegate

    Item {
      width: gridview.cellWidth
      height: gridview.cellHeight

      Image {
        anchors.fill: parent
        anchors.margins: 2
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectFit
        source: model.sourcePath
      }
    }
  }

  GridView {
    id: gridview
    anchors {
      fill: parent
      margins: 5
    }
    cellWidth: root.cellWidth
    cellHeight: root.cellHeight
    model: imageListModel
    delegate: imageCellDelegate

    DropArea {
      anchors.fill: parent

      onDropped: {
        if (drop.hasUrls) {
          var flag = false;

          drop.urls.forEach(function(url, index, array){
            if (root.document.importImage(url)) {
              flag = true;
            }
          })

          if (flag) {
            drop.accept(Qt.CopyAction);
          }
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
