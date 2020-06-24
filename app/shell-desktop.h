/*
 * Copyright © 2019 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SHELLDESKTOP_H
#define SHELLDESKTOP_H

#include <QObject>
#include <QString>
#include <QScreen>
#include <QWindow>
#include <memory>
#include "agl-shell-desktop-client-protocol.h"

/*
 * Basic type to wrap the agl_shell wayland object into a QObject, so that it
 * can be used in callbacks from QML.
 */

class Shell : public QObject
{
Q_OBJECT
	std::shared_ptr<struct agl_shell_desktop> shell;

public:
	Shell(std::shared_ptr<struct agl_shell_desktop> shell, QObject *parent = nullptr) :
		QObject(parent), shell(shell) 
	{}

signals:
	// using Qobject::connect(), callins into qml
	void showOnScreen();

public slots: // calls out of qml into CPP
	void activate_app(QWindow *win, const QString &app_id, const QString &app_data);
	void deactivate_app(const QString &app_id);
	void set_window_props(QWindow *win, const QString &app_id,
			uint32_t props, int x, int y, int bx, int by,
			int bwidth, int bheight);
};

#endif // SHELLDESKTOP_H
