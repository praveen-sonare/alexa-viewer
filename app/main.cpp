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

#include <QGuiApplication>
#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/qqml.h>
#include <QQuickWindow>
#include <QtQuickControls2/QQuickStyle>
#include <qpa/qplatformnativeinterface.h>

#include <json-c/json.h>
#include <qlibhomescreen.h>
#include <guimetadata.h>
#include "afbclient.h"
#include "shell-desktop.h"
#include <iostream>

QString my_app_id = QString("alexa-viewer");

// this and the agl-shell extension should be added in some kind of a wrapper
// for easy usage
static void
global_add(void *data, struct wl_registry *reg, uint32_t name,
	   const char *interface, uint32_t version)
{
	struct agl_shell_desktop **shell =
		static_cast<struct agl_shell_desktop **>(data);

	if (strcmp(interface, agl_shell_desktop_interface.name) == 0) {
		*shell = static_cast<struct agl_shell_desktop *>(
				wl_registry_bind(reg, name, &agl_shell_desktop_interface, version)
				);
	}
}

static void
global_remove(void *data, struct wl_registry *reg, uint32_t id)
{
	(void) data;
	(void) reg;
	(void) id;
}

static const struct wl_registry_listener registry_listener = {
	global_add,
	global_remove,
};

static void
application_id_event(void *data, struct agl_shell_desktop *agl_shell_desktop,
		     const char *app_id)
{
	(void) data;
	(void) agl_shell_desktop;
	// un-used
	qInfo() << "app_id: " << app_id;
}

static void
application_id_state(void *data, struct agl_shell_desktop *agl_shell_desktop,
		const char *app_id, const char *app_data, uint32_t app_state, uint32_t app_role)
{
	(void) data;
	(void) app_data;
	(void) app_role;
	(void) app_state;
	(void) app_id;
	(void) agl_shell_desktop;

	// un-used
}

static const struct agl_shell_desktop_listener agl_shell_desk_listener = {
	application_id_event,
	application_id_state,
};

static struct agl_shell_desktop *
register_agl_shell_desktop(void)
{
        struct wl_display *wl;
        struct wl_registry *registry;
        struct agl_shell_desktop *shell = nullptr;

        QPlatformNativeInterface *native = qApp->platformNativeInterface();
        wl = static_cast<struct wl_display *>(native->nativeResourceForIntegration("display"));
        registry = wl_display_get_registry(wl);

        wl_registry_add_listener(registry, &registry_listener, &shell);
        // Roundtrip to get all globals advertised by the compositor
        wl_display_roundtrip(wl);
        wl_registry_destroy(registry);

        return shell;
}

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
	Shell *aglShell;
	if(!data)
		return;

	if (!closure)
		return;

	aglShell = static_cast<Shell *>(closure);

	qDebug() << "got async_event_cb()";
 	if(!strcmp(event, "vshl-capabilities/setDestination")) {
		// Slight hack here, there's currently no convenient place to hook up raising
		// the navigation app when a route is set by Alexa, so do so here for now.
		aglShell->activate_app(nullptr, "navigation", nullptr);
	} else if(!strcmp(event, "vshl-capabilities/render_template")) {
		// Raise ourselves, the UI code will receive the event as well and render it
		if(!check_template_supported(data)) {
			qDebug() << "Unsupported template type, ignoring!";
			return;
		}
		aglShell->activate_app(nullptr, my_app_id, nullptr);
	} else if(!strcmp(event, "vshl-capabilities/clear_template")) {
		// Hide ourselves
		aglShell->deactivate_app(my_app_id);
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
	QGuiApplication app(argc, argv);
	app.setDesktopFileName(my_app_id);

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

	struct agl_shell_desktop *shell = register_agl_shell_desktop();
	if (!shell) {
		qDebug() << "agl_shell_desktop extension missing";
		exit(EXIT_FAILURE);
	}

	agl_shell_desktop_add_listener(shell, &agl_shell_desk_listener, NULL);

	std::shared_ptr<struct agl_shell_desktop> agl_shell{shell, agl_shell_desktop_destroy};
	Shell *aglShell = new Shell(agl_shell, &app);

	// before loading the QML we can tell the compositor that we'd like to
	// be a pop-up kind of window: we need to do this before creating the
	// window itself (either engine load or any of the comp.create()), or
	// we can use/designate another application to behave like that
	//
	// note that x and y initial positioning values have to be specified
	// here (the last two args)
	aglShell->set_window_props(nullptr, my_app_id,
				   AGL_SHELL_DESKTOP_APP_ROLE_POPUP, 0, 0);

	// Load qml
	QQmlApplicationEngine engine;
	QQmlContext *context = engine.rootContext();
	context->setContextProperty("homescreen", aglShell);
	context->setContextProperty("GuiMetadata", new GuiMetadata(bindingAddress, context));
	engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

	// Create app framework client to handle events when window is not visible
	AfbClient client(port, token.toStdString());
	client.set_event_callback(async_event_cb, static_cast<void *>(aglShell));
	subscribe_async_events(client);

	return app.exec();
}
