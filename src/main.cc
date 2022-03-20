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
