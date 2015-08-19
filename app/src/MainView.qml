import QtQuick 2.2

Item {
  id: root
  state: "IMPORT_IMAGES"

  Loader {
    id: pageLoader
    anchors.fill: parent
  }

  states: [
    State {
      name: "IMPORT_IMAGES"
      PropertyChanges {
        target: pageLoader
        sourceComponent: chooseImagePage
      }
    },
    State {
      name: "PROCESS_SFM"
      PropertyChanges {
        target: pageLoader
        sourceComponent: processSFMPage
      }
    }
  ]

  Component {
    id: chooseImagePage

    ChooseImagePage {
      onDone: {
        //console.log("image paths = " + imagePaths);
        root.state = "PROCESS_SFM";
      }
    }
  }

  Component {
    id: processSFMPage

    Rectangle {
      color: "green"
    }
  }

}
