#include <stdio.h>
#include <assert.h>
#define SPNAV_CONFIG_H_
#include <spnav.h>
#include "ui.h"
#include "spnavcfg.h"
#include "ui_mainwin.h"
#include "ui_bnmaprow.h"
#include <QMessageBox>

#include <X11/Xlib.h>

static QSlider *slider_sens_axis[6];
static QCheckBox *chk_inv[6];
static QComboBox *combo_axismap[6];
static QDoubleSpinBox *spin_sens_axis[6];
static QSpinBox *spin_dead_axis[6];
static QProgressBar *prog_axis[6];
static QPixmap *dev_atlas;

static Ui::row_bnmap *bnrow;
static QVBoxLayout *vbox_bnui;
static QWidget *bnrow_root;
static int bnrow_count;

static bool mask_events;

static QPalette def_cmb_cmap;


struct device_image {
	int devtype;
	int width, height;
	int xoffs, yoffs;
};
static struct device_image devimglist[] = {
	{SPNAV_DEV_UNKNOWN,		150, 150, 0, 0},
	{SPNAV_DEV_SB2003,		150, 150, 1, 0},
	{SPNAV_DEV_SB3003,		150, 150, 2, 0},
	{SPNAV_DEV_SB4000,		150, 150, 3, 0},
	{SPNAV_DEV_SM,			150, 150, 5, 0},
	{SPNAV_DEV_SM5000,		150, 150, 2, 0},
	{SPNAV_DEV_SMCADMAN,	150, 150, 6, 0},
	{SPNAV_DEV_PLUSXT,		150, 150, 5, 0},
	{SPNAV_DEV_CADMAN,		150, 150, 6, 0},
	{SPNAV_DEV_SMCLASSIC,	150, 150, 4, 0},
	{SPNAV_DEV_SB5000,		150, 150, 3, 0},
	{SPNAV_DEV_STRAVEL,		150, 150, 2, 1},
	{SPNAV_DEV_SPILOT,		150, 150, 3, 1},
	{SPNAV_DEV_SNAV,		150, 150, 0, 1},
	{SPNAV_DEV_SEXP,		150, 150, 4, 1},
	{SPNAV_DEV_SNAVNB,		150, 150, 1, 1},
	{SPNAV_DEV_SPILOTPRO,	150, 150, 5, 1},
	{SPNAV_DEV_SMPRO,		150, 150, 6, 1},
	{SPNAV_DEV_NULOOQ,		150, 150, 7, 0},
	{SPNAV_DEV_SMW,			150, 150, 1, 2},
	{SPNAV_DEV_SMPROW,		150, 150, 6, 1},
	{SPNAV_DEV_SMENT,		150, 150, 7, 1},
	{SPNAV_DEV_SMCOMP,		150, 150, 0, 2},
	{SPNAV_DEV_SMMOD,		150, 150, 0, 0},
	{-1}
};


