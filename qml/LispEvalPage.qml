import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import fi.aardsoft.emacsbridge 1.0

Page {
  Connections {
    target: emacsBridge
    function onQueryFinished(key, result) {
      console.log("Query finished");
      historyArea.append("< " + result);
    }
  }

  ColumnLayout {
    anchors.fill: parent
    ScrollView {
      Layout.fillHeight: true
      Layout.fillWidth: true
      implicitHeight: 30
      clip: true
      TextArea {
        id: historyArea
        readOnly: true
        selectByMouse: true
        //background: Rectangle { color: "green" }
      }
    }
    Label { text: "Lisp input:" }
    ScrollView {
      Layout.fillHeight: true
      Layout.fillWidth: true
      implicitHeight: 30
      clip: true
      TextArea {
        id: workArea
        selectByMouse: true
        //background: Rectangle { color: "red" }
      }
    }
    Button {
      Layout.fillWidth: true
      text: "Send data"
      onClicked: {
        console.log("Button clicked");
        historyArea.append("> " + workArea.text);
        emacsBridge.runQuery("LispEval_"+Math.random(), workArea.text);
        workArea.text = "";
      }
    }
  }
}