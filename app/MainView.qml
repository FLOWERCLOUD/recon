import QtQuick 2.2
import recon 1.0

Item {
  id: root
  anchors.fill: parent
  state: "IMPORT_IMAGES"

  ReconImageSet {
    id: sourceImages
    baseUrl: "file:tmp-images"
  }

  Loader {
    id: pageLoader
    anchors.fill: parent
  }

  states: [
    State {
      name: "IMPORT_IMAGES"
      PropertyChanges {
        target: pageLoader
        sourceComponent: importImagePage
      }
    },
    State {
      name: "PROCESS_SFM"
      PropertyChanges {
        target: pageLoader
        sourceComponent: sfmProcessPage
      }
    },
    State {
      name: "VIEW_SFM"
      PropertyChanges {
        target: pageLoader
        sourceComponent: sfmViewerPage
      }
    }
  ]

  Component {
    id: importImagePage

    ImportImagePage {
      imageSet: sourceImages
      onDone: {
        root.state = "PROCESS_SFM";
      }
    }
  }

  Component {
    id: sfmProcessPage

    SFMProcessPage {
      images: sourceImages
      onDone: {
        root.state = "VIEW_SFM";
      }
    }
  }

  Component {
    id: sfmViewerPage

    Rectangle {
      anchors.fill: parent
      color: "green"
    }
  }

}
