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

	void updateui();

public slots:
	void spnav_input();

	void act_trig();
	void slider_moved(int val);
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
