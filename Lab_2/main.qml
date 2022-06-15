import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.1
import SortFilterProxyModel 0.2
import QtQuick.Dialogs 1.3

ApplicationWindow {
    id: window
    width: 600
    height: 450
    visible: true


    maximumHeight: 450
    maximumWidth: 600
    minimumHeight: 450
    minimumWidth: 600

    ListModel {
        id: accountModel
    }

    Connections {
        target: Backend
        onSend_sites_array: { // передача списка сайтов
            for (var i in count)
                accountModel.append({"site":count[i]})
        }
    }

    Page {
        id: window_1
        width: 600
        height: 450
        visible: true

        Rectangle {
            color: "#303030"
            width: 600
            height: 450

            TextField {
                id: text_key
                width: 220
                height: 50

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 230
                echoMode: TextInput.Password

                placeholderText: qsTr("Введите ключ")
                font.pixelSize: 17
            }

            Button{
                id: sign_in
                width: 220
                height: 50

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 150

                text: "Вход"
                font.pixelSize: 20

                onClicked:
                    if (Backend.get_pass(text_key.text) == 0) {
                        window_error.open();
                        text_key.clear();
                    }
                    else {
                        window_2.visible = true;
                        window_1.visible = false
                        Backend.send_sites()
                        text_key.clear()
                    }
            }

            Popup {
                id: window_error
                width: 300
                height: 150

                background: Rectangle {
                    anchors.fill: window_error
                    color: "#F0E68C"
                }

                x: Math.round((parent.width - width) / 2)
                y: Math.round((parent.height - height) / 2)

                Label {
                    text: qsTr("Неправильный ключ!")
                    color: "#FF0000"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 24
                }
            }
        }
    }

    Page{
        id: window_2
        width: 600
        height: 450
        visible: false

        SortFilterProxyModel { //фильтрация
            id: proxyModel
            sourceModel: accountModel
            filters: RegExpFilter {
                roleName: "site"
                pattern: search.text
            }
        }
        Rectangle {
            color: "#303030"
            width: 600
            height: 450

            ListView {
                anchors.fill: parent
                topMargin: 60
                model: proxyModel
                spacing: 30

                Rectangle {
                    y: 10
                    x: 10
                    width: 360
                    height: 40
                    TextField {
                        id: search
                        width: 360
                        height: 40
                        placeholderText: qsTr("Поиск...")
                        font.pixelSize: 17
                        onFocusChanged: {
                            search.focus = true
                        }

                    }

                    Button{
                        id: button_add_new
                        x: 370
                        height: 40
                        width: 100
                        anchors.leftMargin: 10
                        text: "Добавить"
                        font.pixelSize: 16
                        onClicked: {
                            window_add.visible = true
                        }
                    }

                    Button{
                        id: button_to_print
                        height: 40
                        x: 480
                        width: 100
                        anchors.leftMargin: 90
                        text: "Печать"
                        font.pixelSize: 16
                        onClicked: {
                            Backend.print_all_data()
                        }
                    }
                }

                delegate: Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 60

                    Image {
                        width: 40
                        height: 40
                        fillMode: Image.PreserveAspectFit
                        source: "file:///D:/Notebook/Labs_Kavallini_181_331/Lab_2/Lab_2/key.png"
                    }

                    Text {
                        color: "#00BFFF"
                        text: site
                        width: 80
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 16
                    }
                    TextField { // поле логина
                        text: "00000000"
                        echoMode: TextInput.Password
                        width: 140
                        height: 40
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 16

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                ToolTip.show("Скопировано в буфер обмена", 800)
                                Backend.get_login_pass(0, site)
                            }
                        }

                    }
                    TextField { // поле пароля
                        text: "00000000"
                        echoMode: TextInput.Password
                        width: 140
                        height: 40
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 16

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                ToolTip.show("Скопировано в буфер обмена", 800)
                                Backend.get_login_pass(1, site)
                            }
                        }
                    }
                }
            }
        }
    }

    Page {
        id: window_add
        width: 600
        height: 450
        visible: false
        Rectangle {
            color: "#303030"
            width: 600
            height: 450

            TextField {
                id: text_site
                width: 240
                height: 50

                placeholderText: qsTr("Введите сайт")
                font.pixelSize: 17

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 310
            }

            TextField {
                id: text_login
                width: 240
                height: 50

                placeholderText: qsTr("Введите логин")
                font.pixelSize: 17
                echoMode: TextInput.Password

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 250
            }

            TextField {
                id: text_pass
                width: 240
                height: 50

                placeholderText: qsTr("Введите пароль")
                font.pixelSize: 17
                echoMode: TextInput.Password
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 190
            }

            Button{
                id: button_new
                width: 240

                text: "Добавить аккаунт"
                font.pixelSize: 20
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 100

                enabled: if (text_site.text =="") false
                         else true
                onClicked:{
                    accountModel.clear()
                    Backend.get_new_account(text_site.text, text_login.text, text_pass.text);
                    window_add.visible = false
                    text_site.text =""
                    text_login.text =""
                    text_pass.text = ""
                }

            }

            Button {
                height: 40
                width: 100
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Назад"
                anchors.bottomMargin: 35
                font.pixelSize: 14
                onClicked: {
                    window_add.visible = false
                    text_site.text =""
                    text_login.text =""
                    text_pass.text = ""
                }
            }
        }
    }
}

