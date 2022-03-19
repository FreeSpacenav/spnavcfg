#ifndef UI_H_
#define UI_H_

#include "ui_mainwin.h"

class MainWin : public QWidget {
	Q_OBJECT

private:
	Ui::win_main *ui;

public:
	explicit MainWin(QWidget *par = 0);
	~MainWin();
};

#endif	/* UI_H_ */
