/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007-2025 John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QApplication>
#include <QSocketNotifier>
#define SPNAV_CONFIG_H_
#include <spnav.h>
#include "spnavcfg.h"
#include "ui.h"

static bool init();

MainWin *mainwin;
static QSocketNotifier *sockev;

int main(int argc, char **argv)
{
	QCoreApplication::setApplicationName("spnavcfg");

	QApplication app(argc, argv);
	MainWin w;
	w.show();
	mainwin = &w;

	if(!init()) {
		return 1;
	}

	return app.exec();
}

static bool init()
{
	if(!mainwin->init()) {
		return false;
	}

	if(spnav_open() == -1) {
		errorbox("Failed to connect to spacenavd!");
		return false;
	}
	if(spnav_protocol() < 1) {
		errorbox("Currently running version of spacenavd is too old for this version of the configuration tool.\n"
				"\nEither update to a recent version of spacenavd (v0.9 or later), or downgrade to spnavcfg v0.3.1.");
		return false;
	}
	spnav_client_name("spnavcfg");
	spnav_evmask(SPNAV_EVMASK_ALL);

	if(read_devinfo(&devinfo) == -1) {
		errorbox("Failed to read device info.");
		return false;
	}
	if(read_cfg(&cfg) == -1) {
		errorbox("Failed to read current configuration.");
		return false;
	}

	sockev = new QSocketNotifier(spnav_fd(), QSocketNotifier::Read);
	QObject::connect(sockev, &QSocketNotifier::activated, mainwin, &MainWin::spnav_input);

	return true;
}
