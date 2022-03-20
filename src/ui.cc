#include <stdio.h>
#define SPNAV_CONFIG_H_
#include <spnav.h>
#include "ui.h"
#include "spnavcfg.h"
#include "ui_mainwin.h"
#include <QMessageBox>

static QSlider *slider_sens_axis[6];
static QCheckBox *chk_inv[6];
static QComboBox *combo_axismap[6];
static QDoubleSpinBox *spin_sens_axis[6];
static QSpinBox *spin_dead_axis[6];
static QProgressBar *prog_axis[6];
static QPixmap *dev_atlas;


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

	connect(ui->bn_loaddef, SIGNAL(clicked()), this, SLOT(bn_clicked()));
	connect(ui->bn_loadcfg, SIGNAL(clicked()), this, SLOT(bn_clicked()));
	connect(ui->bn_savecfg, SIGNAL(clicked()), this, SLOT(bn_clicked()));

	connect(ui->slider_sens, SIGNAL(sliderMoved(int)), this, SLOT(slider_moved(int)));
	connect(ui->spin_sens, SIGNAL(valueChanged(double)), this, SLOT(dspin_changed(double)));
	for(int i=0; i<6; i++) {
		connect(slider_sens_axis[i], SIGNAL(sliderMoved(int)), this, SLOT(slider_moved(int)));
		connect(spin_sens_axis[i], SIGNAL(valueChanged(double)), this, SLOT(dspin_changed(double)));
	}
}

MainWin::~MainWin()
{
	delete ui;
	delete dev_atlas;
}

void MainWin::updateui()
{
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
		for(int j=0; j<devinfo.naxes; j++) {
			combo_axismap[i]->addItem(QString::number(j));
		}
		for(int j=0; j<devinfo.naxes; j++) {
			if(cfg.map_axis[j] == i) {
				combo_axismap[i]->setCurrentIndex(j);
				spin_dead_axis[i]->setValue(cfg.dead_thres[j]);
			}
		}
	}

	bool same = true;
	for(int i=0; i<devinfo.naxes; i++) {
		if(i > 0 && cfg.dead_thres[i] != cfg.dead_thres[i - 1]) {
			same = false;
		}
	}

	ui->spin_dead->setValue(same ? cfg.dead_thres[0] : 0);
	ui->chk_dead_global->setChecked(same);
}

void MainWin::spnav_input()
{
	static int maxval = 256;
	spnav_event ev;

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

		case SPNAV_EVENT_BUTTON:
			// TODO
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
		}
	} else if(src == ui->bn_loadcfg) {
		if(QMessageBox::question(this, "Restore configuration?", qload_text) == QMessageBox::Yes) {
			spnav_cfg_restore();
		}
	} else if(src == ui->bn_savecfg) {
		if(QMessageBox::question(this, "Save configuration?", qsave_text) == QMessageBox::Yes) {
			spnav_cfg_save();
		}
	}
}

void MainWin::slider_moved(int val)
{
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
			break;
		}
	}
}

void MainWin::dspin_changed(double val)
{
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
			break;
		}
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
