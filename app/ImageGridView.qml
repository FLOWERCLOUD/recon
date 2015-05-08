import QtQuick 2.2
import recon 1.0

Rectangle {
  id: root

  property real cellWidth: 100
  property real cellHeight: 100
  readonly property int imageCount: root.imageSet.count
  property ReconImageSet imageSet: ReconImageSet {
    baseUrl: "file:tmp-images"
  }

  Connections {
    target: root.imageSet

    onImageAdded: {
      var path = root.imageSet.urlFromName(name).toString();
      //console.log("image added: " + name + "("+path+")");
      var data = { 'sourcePath': path };
      imageListModel.append(data);
    }
  }

  Component.onCompleted: {
    root.imageSet.reload();
    root.imageSet.names.forEach(function(name,index,array){
      var path = root.imageSet.urlFromName(name).toString();
      var data = { 'sourcePath': path };
      imageListModel.append(data);
    });
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
            if (root.imageSet.importImage(url)) {
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
