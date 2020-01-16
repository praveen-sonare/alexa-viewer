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
    id: weather_main
    width: 1000
    height: 1000
    radius: 2

    color: "black"

    ColumnLayout {
        id: weather_col
        anchors.fill: parent.fill

        Label {
            id: weather_title
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.topMargin: 20
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: weatherTemplate.title
            color: "white"
            font.pixelSize: 48
            font.bold: true
            maximumLineCount: 1
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            horizontalAlignment: Label.AlignLeft
            verticalAlignment: Label.AlignVCenter
        }

        Label {
            id: weather_subtitle
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.topMargin: 0
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: weatherTemplate.subtitle
            visible: weatherTemplate.subtitle != ""
            color: "white"
            font.pixelSize: 32
            font.bold: false
            maximumLineCount: 1
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            horizontalAlignment: Label.AlignLeft
            verticalAlignment: Label.AlignVCenter
        }

        RowLayout {
            id: weather_row
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.topMargin: 200
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            spacing: 0

            Image {
                id: weather_currentWeatherIcon_imageContent
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 3
                Layout.preferredWidth: (weather_main.width - 3 * parent.spacing) / 3
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                source: weatherTemplate.currentWeatherIconSource
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }

            Text {
                id: weather_currentWeather_textContent
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 3
                Layout.preferredWidth: (weather_main.width - 3 * parent.spacing) / 3
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                text: weatherTemplate.currentTemperature
                color: "white"
                font.pixelSize: 150
                font.bold: false
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 1
            }

            ColumnLayout {
                id: weather_low_high_col
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 3
                Layout.preferredWidth: (weather_main.width - 3 * parent.spacing) / 3

                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                RowLayout {
                    id: weather_high_row
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    spacing: 20

                    Image {
                        id: weather_highTempArrow_imageContent
                        Layout.fillWidth: false
                        Layout.fillHeight: false
                        Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 6
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                        source: weatherTemplate.highTemperatureArrowSource
                        fillMode: Image.PreserveAspectFit
                        horizontalAlignment: Image.AlignHCenter
                        verticalAlignment: Image.AlignVCenter
                    }

                    Text {
                        id: weather_highTemp_textContent
                        Layout.fillWidth: false
                        Layout.fillHeight: false
                        Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 6
                        Layout.preferredWidth: (weather_main.width - 3 * parent.spacing) / 6
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                        text: weatherTemplate.highTemperature
                        color: "white"
                        font.pixelSize: 60
                        font.bold: false
                        wrapMode: Text.Wrap
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        maximumLineCount: 1
                    }
                }

                RowLayout {
                    id: weather_low_row
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    spacing: 20

                    Image {
                        id: weather_lowTempArrow_imageContent
                        Layout.fillWidth: false
                        Layout.fillHeight: false
                        Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 6
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                        source: weatherTemplate.lowTemperatureArrowSource
                        fillMode: Image.PreserveAspectFit
                        horizontalAlignment: Image.AlignHCenter
                        verticalAlignment: Image.AlignVCenter
                    }

                    Text {
                        id: weather_lowTemp_textContent
                        Layout.fillWidth: false
                        Layout.fillHeight: false
                        Layout.maximumWidth: (weather_main.width - 3 * parent.spacing) / 6
                        Layout.preferredWidth: (weather_main.width - 3 * parent.spacing) / 6
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                        text: weatherTemplate.lowTemperature
                        color: "white"
                        font.pixelSize: 60
                        font.bold: false
                        wrapMode: Text.Wrap
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        maximumLineCount: 1
                    }
                }
            }
        }
    }

    Button {
        id: weather_close
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20

        text: "Close"

        onReleased: {
            weather_close.highlighted = false
            clear()
            hide()
        }
        onPressed: {
            weather_close.highlighted = true
        }
        onCanceled: {
            weather_close.highlighted = false
        }
    }

    // Functions

    function clear() {
        weatherTemplate.visible = false

        weatherTemplate.title = ""
        weatherTemplate.subtitle = ""
        weatherTemplate.currentTemperature = ""
        weatherTemplate.currentWeatherIconSource = ""
        weatherTemplate.lowTemperature = ""
        weatherTemplate.lowTemperatureArrowSource = ""
        weatherTemplate.highTemperature = ""
        weatherTemplate.highTemperatureArrowSource = ""
    }
}
