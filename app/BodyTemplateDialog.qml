/*
 * Copyright (C) 2020 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0

Rectangle {
    id: body_main
    width: 1000
    height: 1000
    radius: 2

    color: "black"

    ColumnLayout {
        id: body_col
        anchors.fill: parent.fill

        Label {
            id: body_title
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.topMargin: 20
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: bodyTemplate.title
            color: "white"
            font.pixelSize: 32
            font.bold: true
            maximumLineCount: 1
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            horizontalAlignment: Label.AlignLeft
            verticalAlignment: Label.AlignVCenter
        }

        Label {
            id: body_subtitle
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.topMargin: 0
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: bodyTemplate.subtitle
            visible: bodyTemplate.subtitle != ""
            color: "white"
            font.pixelSize: 22
            font.bold: false
            maximumLineCount: 1
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            horizontalAlignment: Label.AlignLeft
            verticalAlignment: Label.AlignVCenter
        }

        RowLayout {
            id: body_row
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 20
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            spacing: 20

            Text {
                id: body_textContent
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: bodyTemplate.textContent
                color: "white"
                font.pixelSize: 32
                font.bold: false
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignTop
                maximumLineCount: 21

                states: [
                    State {
                        name: "BodyTemplate2"
                        when: bodyTemplate.imageContentSource != ""
                        PropertyChanges {
                            target: body_textContent
                            Layout.maximumWidth: (body_main.width - 3 * parent.spacing) / 2
                            Layout.preferredWidth: (body_main.width - 3 * parent.spacing) / 2
                        }
                    },
                    State {
                        name: "BodyTemplate1"
                        when: bodyTemplate.imageContentSource == ""
                        PropertyChanges {
                            target: body_textContent
                            Layout.maximumWidth: body_main.width - 2 * parent.spacing
                            Layout.preferredWidth: body_main.width - 2 * parent.spacing
                        }
                    }
                ]
             }

            Image {
                id: body_imageContent
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.maximumWidth: (body_main.width - 3 * parent.spacing) / 2
                Layout.preferredWidth: (body_main.width - 3 * parent.spacing) / 2
                Layout.alignment: Qt.AlignTop

                source: bodyTemplate.imageContentSource
                visible: bodyTemplate.imageContentSource != ""
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignTop
            }
        }
    }

    Button {
        id: body_close
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20

        text: "Close"

        onReleased: {
            body_close.highlighted = false
            clear()
            hide()
        }
        onPressed: {
            body_close.highlighted = true
        }
        onCanceled: {
            body_close.highlighted = false
        }
    }

    // Functions

    function clear() {
        bodyTemplate.visible = false

        bodyTemplate.title = ""
        bodyTemplate.subtitle = ""
        bodyTemplate.textContent = ""
        bodyTemplate.imageContentSource = ""
    }
}
