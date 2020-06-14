import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import fi.aardsoft.emacsbridge 1.0

Page {
  Connections {
    target: EmacsBridge
    function onQueryFinished(key, result) {
      if (key.startsWith("LispEval_")){
        console.log("Query finished");
        historyArea.append("< " + result);
      } else {
        console.log("Ignoring data for "+key);
      }
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
        historyArea.append("> " + workArea.text);
        EmacsBridge.runQuery("LispEval_"+Math.random(), workArea.text);
        workArea.text = "";
      }
    }
  }
}