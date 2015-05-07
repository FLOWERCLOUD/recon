import QtQuick 2.2
import QtQuick.Controls 1.1

ApplicationWindow {
  visible: true
  width: 800
  height: 600
  title: qsTr("Recon 3D")
  minimumWidth: 640
  minimumHeight: 480

  menuBar: MenuBar {
    Menu {
      title: qsTr("File")
      MenuItem {
        text: qsTr("Open")
      }
      MenuItem {
        text: qsTr("Quit")
        onTriggered: Qt.quit();
      }
    }
  }

  ImageGridView {
    //anchors.fill: undefined
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: nextButton.top
  }

  Button {
    id: nextButton
    text: "Next"
    anchors {
      right: parent.right
      bottom: parent.bottom
    }
  }
}
