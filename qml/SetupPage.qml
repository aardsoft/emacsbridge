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
      id: l
      anchors.fill: parent
      columns: 2

      function copyOrOpenUrl(url){
        if (copyUrlBox.checked){
          EmacsBridge.copyToClipboard(url);
        } else {
          Qt.openUrlExternally(url);
        }
      }

      Label {
        text: "On mobile devices Emacs bridge can be used with <a href=\"https://termux.com/\">Termux</a> with or without integration, or with any other runtime providing the ability to run Emacs<br/><br/>To run with full integration Termux needs to be configured to allow external app access. If you did not configure this yet click on the 'Copy access script' button below, and paste the result into Termux. Now you can tap the 'Run termux setup' button, which will install missing requirements, and provide a starting configuration for Emacs - if Emacs is already installed it'll try to integrate with the existing configuration.<br/><br/>If you don't want to allow external app access for Termux you can still use the simplified setup by tapping the 'Copy setup script' button, and paste it into Termux - this will just execute the setup script, without enabling external app access<br/><br/>For a completely manual setup please follow the <a href=\""+EmacsBridge.serverProperty('htmlUrl')+"\">quick setup documentation.</a> After this is working you'll have to add the Emacs side startup code to your init files yourself."
        Layout.columnSpan: 2
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        onLinkActivated: l.copyOrOpenUrl(link)
        visible: !EmacsBridge.configured && EmacsBridge.mobile
      }
      Label {
        text: "On a desktop the recommended way of connecting to Emacs bridge is through the local socket. To set that up click on the 'Copy the script to setup PC access to clipboard' button below, and paste the text into a terminal.<br/><br/>If you prefer manual configuration follow the <a href=\""+EmacsBridge.serverProperty('htmlUrl')+"\">quick setup documentation.</a>.<br/><br/>If you don't want Emacs bridge to load the lisp files from the included server please read the <a href=\""+EmacsBridge.serverProperty('manualUrl')+"\">user manual</a> for information on how to set this up."
        Layout.columnSpan: 2
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        onLinkActivated: l.copyOrOpenUrl(link)
        visible: !EmacsBridge.configured && !EmacsBridge.mobile
      }
      Label {
        text: "This page displays the status of the Emacs bridge server and allows changing listen address and port. For the Emacs side of the setup check the <a href=\""+EmacsBridge.serverProperty('htmlUrl')+"\">setup documentation.</a>"
        Layout.columnSpan: 2
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        onLinkActivated: l.copyOrOpenUrl(link)
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
      Label {}
      CheckBox {
        id: copyUrlBox
        text: "Copy URLs instead of opening"
        checked: false
        Layout.fillWidth: true
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
        text: "Configured: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label {
        text: EmacsBridge.configured
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label {
        text: "Connected to Emacs: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label {
        text: EmacsBridge.connected
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
      Label {
        text: "Emacs last seen: "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Label { text: EmacsBridge.emacsLastSeen }
      Button {
        text: "Open server settings"
        onClicked: {
          l.copyOrOpenUrl(EmacsBridge.serverProperty('settingsUrl'))
        }
      }
      Label {
        text: "Open the server settings page in the web browser"
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
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
        visible: EmacsBridge.mobile
        text: "Please read the <a href=\""+EmacsBridge.serverProperty('termuxManualUrl')+"\">termux specific documentation</a> before clicking on the buttons below."
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
      Label {
        visible: EmacsBridge.mobile
        text: "Run Termux configuration script. This requires Termux to be configured for external app access"
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Button {
        visible: EmacsBridge.mobile
        text: "Copy setup script"
        onClicked: {
          EmacsBridge.copyServerProperty('termuxSetupScript')
        }
      }
      Label {
        visible: EmacsBridge.mobile
        text: "Copy the script to setup Termux without app permissions"
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Button {
        visible: EmacsBridge.mobile
        text: "Copy access script"
        onClicked: {
          EmacsBridge.copyServerProperty('termuxAccessScript')
        }
      }
      Label {
        visible: EmacsBridge.mobile
        text: "Copy the script to setup Termux app permissions to clipboard"
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
      Button {
        visible: !EmacsBridge.mobile
        text: "Copy access script"
        onClicked: {
          EmacsBridge.copyServerProperty('pcSetupScript')
        }
      }
      Label {
        visible: !EmacsBridge.mobile
        text: "Copy the script to setup PC access to clipboard"
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
      }
    }
  }
}