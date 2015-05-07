import QtQuick 2.2
import QtQuick.Controls 1.1

Item {
  //anchors.fill: parent
  clip: true
  anchors.margins: 5

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
    anchors { fill: parent }
    cellWidth: 100
    cellHeight: 100
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
        //console.log("entered");
        //frontLayer.border.width = 10;
      }

      onExited: {
        //console.log("exited");
        //frontLayer.border.width = 0;
      }
    }
  }

  /*
  Rectangle {
    id: frontLayer
    anchors.fill:parent

    color: "transparent"

    border {
      width: 0
      color: "blue"
    }

    Behavior on border.width {
      PropertyAnimation {
        duration: 500
      }
    }
  }*/
}