MainWin::MainWin(QWidget *par)
	: QWidget(par)
{
	mask_events = true;

	ui = new Ui::win_main;
	ui->setupUi(this);

	dev_atlas = new QPixmap(":/icons/devices.png");

	slider_sens_axis[0] = ui->slider_sens_tx;
	slider_sens_axis[1] = ui->slider_sens_ty;
	slider_sens_axis[2] = ui->slider_sens_tz;
	slider_sens_axis[3] = ui->slider_sens_rx;
	slider_sens_axis[4] = ui->slider_sens_ry;
	slider_sens_axis[5] = ui->slider_sens_rz;

	chk_inv[0] = ui->chk_inv_tx;
	chk_inv[1] = ui->chk_inv_ty;
	chk_inv[2] = ui->chk_inv_tz;
	chk_inv[3] = ui->chk_inv_rx;
	chk_inv[4] = ui->chk_inv_ry;
	chk_inv[5] = ui->chk_inv_rz;

	combo_axismap[0] = ui->combo_axismap_tx;
	combo_axismap[1] = ui->combo_axismap_ty;
	combo_axismap[2] = ui->combo_axismap_tz;
	combo_axismap[3] = ui->combo_axismap_rx;
	combo_axismap[4] = ui->combo_axismap_ry;
	combo_axismap[5] = ui->combo_axismap_rz;

	spin_dead_axis[0] = ui->spin_dead_tx;
	spin_dead_axis[1] = ui->spin_dead_ty;
	spin_dead_axis[2] = ui->spin_dead_tz;
	spin_dead_axis[3] = ui->spin_dead_rx;
	spin_dead_axis[4] = ui->spin_dead_ry;
	spin_dead_axis[5] = ui->spin_dead_rz;

	spin_sens_axis[0] = ui->spin_sens_tx;
	spin_sens_axis[1] = ui->spin_sens_ty;
	spin_sens_axis[2] = ui->spin_sens_tz;
	spin_sens_axis[3] = ui->spin_sens_rx;
	spin_sens_axis[4] = ui->spin_sens_ry;
	spin_sens_axis[5] = ui->spin_sens_rz;

	prog_axis[0] = ui->prog_tx;
	prog_axis[1] = ui->prog_ty;
	prog_axis[2] = ui->prog_tz;
	prog_axis[3] = ui->prog_rx;
	prog_axis[4] = ui->prog_ry;
	prog_axis[5] = ui->prog_rz;

	connect(ui->combo_led, SIGNAL(currentIndexChanged(int)), this, SLOT(combo_idx_changed(int)));
	connect(ui->ed_serpath, SIGNAL(editingFinished()), this, SLOT(serpath_changed()));
	connect(ui->chk_serial, SIGNAL(stateChanged(int)), this, SLOT(chk_changed(int)));

	connect(ui->bn_loaddef, SIGNAL(clicked()), this, SLOT(bn_clicked()));
	connect(ui->bn_loadcfg, SIGNAL(clicked()), this, SLOT(bn_clicked()));
	connect(ui->bn_savecfg, SIGNAL(clicked()), this, SLOT(bn_clicked()));

	connect(ui->chk_grab, SIGNAL(stateChanged(int)), this, SLOT(chk_changed(int)));
	connect(ui->chk_swapyz, SIGNAL(stateChanged(int)), this, SLOT(chk_changed(int)));

	connect(ui->slider_sens, SIGNAL(sliderMoved(int)), this, SLOT(slider_moved(int)));
	connect(ui->spin_sens, SIGNAL(valueChanged(double)), this, SLOT(dspin_changed(double)));
	connect(ui->spin_dead, SIGNAL(valueChanged(int)), this, SLOT(spin_changed(int)));
	for(int i=0; i<6; i++) {
		connect(slider_sens_axis[i], SIGNAL(sliderMoved(int)), this, SLOT(slider_moved(int)));
		connect(spin_sens_axis[i], SIGNAL(valueChanged(double)), this, SLOT(dspin_changed(double)));
		connect(spin_dead_axis[i], SIGNAL(valueChanged(int)), this, SLOT(spin_changed(int)));

		connect(chk_inv[i], SIGNAL(stateChanged(int)), this, SLOT(chk_changed(int)));

		connect(combo_axismap[i], SIGNAL(currentIndexChanged(int)), this, SLOT(combo_idx_changed(int)));
	}
}

MainWin::~MainWin()
{
	delete [] bnrow_root;
	delete [] bnrow;
	delete vbox_bnui;

	delete ui;
	delete dev_atlas;
}

