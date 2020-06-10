import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

Page {
  GridLayout {
    anchors.fill: parent
    columns: 2
    Label { text: "Android build: "; }
    Label { text: EmacsBridge.mobile; }
    Label { text: "UI startup time: "; }
    Label { text: EmacsBridge.startupTime }
    Label { text: "Service startup time: "; }
    Label { text: EmacsBridge.serviceStartupTime }
    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true
    }
  }
}