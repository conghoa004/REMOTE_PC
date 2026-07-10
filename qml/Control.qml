import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: controlWindow
    visible: true
    width: 1100
    height: 800
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    title: "Remote Desktop Connection"
    color: "#0b0c10"

    // Biến kết nối nhận từ Home Window
    property var connection: null

    // ==========================================
    // TẢI FONT AWESOME VÀO HỆ THỐNG QML
    // ==========================================
    FontLoader {
        id: homeIconFont
        source: "../fonts/as7.otf"
    }

    // ==========================================
    // KẾT NỐI SIGNAL frameReceived -> ScreenRenderer
    // ==========================================
    Connections {
        target: connection
        function onFrameReceived(image) {
            remoteScreen.setImage(image)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ==========================================
        // 1. THANH CÔNG CỤ CHUYÊN NGHIỆP (TOP BAR)
        // ==========================================
        Rectangle {
            id: topBar
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#14161d"

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#222531"
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 15

                // --- TRẠNG THÁI KẾT NỐI ---
                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        id: statusDot
                        width: 8
                        height: 8
                        radius: 4
                        color: "#00e676"

                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            PropertyAnimation { to: 0.4; duration: 1000; easing.type: Easing.InOutQuad }
                            PropertyAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
                        }
                    }

                    Text {
                        text: "Connected"
                        color: "#ffffff"
                        font.family: "Segoe UI"
                        font.pixelSize: 13
                        font.bold: true
                    }
                }

                Item { Layout.fillWidth: true }

                // --- NÚT CHUYỂN CHẤT LƯỢNG ---
                Button {
                    id: qualityButton
                    checkable: true
                    checked: connection ? connection.losslessQuality : false
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 36

                    onClicked: {
                        if (connection) {
                            connection.losslessQuality = checked
                        }
                    }

                    contentItem: RowLayout {
                        spacing: 8
                        anchors.centerIn: parent

                        Text {
                            text: qualityButton.checked ? "\uf06e" : "\uf1da"
                            font.family: homeIconFont.name
                            font.pixelSize: 14
                            color: qualityButton.checked ? "#ffffff" : "#a0aec0"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: qualityButton.checked ? "Lossless" : "Optimized"
                            font.family: "Segoe UI"
                            font.pixelSize: 13
                            font.bold: true
                            color: qualityButton.checked ? "#ffffff" : "#a0aec0"
                        }
                    }

                    background: Rectangle {
                        color: qualityButton.checked ? "#6d28d9" : (qualityButton.hovered ? "#222531" : "#1a1d26")
                        radius: 6
                        border.color: qualityButton.checked ? "transparent" : "#313547"
                        border.width: 1
                        scale: qualityButton.pressed ? 0.96 : (qualityButton.hovered ? 1.02 : 1.0)

                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }
                    }
                }

                // NÚT TẮT TIẾNG (MUTE BUTTON)
                Button {
                    id: muteButton
                    checkable: true
                    Layout.preferredWidth: 115
                    Layout.preferredHeight: 36

                    contentItem: RowLayout {
                        spacing: 8
                        anchors.centerIn: parent

                        Text {
                            text: muteButton.checked ? "\uf6a9" : "\uf028"
                            font.family: homeIconFont.name
                            font.pixelSize: 14
                            color: muteButton.checked ? "#ffffff" : "#a0aec0"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: muteButton.checked ? "Unmute" : "Mute"
                            font.family: "Segoe UI"
                            font.pixelSize: 13
                            font.bold: true
                            color: muteButton.checked ? "#ffffff" : "#a0aec0"
                        }
                    }

                    background: Rectangle {
                        color: muteButton.checked ? "#d97706" : (muteButton.hovered ? "#222531" : "#1a1d26")
                        radius: 6
                        border.color: muteButton.checked ? "transparent" : "#313547"
                        border.width: 1
                        scale: muteButton.pressed ? 0.96 : (muteButton.hovered ? 1.02 : 1.0)

                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }
                    }
                }

                // NÚT NGẮT KẾT NỐI (DISCONNECT BUTTON)
                Button {
                    id: disconnectButton
                    Layout.preferredWidth: 130
                    Layout.preferredHeight: 36

                    contentItem: RowLayout {
                        spacing: 8
                        anchors.centerIn: parent

                        Text {
                            text: "\uf1e6"
                            font.family: homeIconFont.name
                            font.pixelSize: 14
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: "Disconnect"
                            font.family: "Segoe UI"
                            font.pixelSize: 13
                            font.bold: true
                            color: "white"
                        }
                    }

                    background: Rectangle {
                        color: disconnectButton.down ? "#b91c1c" : (disconnectButton.hovered ? "#ef4444" : "#dc2626")
                        radius: 6
                        scale: disconnectButton.pressed ? 0.96 : (disconnectButton.hovered ? 1.02 : 1.0)

                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }
                    }

                    onClicked: {
                        if (connection) {
                            connection.disconnectFromHost()
                        }
                        controlWindow.close()
                    }
                }
            }
        }

        // ==========================================
        // 2. VÙNG HIỂN THỊ MÀN HÌNH (REMOTE CANVAS)
        // ==========================================
        Rectangle {
            id: screenArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#08090c"
            focus: true

            // ScreenRenderer hiển thị khung hình từ host
            ScreenRenderer {
                id: remoteScreen
                anchors.fill: parent
                anchors.margins: 4
                visible: true
            }

            // MouseArea bắt sự kiện chuột
            MouseArea {
                id: inputArea
                anchors.fill: remoteScreen
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                cursorShape: Qt.ArrowCursor

                onPositionChanged: function(mouse) {
                    if (!connection) return
                    var p = remoteScreen.mapToSource(Qt.point(mouse.x, mouse.y))
                    connection.sendMouseMove(Math.round(p.x), Math.round(p.y))
                }

                onPressed: function(mouse) {
                    screenArea.forceActiveFocus()
                    if (!connection) return
                    var p = remoteScreen.mapToSource(Qt.point(mouse.x, mouse.y))
                    connection.sendMousePress(mouse.button, Math.round(p.x), Math.round(p.y))
                }

                onReleased: function(mouse) {
                    if (!connection) return
                    var p = remoteScreen.mapToSource(Qt.point(mouse.x, mouse.y))
                    connection.sendMouseRelease(mouse.button, Math.round(p.x), Math.round(p.y))
                }

                onWheel: function(wheel) {
                    if (!connection) return
                    connection.sendMouseWheel(wheel.angleDelta.y)
                }
            }

            // Keyboard handlers
            Keys.onPressed: function(event) {
                if (!connection) return
                connection.sendKeyPress(event.key)
                event.accepted = true
            }

            Keys.onReleased: function(event) {
                if (!connection) return
                connection.sendKeyRelease(event.key)
                event.accepted = true
            }

            // Placeholder khi chưa có khung hình
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12
                visible: !remoteScreen.visible || remoteScreen.width === 0

                Text {
                    text: "\uf108"
                    font.family: homeIconFont.name
                    font.pixelSize: 48
                    color: "#313547"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "WAITING FOR SCREEN DATA..."
                    color: "#4e566d"
                    font.pixelSize: 12
                    font.family: "Segoe UI"
                    font.bold: true
                    font.letterSpacing: 2
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}