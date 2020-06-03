import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.14
import Qt.labs.settings 1.0
import fi.aardsoft.emacsbridge 1.0

ApplicationWindow {
  visible: true
  title: qsTr("Emacs Bridge")
  id: window

  Material.theme: Material.Dark
  Material.accent: Material.purple

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
      anchors.fill: parent
      ToolButton {
        text: "..."
        action: navigateBackAction
      }
      Label {
        text: "Emacs Bridge"
        elide: Label.ElideRight
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        Layout.fillWidth: true
      }
      ToolButton {
        text: qsTr("⋮")
        onClicked: optionsMenu.open()

        Menu {
          id: optionsMenu
          x: parent.width - width
          transformOrigin: Menu.TopRight
          Action {
            text: "Settings"
            onTriggered: settingsDialog.open()
          }
          Action {
            text: "About"
            onTriggered: aboutDialog.open()
          }
        }
      }
    }
  }

  Drawer {
    id: drawer
    height: window.height
    interactive: stackView.depth === 1
    width: Math.min(window.width, window.height) / 3 * 2

    ListView {
      id: listView
      focus: true
      currentIndex: -1
      anchors.fill: parent

      delegate: ItemDelegate {
        width: parent.width
        text: model.title
        highlighted: ListView.isCurrentItem
        onClicked: {
          listView.currentIndex = index
          stackView.push(model.source)
          drawer.close()
        }
      }

      model: ListModel {
        ListElement { title: "Lisp eval"; source: "qrc:/qml/LispEvalPage.qml" }
        ListElement { title: "Test notifications"; source: "qrc:/qml/NotificationTestPage.qml" }
      }

      ScrollIndicator.vertical: ScrollIndicator { }
    }
  }

  StackView {
    id: stackView
    anchors.fill: parent

    initialItem: { source: "qrc:/qml/StatusPage.qml" }
  }

  Dialog {
    id: settingsDialog
    width: window.width
    height: window.height - header.height
    standardButtons: Dialog.OK | Dialog.Cancel

    ScrollView {
      Layout.fillHeight: true
      Layout.fillWidth: true
      implicitHeight: 30
      clip: true
      GridLayout {
        anchors.fill: parent
        columns: 2

        Label { text: "Agent secret: "; }
        TextField { id: settingsSecret; text: settings.secret; Layout.fillWidth: true; Layout.fillHeight: true}
      }
    }
  }

  Dialog {
    id: aboutDialog
    modal: true
    focus: true
    title: "About"
    width: window.width
    height: window.height - header.height

    Column {
      id: aboutColumn
      spacing: 20

      Label {
        width: aboutDialog.availableWidth
        text: emacsBridge.mobile
        wrapMode: Label.Wrap
      }

      Label {
        visible: emacsBridge.mobile
        width: aboutDialog.availableWidth
        text: "In comparison to the desktop-oriented Qt Quick Controls 1, Qt Quick Controls 2 "
        + "are an order of magnitude simpler, lighter and faster, and are primarily targeted "
        + "towards embedded and mobile platforms."
        wrapMode: Label.Wrap
      }
    }
  }
}
