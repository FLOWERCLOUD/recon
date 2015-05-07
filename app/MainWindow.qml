import QtQuick 2.2
import QtQuick.Controls 1.1

ApplicationWindow {
  id: mainWindow
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

  MainView {
    id: contentView
  }
}