void MainWin::updateui()
{
	mask_events = true;

	struct device_image devimg = devimglist[0];
	for(int i=0; devimglist[i].devtype != -1; i++) {
		if(devimglist[i].devtype == devinfo.type) {
			devimg = devimglist[i];
			break;
		}
	}
	int ncol = dev_atlas->width() / devimg.width;
	int nrow = dev_atlas->height() / devimg.height;
	devimg.xoffs = devimg.xoffs * dev_atlas->width() / ncol;
	devimg.yoffs = devimg.yoffs * dev_atlas->height() / nrow;

	QPixmap pix = dev_atlas->copy(devimg.xoffs, devimg.yoffs, devimg.width, devimg.height);
	ui->img_dev->setPixmap(pix);

	ui->lb_devname->setText(devinfo.name);
	ui->lb_devfile->setText(devinfo.path);
	ui->lb_numaxes->setText(QString::number(devinfo.naxes));
	ui->lb_numbn->setText(QString::number(devinfo.nbuttons));

	ui->combo_led->setCurrentIndex(cfg.led);
	ui->chk_grab->setChecked(cfg.grab);
	if(cfg.serdev) {
		ui->ed_serpath->setText(cfg.serdev);
		ui->chk_serial->setChecked(true);
	} else {
		ui->chk_serial->setChecked(false);
	}
	if(cfg.repeat > 0) {
		ui->chk_repeat->setChecked(true);
		ui->spin_repeat->setValue(cfg.repeat);
	} else {
		ui->chk_repeat->setChecked(false);
	}

	ui->slider_sens->setValue(cfg.sens * 10);
	ui->spin_sens->setValue(cfg.sens);
	for(int i=0; i<6; i++) {
		slider_sens_axis[i]->setValue(cfg.sens_axis[i] * 10);
		spin_sens_axis[i]->setValue(cfg.sens_axis[i]);
		chk_inv[i]->setChecked((cfg.invert >> i) & 1);

		combo_axismap[i]->clear();
		combo_axismap[i]->addItem("-");
		for(int j=0; j<devinfo.naxes; j++) {
			combo_axismap[i]->addItem(QString::number(j));
			combo_axismap[i]->setCurrentIndex(0);
		}
		for(int j=0; j<devinfo.naxes; j++) {
			if(cfg.map_axis[j] == i) {
				combo_axismap[i]->setCurrentIndex(j + 1);
				spin_dead_axis[i]->setValue(cfg.dead_thres[j]);
			}
		}

		prog_axis[i]->setValue(0);
		prog_axis[i]->setEnabled(1);
	}

	bool same = true;
	for(int i=0; i<devinfo.naxes; i++) {
		if(i > 0 && cfg.dead_thres[i] != cfg.dead_thres[i - 1]) {
			same = false;
		}
	}

	ui->spin_dead->setValue(same ? cfg.dead_thres[0] : 0);
	ui->chk_dead_global->setChecked(same);

	ui->chk_swapyz->setChecked(cfg.swapyz);

	// button mapping ui
	delete [] bnrow_root;
	delete [] bnrow;
	delete vbox_bnui;

	bnrow_count = devinfo.nbuttons;
	bnrow = new Ui::row_bnmap[bnrow_count];
	bnrow_root = new QWidget[bnrow_count];

	vbox_bnui = new QVBoxLayout;
	ui->scroll_area_buttons_cont->setLayout(vbox_bnui);

	for(int i=0; i<bnrow_count; i++) {
		bnrow[i].setupUi(bnrow_root + i);
		vbox_bnui->addWidget(bnrow_root + i);

		bnrow[i].lb_bidx->setText(QString::asprintf("%02d", i));
		bnrow[i].spin_bnmap->setMaximum(devinfo.nbuttons - 1);
		bnrow[i].spin_bnmap->setValue(cfg.map_bn[i]);

		bnrow[i].cmb_action->setCurrentIndex(cfg.bnact[i]);
		if(cfg.bnact[i]) {
			bnrow[i].rad_action->setChecked(true);
		}

		char *str;
		if(cfg.kbmap[i] > 0 && (str = XKeysymToString(cfg.kbmap[i]))) {
			bnrow[i].rad_mapkey->setChecked(true);
			bnrow[i].cmb_mapkey->setCurrentText(str);
		}
		bnrow[i].cmb_mapkey->setCompleter(0);
		def_cmb_cmap = bnrow[i].cmb_mapkey->lineEdit()->palette();

		connect(bnrow[i].rad_bnmap, SIGNAL(toggled(bool)), this, SLOT(rad_changed(bool)));
		connect(bnrow[i].spin_bnmap, SIGNAL(valueChanged(int)), this, SLOT(spin_changed(int)));
		connect(bnrow[i].rad_action, SIGNAL(toggled(bool)), this, SLOT(rad_changed(bool)));
		connect(bnrow[i].cmb_action, SIGNAL(currentIndexChanged(int)), this, SLOT(combo_idx_changed(int)));
		connect(bnrow[i].rad_mapkey, SIGNAL(toggled(bool)), this, SLOT(rad_changed(bool)));
		connect(bnrow[i].cmb_mapkey, SIGNAL(currentTextChanged(const QString&)), this, SLOT(combo_str_changed(const QString&)));
	}

	mask_events = false;
}

