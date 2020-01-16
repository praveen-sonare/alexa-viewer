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
import QtQuick.Window 2.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

Window {
    id: root
    width: 1080
    height: 1488
    color: '#00000000'

    visible: true
    flags: Qt.FramelessWindowHint

    BodyTemplateDialog {
        id: bodyTemplate
        anchors.centerIn: parent
        visible: true

        property string title: ""
        property string subtitle: ""

        property string textContent: ""
        property url imageContentSource: ""
    }

    WeatherTemplateDialog {
        id: weatherTemplate
        anchors.centerIn: parent
        visible: false

        property string title: ""
        property string subtitle: ""

        property string currentTemperature: ""
        property url currentWeatherIconSource: ""
        property string lowTemperature: ""
        property url lowTemperatureArrowSource: ""
        property string highTemperature: ""
        property url highTemperatureArrowSource: ""
    }

    Connections {
        target: GuiMetadata

        onRenderTemplate: {
            console.log("Received renderTemplate, type = " + GuiMetadata.type)
            if(GuiMetadata.type == "BodyTemplate1" || GuiMetadata.type == "BodyTemplate2") {
                // Normally setting the target to visible would be after changes to the
                // content, but I was seeing better behavior during testing by doing it
                // here. Further investigation is required, and likely hooking up
                // loading indication on the Images in the template dialogs.
                weatherTemplate.visible = false
                bodyTemplate.visible = true

                bodyTemplate.title = GuiMetadata.title
                bodyTemplate.subtitle = GuiMetadata.subtitle
                bodyTemplate.textContent = GuiMetadata.bodyText

                if(GuiMetadata.type == "BodyTemplate1") {
                    bodyTemplate.imageContentSource = ""
                } else {
                    bodyTemplate.imageContentSource = GuiMetadata.bodyImageSmallUrl
                }
            } else if(GuiMetadata.type == "WeatherTemplate") {
                bodyTemplate.visible = false
                weatherTemplate.visible = true

                weatherTemplate.title = GuiMetadata.title
                weatherTemplate.subtitle = GuiMetadata.subtitle

                weatherTemplate.currentTemperature = GuiMetadata.weatherCurrentTemperature
                weatherTemplate.currentWeatherIconSource = GuiMetadata.weatherCurrentWeatherIconMediumDarkBgUrl
                weatherTemplate.lowTemperature = GuiMetadata.weatherLowTemperature
                weatherTemplate.lowTemperatureArrowSource = GuiMetadata.weatherLowTemperatureArrowMediumDarkBgUrl
                weatherTemplate.highTemperature = GuiMetadata.weatherHighTemperature
                weatherTemplate.highTemperatureArrowSource = GuiMetadata.weatherHighTemperatureArrowMediumDarkBgUrl
            } else {
                // Should not happen, but just in case
                bodyTemplate.title = "Unsupported Template"
                bodyTemplate.subtitle = ""
                bodyTemplate.textContent = "The display template for this response is currently unsupported."
                bodyTemplate.imageContentSource = ""

                weatherTemplate.visible = false
                bodyTemplate.visible = false
            }
        }

        onClearTemplate: {
            console.log("Received clearTemplate!")
            bodyTemplate.clear()
            weatherTemplate.clear()
            hide()
        }
    }

    // Functions

    function hide() {
        console.log("hiding window!")
        homescreen.hideWindow("alexa-viewer")
    }
}
