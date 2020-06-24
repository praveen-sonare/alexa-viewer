/*
 * Copyright © 2020 Collabora Ltd.
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

#include <QGuiApplication>
#include <QDebug>
#include "shell-desktop.h"
#include <qpa/qplatformnativeinterface.h>
#include <stdio.h>

static struct wl_output *
getWlOutput(QScreen *screen)
{
        QPlatformNativeInterface *native = qApp->platformNativeInterface();

        void *output = native->nativeResourceForScreen("output", screen);
        return static_cast<struct ::wl_output*>(output);
}

static void
flush_connection(void)
{
	QPlatformNativeInterface *native = qApp->platformNativeInterface();
	struct wl_display *wl = static_cast<struct wl_display *>(native->nativeResourceForIntegration("display"));

	wl_display_roundtrip(wl);
}

void Shell::activate_app(QWindow *win, const QString &app_id, const QString &app_data)
{
	QScreen *screen = nullptr;
	struct wl_output *output;

	if (!win || !win->screen()) {
		screen = qApp->screens().first();
	} else {
		screen = win->screen();
	}

	if (!screen) {
		return;
	}

	output = getWlOutput(screen);
	agl_shell_desktop_activate_app(this->shell.get(),
				       app_id.toStdString().c_str(),
				       app_data.toStdString().c_str(), output);

	flush_connection();
}

void Shell::deactivate_app(const QString &app_id)
{
	agl_shell_desktop_deactivate_app(this->shell.get(), 
					 app_id.toStdString().c_str());
	flush_connection();
}

void Shell::set_window_props(QWindow *win, const QString &app_id,
			     uint32_t props, int x, int y, int bx, int by,
			     int bwidth, int bheight)
{
	QScreen *screen = nullptr;
	struct wl_output *output;

	if (!win || !win->screen()) {
		screen = qApp->screens().first();
	} else {
		screen = win->screen();
	}

	if (!screen) {
		return;
	}

	output = getWlOutput(screen);
	agl_shell_desktop_set_app_property(this->shell.get(),
					   app_id.toStdString().c_str(),
					   props, x, y, bx, by, bwidth, bheight, output);
	flush_connection();
}
