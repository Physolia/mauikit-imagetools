import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts 

import org.mauikit.controls as Maui

import org.kde.kquickimageeditor as KQuickImageEditor

ColumnLayout
{
    id: control

    spacing: 0

    property alias rotationSlider: _freeRotationSlider
    property alias rotationButton : _freeRotationButton
    property alias cropButton : _cropButton

    Maui.ToolBar
    {
        position: ToolBar.Footer
        Layout.fillWidth: true
        visible: _freeRotationButton.checked
        background: Rectangle
        {
            color: Maui.Theme.backgroundColor
        }
        leftContent: [
            ToolButton
            {
                icon.name: "object-flip-vertical"
                text: i18nc("@action:button Mirror an image vertically", "Flip");
                autoExclusive: true
                onClicked: imageDoc.mirror(false, true);
            },

            ToolButton
            {
                icon.name: "object-flip-horizontal"
                text: i18nc("@action:button Mirror an image horizontally", "Mirror");
                checkable: true
                autoExclusive: true
                onClicked: imageDoc.mirror(true, false);
            }

        ]

        rightContent: ToolButton
        {
            icon.name: "object-rotate-left"
            //                    display: ToolButton.IconOnly
            text: i18nc("@action:button Rotate an image 90°", "Rotate 90°");
            onClicked:
            {
                let value = _freeRotationSlider.value-90
                _freeRotationSlider.value = value < -180 ? 90 : value
            }
        }

        //                middleContent: Label
        //                {
        //                    text: i18nd("mauikitimagetools","Rotate")
        //                }
    }

    Maui.ToolBar
    {
        id: _freeRotation

        visible: _freeRotationButton.checked
        position: ToolBar.Footer
        background: Rectangle
        {
            color: Maui.Theme.backgroundColor
        }

        Layout.fillWidth: true


        middleContent: Ruler
        {
            id: _freeRotationSlider
            Layout.fillWidth: true
            from : -180
            to: 180
            value: 0
            snapMode: Slider.SnapAlways
            stepSize: 1
        }
    }

    Maui.ToolBar
    {
        position: ToolBar.Footer
        Layout.fillWidth: true
        background: Rectangle
        {
            color: Maui.Theme.backgroundColor
        }
        middleContent: Maui.ToolActions
        {
            autoExclusive: true
            Layout.alignment: Qt.AlignHCenter
            Action
            {
                id: _cropButton
                checkable: true
                icon.name:  "transform-crop"
                text:  i18nc("@action:button Crop an image", "Crop");
            }

            Action
            {
                id: _freeRotationButton
                icon.name: "transform-rotate"
                checkable: true
                text: i18nc("@action:button Rotate an image", "Rotate");
            }
        }

        leftContent: ToolButton
        {
            //                    text: i18nd("mauikitimagetools","Accept")
            visible: _freeRotationButton.checked || _cropButton.checked

            icon.name: "checkmark"
            onClicked:
            {
                if(_freeRotationButton.checked)
                {
                    var value = _freeRotationSlider.value
                    _freeRotationSlider.value = 0

                    console.log("Rotate >> " , value)
                    imageDoc.rotate(value);
                }

                if(_cropButton.checked)
                {
                    crop()
                }
            }
        }

        rightContent:  ToolButton
        {
            //                    text: i18nd("mauikitimagetools","Cancel")
            visible: _freeRotationButton.checked || _cropButton.checked
            icon.name: "dialog-cancel"
            onClicked:
            {
                if(_freeRotationButton.checked)
                {
                    _freeRotationSlider.value = 0
                    _freeRotationButton.checked = false

                }

                if(_cropButton.checked)
                {
                    _cropButton.checked = false
                }
            }
        }
    }
}

