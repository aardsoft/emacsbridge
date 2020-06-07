import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

Page {
  ColumnLayout {
    anchors.fill: parent

    Label {
      text: "Notification topic"
      Layout.fillWidth: true
      Layout.fillHeight: false
      height: 1
    }
    TextField {
      id: notificationTopic
      Layout.fillWidth: true
      Layout.fillHeight: false
      height: 1
    }
    Label {
      text: "Notification message"
      Layout.fillWidth: true
      Layout.fillHeight: false
      height: 1
    }
    TextField {
      id: notificationMessage
      text: "Sample notification"
      Layout.fillWidth: true
      Layout.fillHeight: false
      height: 1
    }
    Button {
      text: "Send notification"
      Layout.fillWidth: true
      onClicked: emacsBridge.notification = notificationMessage.text
      Layout.fillHeight: false
      height: 1
    }
    Item {}
    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true
    }
  }
}