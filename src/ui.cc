#include <stdio.h>
#include "ui.h"

MainWin::MainWin(QWidget *par)
	: QWidget(par)
{
	ui = new Ui::win_main;
	ui->setupUi(this);
}

MainWin::~MainWin()
{
	delete ui;
}
