import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import fi.aardsoft.emacsbridge 1.0

Page {
  ScrollView {
    height: parent.height
    width: parent.width
    implicitHeight: height
    contentWidth: availableWidth
    clip: true

    GridLayout {
        anchors.fill: parent
        columns: 2

      Label {
        text: "This page displays the status of the Emacs bridge server and allows changing listen address and port. For the Emacs side of the setup check the <a href=\""+EmacsBridge.serverProperty('htmlUrl')+"\">setup documentation.</a>"
        Layout.columnSpan: 2
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        onLinkActivated: Qt.openUrlExternally(link)
      }
      Label {
        text: "Server listen address: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      TextField { id: listenAddressField; text: EmacsBridge.serverListenAddress }
      Label {
        text: "Server listen port: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      TextField { id: listenPortField; text: EmacsBridge.serverListenPort }
      Label {}
      GridLayout {
        columns: 2
        Button {
          text: "Cancel"
          onClicked: {
            listenAddressField.text = EmacsBridge.serverListenAddress;
            listenPortField.text = EmacsBridge.serverListenPort;
          }
        }
        Button {
          text: "Apply changes"
          onClicked: {
            console.log("Server bind: "+listenAddressField.text+":"+listenPortField.text);
            EmacsBridge.serverListenAddress = listenAddressField.text;
            EmacsBridge.serverListenPort = listenPortField.text;
          }
        }
      }
      Label {
        text: "Active server port: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label { text: EmacsBridge.activeServerListenPort; }
      Label {
        text: "Version: "
      }
      Label { text: Qt.application.version; }
      Label {
        text: "Android build: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label {
        text: EmacsBridge.mobile
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label {
        text: "UI startup time: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label { text: EmacsBridge.startupTime }
      Label {
        text: "Service startup time: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label { text: EmacsBridge.serviceStartupTime }
      Button {
        visible: EmacsBridge.mobile
        text: "Open app settings"
        onClicked: {
          EmacsBridge.openAppSettings()
        }
      }
      Label {
        visible: EmacsBridge.mobile
        text: "To restart configuration open application settings and clear storage"
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label {}
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }
      Label {
        text: "Read the termux specific setup instructions before.."
        Layout.columnSpan: 2
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        onLinkActivated: Qt.openUrlExternally(link)
      }
      Button {
        visible: EmacsBridge.mobile
        text: "Run termux setup"
        onClicked: {
          EmacsBridge.setupTermux()
        }
      }
      Label { visible: EmacsBridge.mobile }

    }
  }
}