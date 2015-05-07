import QtQuick 2.2

Item {
  id: root
  anchors.fill: parent
  state: "IMPORT_IMAGES"

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
      onDone: {
        root.state = "CAMERA_CALIBRATION";
      }
    }
  }

  Component {
    id: cameraCalibrationPage

    CameraCalibrationPage {
    }
  }
}
