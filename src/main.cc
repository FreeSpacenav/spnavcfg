#include <QApplication>
#include "ui.h"

QWidget *mainwin;

int main(int argc, char **argv)
{
	QCoreApplication::setApplicationName("spnavcfg");

	QApplication app(argc, argv);
	MainWin w;
	w.show();
	mainwin = &w;

	return app.exec();
}
