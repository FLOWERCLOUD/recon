import QtQuick 2.2

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
        cache: true
        fillMode: Image.PreserveAspectFit
        source: sourcePath
      }
    }
  }

  ListModel {
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
              var data = { 'sourcePath': url };
              imageListModel.append(data);
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
