import QtQuick
import QtQuick.Controls

Item {
    id: toastRoot
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 60
    width: toastRow.implicitWidth + 36 // Tăng nhẹ khoảng cách lề hai bên cho thoáng
    height: 40
    z: 9999
    opacity: 0
    visible: opacity > 0

    property alias text: label.text
    property int duration: 3000

    function show(message) {
        text = message
        anim.restart()
    }

    Rectangle {
        anchors.fill: parent
        color: "#0F172A" // Màu xanh biển đêm siêu tối (Slate 900) làm nền rất sang
        border.color: "#1E3A8A" // Viền xanh Royal mỏng tinh tế để định hình khối
        border.width: 1
        radius: height / 2 // Bo tròn dạng viên thuốc hoàn hảo

        Row {
            id: toastRow
            anchors.centerIn: parent
            spacing: 10 // Tăng nhẹ khoảng cách giữa chấm và chữ

            // Dấu chấm trạng thái màu xanh ngọc sáng (Cyan) tạo điểm nhấn phát sáng
            Rectangle {
                width: 6
                height: 6
                radius: 3
                color: "#38BDF8" // Xanh Sky Blue sáng
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: label
                color: "#F0F9FF" // Màu chữ trắng pha xanh cực nhạt, dịu mắt hơn trắng tinh (#FFF)
                font.pixelSize: 13
                font.bold: true
                font.family: "Segoe UI", "SF Pro Text", "Helvetica"
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    // Giữ nguyên cơ chế hiệu ứng mượt mà cũ của bạn
    SequentialAnimation {
        id: anim

        NumberAnimation {
            target: toastRoot
            property: "opacity"
            to: 1.0
            duration: 150
            easing.type: Easing.OutQuad
        }

        PauseAnimation {
            duration: toastRoot.duration
        }

        NumberAnimation {
            target: toastRoot
            property: "opacity"
            to: 0.0
            duration: 250
            easing.type: Easing.InQuad
        }
    }
}