void MainWin::spnav_input()
{
	static unsigned char bnstate[MAX_BUTTONS];
	static int maxval = 256;
	char bnstr[MAX_BUTTONS * 4 + 20];
	char *endp;
	spnav_event ev;
	QLabel *lb;
	QPalette cmap;
	static QPalette def_cmap;

	while(spnav_poll_event(&ev)) {
		switch(ev.type) {
		case SPNAV_EVENT_MOTION:
			for(int i=0; i<6; i++) {
				if(abs(ev.motion.data[i] > maxval)) maxval = abs(ev.motion.data[i]);
			}
			for(int i=0; i<6; i++) {
				prog_axis[i]->setMinimum(-maxval);
				prog_axis[i]->setMaximum(maxval);
				prog_axis[i]->setValue(ev.motion.data[i]);
			}
			break;

		case SPNAV_EVENT_RAWBUTTON:
			assert(ev.button.bnum < bnrow_count);
			lb = bnrow[ev.button.bnum].lb_bidx;

			if(ev.button.press) {
				bnstate[ev.button.bnum] = 1;
				def_cmap = cmap = lb->palette();
				cmap.setColor(QPalette::WindowText, Qt::red);
				lb->setPalette(cmap);
			} else {
				bnstate[ev.button.bnum] = 0;
				lb->setPalette(def_cmap);
			}

			strcpy(bnstr, "Buttons pressed:");
			endp = bnstr + strlen(bnstr);
			for(int i=0; i<devinfo.nbuttons; i++) {
				if(bnstate[i]) {
					endp += sprintf(endp, " %02d", i);
				}
			}
			ui->lb_bnstate->setText(bnstr);
			break;

		case SPNAV_EVENT_CFG:
			read_cfg(&cfg);
			break;

		default:
			break;
		}
	}
}

static const char *qdefaults_text =
	"Restoring the default spacenavd settings will undo all changes.\n"
	"Are you sure you want to proceed?";
static const char *qload_text =
	"Restoring spacenavd settings from the configuration file, will undo any changes since the last save!\n"
	"Are you sure you want to proceed?";
static const char *qsave_text =
	"Saving will overwrite the current spacenavd configuration file.\n"
	"Are you sure you want to proceed?";

void MainWin::bn_clicked()
{
	QObject *src = QObject::sender();
	if(src == ui->bn_loaddef) {
		if(QMessageBox::question(this, "Reset defaults?", qdefaults_text) == QMessageBox::Yes) {
			spnav_cfg_reset();
			read_cfg(&cfg);
		}
	} else if(src == ui->bn_loadcfg) {
		if(QMessageBox::question(this, "Restore configuration?", qload_text) == QMessageBox::Yes) {
			spnav_cfg_restore();
			read_cfg(&cfg);
		}
	} else if(src == ui->bn_savecfg) {
		if(QMessageBox::question(this, "Save configuration?", qsave_text) == QMessageBox::Yes) {
			spnav_cfg_save();
		}
	}
}

void MainWin::slider_moved(int val)
{
	if(mask_events) return;

	QObject *src = QObject::sender();
	if(src == ui->slider_sens) {
		cfg.sens = val / 10.0f;
		ui->spin_sens->setValue(cfg.sens);
		spnav_cfg_set_sens(cfg.sens);
		return;
	}

	for(int i=0; i<6; i++) {
		if(src == slider_sens_axis[i]) {
			cfg.sens_axis[i] = val / 10.0f;
			spin_sens_axis[i]->setValue(cfg.sens_axis[i]);
			spnav_cfg_set_axis_sens(cfg.sens_axis);
			return;
		}
	}
}

void MainWin::dspin_changed(double val)
{
	if(mask_events) return;

	QObject *src = QObject::sender();
	if(src == ui->spin_sens) {
		cfg.sens = val;
		ui->slider_sens->setValue(val * 10.0f);
		spnav_cfg_set_sens(cfg.sens);
		return;
	}

	for(int i=0; i<6; i++) {
		if(src == spin_sens_axis[i]) {
			cfg.sens_axis[i] = val;
			slider_sens_axis[i]->setValue(val * 10.0f);
			spnav_cfg_set_axis_sens(cfg.sens_axis);
			return;
		}
	}
}

void MainWin::spin_changed(int val)
{
	if(mask_events) return;

	QObject *src = QObject::sender();
	if(src == ui->spin_dead) {
		for(int i=0; i<devinfo.naxes; i++) {
			if(cfg.dead_thres[i] != val) {
				cfg.dead_thres[i] = val;
				spnav_cfg_set_deadzone(i, val);
			}
		}
		return;
	}

	for(int i=0; i<6; i++) {
		if(src == spin_dead_axis[i]) {
			cfg.dead_thres[i] = val;
			spnav_cfg_set_deadzone(i, val);
			return;
		}
	}
}


void MainWin::chk_changed(int checked)
{
	if(mask_events) return;

	QObject *src = QObject::sender();
	if(src == ui->chk_grab) {
		cfg.grab = checked;
		spnav_cfg_set_grab(checked);
		return;
	}

	if(src == ui->chk_serial) {
		*cfg.serdev = 0;
		spnav_cfg_set_serial(cfg.serdev);
	}

	if(src == ui->chk_swapyz) {
		cfg.swapyz = checked;
		spnav_cfg_set_swapyz(checked);
		return;
	}

	for(int i=0; i<6; i++) {
		if(src == chk_inv[i]) {
			if(checked) {
				cfg.invert |= 1 << i;
			} else {
				cfg.invert &= ~(1 << i);
			}
			spnav_cfg_set_invert(cfg.invert);
			return;
		}
	}
}

