import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import fi.aardsoft.emacsbridge 1.0

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