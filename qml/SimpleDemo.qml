import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import fi.aardsoft.emacsbridge 1.0

Page {
  Connections {
    target: EmacsBridge
    function onQueryFinished(key, result){
      if (key.startsWith("SimpleDemo_1")){
        resultLabel1.text=result;
      }else if(key.startsWith("SimpleDemo_2")){
        resultLabel2.text=result;
      }
    }
    function onDataSet(requesterId, json){
      if (requesterId=="SimpleDemo_2"){
        resultLabel3.text=json["emacs-version"]
      }
    }
  }

  ColumnLayout {
    anchors.fill: parent
    Label { text: "Emacs return value: "; }
    Label { id: resultLabel1; }
    Button {
      Layout.fillWidth: true
      text: "Query emacs (Lisp only)"
      onClicked: {
        EmacsBridge.runQuery("SimpleDemo_1"+Math.random(), "(emacs-version)");
      }
    }
    Label { text: "Emacs return value: "; }
    Label { id: resultLabel2; }
    Label { text: "Data parsed out of JSON: "; }
    Label { id: resultLabel3; }
    Button {
      Layout.fillWidth: true
      text: "Query emacs (Lisp and JSON)"
      onClicked: {
        EmacsBridge.runQuery("SimpleDemo_2"+Math.random(),
             "(emacsbridge-post-json \"setData\"
                       (json-encode `((:requester-id . \"SimpleDemo_2\")
                                      (:emacs-version . ,(emacs-version)))))");
      }
    }
    Item {
      Layout.fillWidth: true
      Layout.fillHeight: true
    }
  }
}