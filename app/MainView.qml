import QtQuick 2.2
import recon 1.0

Item {
  id: root
  anchors.fill: parent
  state: "IMPORT_IMAGES"

  property ReconImageSet sourceImages: ReconImageSet {
    baseUrl: "file:tmp-images"
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
      name: "CAMERA_CALIBRATION"
      PropertyChanges {
        target: pageLoader
        sourceComponent: cameraCalibrationPage
      }
    }
  ]

  Loader {
    id: pageLoader
    anchors.fill: parent
  }

  Component {
    id: importImagePage

    ImportImagePage {
      imageSet: root.sourceImages
      onDone: {
        root.state = "CAMERA_CALIBRATION";
      }
    }
  }

  Component {
    id: cameraCalibrationPage

    CameraCalibrationPage {
      sourceImages: root.sourceImages
    }
  }
}