void MainWin::rad_changed(bool active)
{
	if(mask_events) return;

	QObject *src = QObject::sender();
	for(int i=0; i<bnrow_count; i++) {
		if(src == bnrow[i].rad_bnmap) {
			if(!active) return;
			spnav_cfg_set_bnmap(i, cfg.map_bn[i]);
			return;
		}
		if(src == bnrow[i].rad_action) {
			if(active) {
				spnav_cfg_set_bnaction(i, cfg.bnact[i]);
			} else {
				spnav_cfg_set_bnaction(i, SPNAV_BNACT_NONE);
			}
			return;
		}
		if(src == bnrow[i].rad_mapkey) {
			if(active) {
				if(cfg.kbmap[i]) {
					spnav_cfg_set_kbmap(i, cfg.kbmap[i]);
				}
			} else {
				spnav_cfg_set_kbmap(i, 0);
			}
			return;
		}
	}
}

static void unmap_axis(int axis, int skip_devaxis)
{
	for(int i=0; i<devinfo.naxes; i++) {
		if(skip_devaxis == i) continue;
		if(cfg.map_axis[i] == axis) {
			cfg.map_axis[i] = -1;
			spnav_cfg_set_axismap(i, -1);
		}
	}
}

void MainWin::combo_idx_changed(int sel)
{
	if(mask_events) return;

	QObject *src = QObject::sender();
	if(src == ui->combo_led) {
		cfg.led = sel;
		spnav_cfg_set_led(sel);
		return;
	}

	for(int i=0; i<6; i++) {
		if(src == combo_axismap[i]) {
			int devaxis = sel - 1;

			if(devaxis < 0) {
				unmap_axis(i, -1);
				prog_axis[i]->setEnabled(0);
				prog_axis[i]->setValue(0);
			} else {
				unmap_axis(i, devaxis);
				cfg.map_axis[devaxis] = i;
				spnav_cfg_set_axismap(devaxis, i);

				for(int j=0; j<6; j++) {
					if(j != i && combo_axismap[j]->currentIndex() == sel) {
						mask_events = true;
						combo_axismap[j]->setCurrentIndex(0);
						prog_axis[j]->setEnabled(0);
						prog_axis[j]->setValue(0);
						mask_events = false;
					}
				}

				if(!prog_axis[i]->isEnabled()) {
					prog_axis[i]->setEnabled(1);
				}
			}
			return;
		}
	}

	for(int i=0; i<bnrow_count; i++) {
		if(src == bnrow[i].cmb_action) {
			cfg.bnact[i] = bnrow[i].cmb_action->currentIndex();
			spnav_cfg_set_bnaction(i, cfg.bnact[i]);
			return;
		}
	}
}

void MainWin::combo_str_changed(const QString &qstr)
{
	const char *str;

	if(mask_events) return;

	QObject *src = QObject::sender();
	for(int i=0; i<bnrow_count; i++) {
		if(src == bnrow[i].cmb_mapkey) {
			str = qstr.toLatin1().data();
			if(!str || !*str) return;

			QLineEdit *ed = bnrow[i].cmb_mapkey->lineEdit();
			KeySym sym = XStringToKeysym(str);
			if(sym == NoSymbol) {
				QPalette cmap = def_cmb_cmap;
				cmap.setColor(QPalette::Text, Qt::red);
				ed->setPalette(cmap);
				return;
			}
			ed->setPalette(def_cmb_cmap);

			cfg.kbmap[i] = sym;
			spnav_cfg_set_kbmap(i, cfg.kbmap[i]);
			return;
		}
	}
}

void MainWin::serpath_changed()
{
	free(cfg.serdev);
	cfg.serdev = strdup(ui->ed_serpath->text().toUtf8().data());

	if(cfg.serdev) {
		spnav_cfg_set_serial(cfg.serdev);
		read_cfg(&cfg);
	}
}

extern "C" void update_ui(void)
{
	mainwin->updateui();
}

extern "C" void errorbox(const char *msg)
{
	QMessageBox::critical(mainwin, "Error", msg, QMessageBox::Ok);
}

extern "C" void errorboxf(const char *fmt, ...)
{
	va_list ap;
	static char buf[512];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	errorbox(buf);
}
