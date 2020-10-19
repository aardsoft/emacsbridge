import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls.Material 2.13
import Qt.labs.settings 1.0
import fi.aardsoft.emacsbridge 1.0

ApplicationWindow {
  visible: true
  title: qsTr("Emacs Bridge")
  id: window

  Material.theme: Material.Dark
  Material.accent: Material.purple

  Component.onCompleted: {
    EmacsBridge.initDrawer();
  }

  Settings {
    id: settings
    property string style: "Default"
  }

  Shortcut {
    sequences: ["Esc", "Back"]
    enabled: stackView.depth > 1
        onActivated: navigateBackAction.trigger()
  }

  Action {
    id: navigateBackAction
    icon.name: stackView.depth > 1 ? "back" : "drawer"
    onTriggered: {
      if (stackView.depth > 1) {
                stackView.pop()
        listView.currentIndex = -1
      } else {
        drawer.open()
      }
    }
  }

  header: ToolBar {
    RowLayout {
      spacing: 20
      anchors.fill: parent
      ToolButton {
        action: navigateBackAction
      }
      Label {
        text: "Emacs Bridge"
        elide: Label.ElideRight
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        Layout.fillWidth: true
      }
    }
  }

  Drawer {
    id: drawer
    height: window.height
    interactive: stackView.depth === 1
    width: Math.min(window.width, window.height) / 3 * 2

    function findInModel(model, source){
      for(var i=0;i<model.count;++i)
      if (model.get(i).source==source){
        return model.get(i);
      }
      return null;
    }

    function findPositionInModel(model, source){
      for(var i=0;i<model.count;++i)
      if (model.get(i).source==source){
        return i;
      }
      return null;
    }

    function insertToModel(model, item){
      var oldItem=findInModel(model, item.fileName)
      if (oldItem==null){
        model.append({"title": item.title, "source": item.fileName});
      }else{
        console.log("Component "+item.title+" already exists");
      }
    }

    function removeFromModel(model, item){
      var pos=findPositionInModel(model, item)
      if (pos!=null){
        console.log(pos);
        model.remove(pos);
      }
    }

    ListView {
      id: listView
      focus: true
      currentIndex: -1
      anchors.fill: parent

      Connections {
        target: EmacsBridge
        function onComponentAdded(component){
          console.log("New component added "+component.title);
          drawer.insertToModel(listModel, component)
        }
        function onComponentRemoved(component){
          console.log("Component removed "+component);
          drawer.removeFromModel(listModel, component);
        }
      }

      delegate: ItemDelegate {
        width: parent.width
        text: model.title
        highlighted: ListView.isCurrentItem
        onClicked: {
          listView.currentIndex = index
          let comp = Qt.createComponent(model.source)
          stackView.createFromComponentAndPush(comp)
          drawer.close()
        }
      }

      model: ListModel {
        id: listModel
        ListElement { title: "Lisp eval"; source: "qrc:/qml/LispEvalPage.qml" }
      }

      ScrollIndicator.vertical: ScrollIndicator { }
    }
  }

  StackView {
    id: stackView
    anchors.fill: parent

    // Create an item from the given component (synchronously, if it is ready, or asynchronously if not).
    // Ownership of the component passes, and it is destroyed when completion is done.
    function createFromComponentAndPush(component) {
      let createItemAndPush = (c) => {
        let item = c.createObject(stackView);
        stackView.push(item);
      };

      let isLoadingAsync = false;
      let handleStatusChanged = () => {
        console.log("Component loading status changed: ", component.status);
        switch (component.status) {
        case Component.Ready:
          createItemAndPush(component);
          component.destroy();
          break;
        case Component.Loading:
          if (isLoadingAsync) {
            console.warn("Component tried to load asynchronously while it was already doing so; I give up");
            component.destroy();
            return;
          } else {
            isLoadingAsync = true;
            component.statusChanged.connect(handleStatusChanged);
          }
          break;
        case Component.Error:
          console.warn("Error loading component: %1".arg(component.errorString()));
          component.destroy();
          break;
        };
      };

      // Check the current status, and create if it's ready, wait for it if not.
      handleStatusChanged();
    }

    initialItem: { source: EmacsBridge.defaultPage }
  }
}
