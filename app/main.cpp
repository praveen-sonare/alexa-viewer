/*
 * Copyright (C) 2016 The Qt Company Ltd.
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

#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/qqml.h>
#include <QQuickWindow>
#include <QtQuickControls2/QQuickStyle>

#include <json-c/json.h>
#include <qlibwindowmanager.h>
#include <qlibhomescreen.h>
#include <guimetadata.h>
#include "afbclient.h"
#include <iostream>

// Disable window activation at launch by default, but keep option
// for potential debug use.
#define HIDE_AT_LAUNCH

bool check_template_supported(json_object *data)
{
	json_object *jtype = NULL;
	json_object_object_get_ex(data, "type", &jtype);
	if(!jtype) {
		qWarning("render_template event missing type element");
		return false;
	}
	const char *type_value = json_object_get_string(jtype);
	if(!type_value) {
		qWarning("render_template event type element not parsed");
		return false;
	}
	// We only handle BodyTemplate[12] and WeatherTemplate, ignore
	// others
	if(!(strcmp(type_value, "BodyTemplate1") &&
	     strcmp(type_value, "BodyTemplate2") &&
	     strcmp(type_value, "WeatherTemplate")))
		return true;

	return false;
}

void async_event_cb(const char *event, json_object *data, void *closure)
{
	if(!data)
		return;

 	if(!strcmp(event, "vshl-capabilities/setDestination")) {
		// Slight hack here, there's currently no convenient place to hook up raising
		// the navigation app when a route is set by Alexa, so do so here for now.
		if(closure != nullptr) {
			static_cast<QLibHomeScreen*>(closure)->showWindow("navigation", "normal");
		}
	} else if(!strcmp(event, "vshl-capabilities/render_template")) {
		// Raise ourselves, the UI code will receive the event as well and render it
		if(closure != nullptr) {
			if(!check_template_supported(data)) {
				qDebug() << "Unsupported template type, ignoring!";
				return;
			}
			static_cast<QLibHomeScreen*>(closure)->showWindow("alexa-viewer", "on_screen");
		}
	} else if(!strcmp(event, "vshl-capabilities/clear_template")) {
		// Hide ourselves
		if(closure != nullptr) {
			static_cast<QLibHomeScreen*>(closure)->hideWindow("alexa-viewer");
		}
	}
}

void subscribe_async_events(AfbClient &client)
{
	const char *vshl_capabilities_nav_events[] = {
		"setDestination",
		NULL,
	};
	const char **tmp = vshl_capabilities_nav_events;
	json_object *args = json_object_new_object();
	json_object *actions = json_object_new_array();
	while (*tmp) {
		json_object_array_add(actions, json_object_new_string(*tmp++));
	}
	json_object_object_add(args, "actions", actions);
	if(json_object_array_length(actions)) {
		client.subscribe("vshl-capabilities", args, "navigation/subscribe");
	} else {
		json_object_put(args);
	}

	// NOTE: Not subscribing to "clear_template", as it will be passed to
	//       the app QML to handle by the libqtappfw wrapper.
	const char *vshl_capabilities_guimetadata_events[] = {
		"render_template",
		NULL,
	};
	tmp = vshl_capabilities_guimetadata_events;
	args = json_object_new_object();
	actions = json_object_new_array();
	while (*tmp) {
		json_object_array_add(actions, json_object_new_string(*tmp++));
	}
	json_object_object_add(args, "actions", actions);
	if(json_object_array_length(actions)) {
		client.subscribe("vshl-capabilities", args, "guimetadata/subscribe");
	} else {
		json_object_put(args);
	}
}

int main(int argc, char *argv[])
{
	QString graphic_role = QString("on_screen");

	QGuiApplication app(argc, argv);

	QCommandLineParser parser;
	parser.addPositionalArgument("port", app.translate("main", "port for binding"));
	parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);
	QStringList positionalArguments = parser.positionalArguments();
	QUrl bindingAddress;

	int port = 0;
	QString token;
	if (positionalArguments.length() == 2) {
		port = positionalArguments.takeFirst().toInt();
		token = positionalArguments.takeFirst();
		bindingAddress.setScheme(QStringLiteral("ws"));
		bindingAddress.setHost(QStringLiteral("localhost"));
		bindingAddress.setPort(port);
		bindingAddress.setPath(QStringLiteral("/api"));
		QUrlQuery query;
		query.addQueryItem(QStringLiteral("token"), token);
		bindingAddress.setQuery(query);
	}

	// QLibWM
	QLibWindowmanager* qwmHandler = new QLibWindowmanager();
	int res;
	if((res = qwmHandler->init(port, token)) != 0){
		qCritical("init qlibwm err(%d)", res);
		return -1;
	}
	if((res = qwmHandler->requestSurface(graphic_role)) != 0) {
		qCritical("requestSurface error(%d)", res);
		return -1;
	}
	qwmHandler->set_event_handler(QLibWindowmanager::Event_SyncDraw,
				      [qwmHandler, &graphic_role](json_object *object) {
					      qwmHandler->endDraw(graphic_role);
				      });

	// QLibHS
	QLibHomeScreen* qhsHandler = new QLibHomeScreen();
	qhsHandler->init(port, token.toStdString().c_str());
	qhsHandler->set_event_handler(QLibHomeScreen::Event_ShowWindow,
				      [qwmHandler, &graphic_role](json_object *object){
					      qDebug("Surface %s got showWindow\n", graphic_role.toStdString().c_str());
					      qwmHandler->activateWindow(graphic_role, "on_screen");
				      });
	qhsHandler->set_event_handler(QLibHomeScreen::Event_HideWindow,
				      [qwmHandler, &graphic_role](json_object *object){
					      qDebug("Surface %s got hideWindow\n", graphic_role.toStdString().c_str());
					      qwmHandler->deactivateWindow(graphic_role);
				      });

	// Load qml
	QQmlApplicationEngine engine;
	QQmlContext *context = engine.rootContext();

	context->setContextProperty("homescreen", qhsHandler);
	context->setContextProperty("GuiMetadata", new GuiMetadata(bindingAddress, context));
	engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

#ifndef HIDE_AT_LAUNCH
	QObject *root = engine.rootObjects().first();
	QQuickWindow *window = qobject_cast<QQuickWindow *>(root);
	QObject::connect(window, SIGNAL(frameSwapped()), qwmHandler, SLOT(slotActivateWindow()));
#endif
	// Create app framework client to handle events when window is not visible
	AfbClient client(port, token.toStdString());
	client.set_event_callback(async_event_cb, (void*) qhsHandler);
	subscribe_async_events(client);

	return app.exec();
}
