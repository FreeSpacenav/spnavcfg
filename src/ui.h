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
#ifndef UI_H_
#define UI_H_

#ifdef __cplusplus

#include <QMainWindow>

namespace Ui {
	class win_main;
}

class MainWin : public QMainWindow {
	Q_OBJECT

private:
	Ui::win_main *ui;

public:
	explicit MainWin(QWidget *par = 0);
	~MainWin();

	bool init();
	void updateui();

public slots:
	void spnav_input();

	void act_trig();
	void slider_changed(int val);
	void dspin_changed(double val);
	void spin_changed(int val);
	void chk_changed(int checked);
	void rad_changed(bool active);
	void combo_idx_changed(int sel);
	void combo_str_changed(const QString &qstr);
	void serpath_changed();
};

extern MainWin *mainwin;

extern "C" {
#endif

void update_ui(void);
void errorbox(const char *msg);
void errorboxf(const char *fmt, ...);
void aboutbox(void);

#ifdef __cplusplus
}
#endif

#endif	/* UI_H_ */
