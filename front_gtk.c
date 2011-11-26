/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007-2009 John Tsiombikas <nuclear@siggraph.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "cfgfile.h"
#include "cmd.h"
#include "ui.h"

#define CFGFILE		"/etc/spnavrc"

#define CHK_AXINV_TRANS_X			"axinv_trans_x"
#define CHK_AXINV_TRANS_Y			"axinv_trans_y"
#define CHK_AXINV_TRANS_Z			"axinv_trans_z"

#define CHK_AXINV_ROT_X				"axinv_rot_x"
#define CHK_AXINV_ROT_Y				"axinv_rot_y"
#define CHK_AXINV_ROT_Z				"axinv_rot_z"

#define CHK_SWAP_YZ					"swap_yz"

#define CHK_ENABLE_LED				"enable_led"
#define CHK_GRAB_DEVICE				"grab_device"

#define SLIDER_SENS_GLOBAL			"sens_global"

#define SLIDER_SENS_TRANS			"sens_trans"
#define SLIDER_SENS_TRANS_X			"sens_trans_x"
#define SLIDER_SENS_TRANS_Y			"sens_trans_y"
#define SLIDER_SENS_TRANS_Z			"sens_trans_z"

#define SLIDER_SENS_ROT				"sens_rot"
#define SLIDER_SENS_ROT_X			"sens_rot_x"
#define SLIDER_SENS_ROT_Y			"sens_rot_y"
#define SLIDER_SENS_ROT_Z			"sens_rot_z"

#define SLIDER_DEADZONE				"deadzone"
#define SLIDER_DEADZONE_TRANS_X		"deadzone_trans_x"
#define SLIDER_DEADZONE_TRANS_Y		"deadzone_trans_y"
#define SLIDER_DEADZONE_TRANS_Z		"deadzone_trans_z"
#define SLIDER_DEADZONE_ROT_X		"deadzone_rot_x"
#define SLIDER_DEADZONE_ROT_Y		"deadzone_rot_y"
#define SLIDER_DEADZONE_ROT_Z		"deadzone_rot_z"

#define BTN_PING					"ping_daemon"



int get_daemon_pid(void);	/* back.c */

enum {TX, TY, TZ, RX, RY, RZ};

static const int def_axinv[] = {0, 1, 1, 0, 1, 1};

static void update_cfg(void);
static void create_ui(void);

G_MODULE_EXPORT void chk_handler(GtkToggleButton *bn, gpointer data);
G_MODULE_EXPORT void slider_handler(GtkRange *rng, gpointer data);
G_MODULE_EXPORT void bn_handler(GtkButton *bn, gpointer data);

static struct cfg cfg;
static int pipe_fd;

struct widgets {

	GtkWidget *win;
	GObject *slider_sens_trans_x;
	GObject *slider_sens_trans_y;
	GObject *slider_sens_trans_z;
	GObject *slider_sens_rot_x;
	GObject *slider_sens_rot_y;
	GObject *slider_sens_rot_z;
	GObject *slider_deadzone_trans_x;
	GObject *slider_deadzone_trans_y;
	GObject *slider_deadzone_trans_z;
	GObject *slider_deadzone_rot_x;
	GObject *slider_deadzone_rot_y;
	GObject *slider_deadzone_rot_z;

};

static struct widgets widgets;




void frontend(int pfd)
{
	int argc = 0;
	int ruid = getuid();
#ifdef __linux__
	int setresuid(uid_t ruid, uid_t euid, uid_t suid);
	setresuid(ruid, ruid, ruid);
#else
	seteuid(ruid);
#endif

	pipe_fd = pfd;

	gtk_init(&argc, 0);

	read_cfg(CFGFILE, &cfg);

	create_ui();

	gtk_widget_show_all(widgets.win);

	gtk_main();
}

static void update_cfg(void)
{
	int cmd = CMD_CFG;
	write(pipe_fd, &cmd, 1);
	write(pipe_fd, &cfg, sizeof cfg);
}

static int query_x11(void)
{
	Display *dpy;
	Window win, root_win;
	XTextProperty wname;
	Atom type, command_event;
	int fmt;
	unsigned long nitems, bytes_after;
	unsigned char *prop;

	if(!(dpy = XOpenDisplay(0))) {
		return 0;
	}
	root_win = DefaultRootWindow(dpy);

	if((command_event = XInternAtom(dpy, "CommandEvent", True)) == None) {
		XCloseDisplay(dpy);
		return 0;
	}

	XGetWindowProperty(dpy, root_win, command_event, 0, 1, False, AnyPropertyType,
			&type, &fmt, &nitems, &bytes_after, &prop);
	if(!prop) {
		XCloseDisplay(dpy);
		return 0;
	}

	win = *(Window*)prop;
	XFree(prop);

	if(!XGetWMName(dpy, win, &wname) || strcmp("Magellan Window", (char*)wname.value) != 0) {
		XCloseDisplay(dpy);
		return 0;
	}
	XCloseDisplay(dpy);

	/* found a magellan window, still it might belong to the 3dxsrv driver */
	if(get_daemon_pid() == -1) {
		return 0;
	}

	/* this could still mean that the daemon crashed and left behind the pidfile... */
	return 1;	/* ... but wtf */
}

static void create_ui(void)
{
	GObject *obj;
	GtkBuilder *gtk_builder;

	gtk_builder = gtk_builder_new();
	gtk_builder_add_from_string(gtk_builder, (gchar*)ui_xml, -1, NULL);

	widgets.win = GTK_WIDGET(gtk_builder_get_object(gtk_builder, "main"));

	obj = gtk_builder_get_object (gtk_builder, CHK_AXINV_TRANS_X);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.invert[TX] != def_axinv[TX]);
	obj = gtk_builder_get_object (gtk_builder, CHK_AXINV_TRANS_Y);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.invert[TY] != def_axinv[TY]);
	obj = gtk_builder_get_object (gtk_builder, CHK_AXINV_TRANS_Z);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.invert[TZ] != def_axinv[TZ]);
	obj = gtk_builder_get_object (gtk_builder, CHK_AXINV_ROT_X);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.invert[RX] != def_axinv[RX]);
	obj = gtk_builder_get_object (gtk_builder, CHK_AXINV_ROT_Y);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.invert[RY] != def_axinv[RY]);
	obj = gtk_builder_get_object (gtk_builder, CHK_AXINV_ROT_Z);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.invert[RZ] != def_axinv[RZ]);

	obj = gtk_builder_get_object (gtk_builder, CHK_SWAP_YZ);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.map_axis[1] == 1);

	obj = gtk_builder_get_object (gtk_builder, SLIDER_SENS_GLOBAL);
	gtk_range_set_value(GTK_RANGE(obj), cfg.sensitivity);

	obj = gtk_builder_get_object (gtk_builder, SLIDER_SENS_TRANS);
	gtk_range_set_value(GTK_RANGE(obj), cfg.sens_trans[0]);

	widgets.slider_sens_trans_x = gtk_builder_get_object (gtk_builder, SLIDER_SENS_TRANS_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_x), cfg.sens_trans[0]);
	widgets.slider_sens_trans_y = gtk_builder_get_object (gtk_builder, SLIDER_SENS_TRANS_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_y), cfg.sens_trans[1]);
	widgets.slider_sens_trans_z = gtk_builder_get_object (gtk_builder, SLIDER_SENS_TRANS_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_z), cfg.sens_trans[2]);

	obj = gtk_builder_get_object (gtk_builder, SLIDER_SENS_ROT);
	gtk_range_set_value(GTK_RANGE(obj), cfg.sens_rot[0]);

	widgets.slider_sens_rot_x = gtk_builder_get_object (gtk_builder, SLIDER_SENS_ROT_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_x), cfg.sens_rot[0]);
	widgets.slider_sens_rot_y = gtk_builder_get_object (gtk_builder, SLIDER_SENS_ROT_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_y), cfg.sens_rot[1]);
	widgets.slider_sens_rot_z = gtk_builder_get_object (gtk_builder, SLIDER_SENS_ROT_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_z), cfg.sens_rot[2]);

	obj = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE);
	gtk_range_set_value(GTK_RANGE(obj), cfg.dead_threshold[TX]);

	widgets.slider_deadzone_trans_x = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE_TRANS_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_x), cfg.dead_threshold[TX]);
	widgets.slider_deadzone_trans_y = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE_TRANS_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_y), cfg.dead_threshold[TY]);
	widgets.slider_deadzone_trans_z = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE_TRANS_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_z), cfg.dead_threshold[TZ]);
	widgets.slider_deadzone_rot_x = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE_ROT_X);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_x), cfg.dead_threshold[RX]);
	widgets.slider_deadzone_rot_y = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE_ROT_Y);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_y), cfg.dead_threshold[RY]);
	widgets.slider_deadzone_rot_z = gtk_builder_get_object (gtk_builder, SLIDER_DEADZONE_ROT_Z);
	gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_z), cfg.dead_threshold[RZ]);

	obj = gtk_builder_get_object (gtk_builder, CHK_GRAB_DEVICE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.grab_device);

	obj = gtk_builder_get_object (gtk_builder, CHK_ENABLE_LED);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj), cfg.led);

	gtk_builder_connect_signals (gtk_builder, NULL);

	g_object_unref (G_OBJECT (gtk_builder));

}

G_MODULE_EXPORT void chk_handler(GtkToggleButton *bn, gpointer data)
{
	int tmp;
	int state = gtk_toggle_button_get_active(bn);
	const gchar* ctrlname = gtk_buildable_get_name(GTK_BUILDABLE(bn));

	if(strcmp(ctrlname, CHK_AXINV_TRANS_X) == 0) {
		cfg.invert[TX] = !cfg.invert[TX];
		update_cfg();
	} else if(strcmp(ctrlname, CHK_AXINV_TRANS_Y) == 0) {
		cfg.invert[TY] = !cfg.invert[TY];
		update_cfg();
	} else if(strcmp(ctrlname, CHK_AXINV_TRANS_Z) == 0) {
		cfg.invert[TZ] = !cfg.invert[TZ];
		update_cfg();
	} else if(strcmp(ctrlname, CHK_AXINV_ROT_X) == 0) {
		cfg.invert[RX] = !cfg.invert[RX];
		update_cfg();
	} else if(strcmp(ctrlname, CHK_AXINV_ROT_Y) == 0) {
		cfg.invert[RY] = !cfg.invert[RY];
		update_cfg();
	} else if(strcmp(ctrlname, CHK_AXINV_ROT_Z) == 0) {
		cfg.invert[RZ] = !cfg.invert[RZ];
		update_cfg();
	} else if(strcmp(ctrlname, CHK_GRAB_DEVICE) == 0) {
		cfg.grab_device = state;
		update_cfg();
	} else if(strcmp(ctrlname, CHK_ENABLE_LED) == 0) {
		cfg.led = state;
		update_cfg();
	} else if(strcmp(ctrlname, CHK_SWAP_YZ) == 0) {
		tmp = cfg.map_axis[TY];
		cfg.map_axis[TY] = cfg.map_axis[TZ];
		cfg.map_axis[TZ] = tmp;
		tmp = cfg.map_axis[RY];
		cfg.map_axis[RY] = cfg.map_axis[RZ];
		cfg.map_axis[RZ] = tmp;
		update_cfg();
	}

}

G_MODULE_EXPORT void slider_handler(GtkRange *rng, gpointer data)
{
	int i;
	const gchar* ctrlname = gtk_buildable_get_name(GTK_BUILDABLE(rng));
	gdouble value = gtk_range_get_value(rng);

	if(strcmp(ctrlname, SLIDER_SENS_GLOBAL) == 0) {
		cfg.sensitivity = gtk_range_get_value(rng);
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS) == 0) {
		cfg.sens_trans[0] = cfg.sens_trans[1] = cfg.sens_trans[2] = value;
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_trans_z), value);
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS_X) == 0) {
		cfg.sens_trans[0] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS_Y) == 0) {
		cfg.sens_trans[1] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_TRANS_Z) == 0) {
		cfg.sens_trans[2] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT) == 0) {
		cfg.sens_rot[0] = cfg.sens_rot[1] = cfg.sens_rot[2] = value;
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_sens_rot_z), value);
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT_X) == 0) {
		cfg.sens_rot[0] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT_Y) == 0) {
		cfg.sens_rot[1] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_SENS_ROT_Z) == 0) {
		cfg.sens_rot[2] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE) == 0) {
		for(i=0; i<6; i++) {
			cfg.dead_threshold[i] = value;
		}
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_trans_z), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_x), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_y), value);
		gtk_range_set_value(GTK_RANGE(widgets.slider_deadzone_rot_z), value);
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_TRANS_X) == 0) {
		cfg.dead_threshold[TX] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_TRANS_Y) == 0) {
		cfg.dead_threshold[TY] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_TRANS_Z) == 0) {
		cfg.dead_threshold[TZ] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_ROT_X) == 0) {
		cfg.dead_threshold[RX] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_ROT_Y) == 0) {
		cfg.dead_threshold[RY] = value;
		update_cfg();
	} else if(strcmp(ctrlname, SLIDER_DEADZONE_ROT_Z) == 0) {
		cfg.dead_threshold[RZ] = value;
		update_cfg();
	}

}

G_MODULE_EXPORT void bn_handler(GtkButton *bn, gpointer data)
{
	GtkWidget *dlg;
	int tmp;
	const gchar* ctrlname = gtk_buildable_get_name(GTK_BUILDABLE(bn));

	if(strcmp(ctrlname, BTN_PING) == 0) {
		tmp = CMD_PING;
		write(pipe_fd, &tmp, 1);
		read(pipe_fd, &tmp, 1);

		if(tmp) {	/* daemon alive */
			dlg = gtk_message_dialog_new(GTK_WINDOW(widgets.win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "The spacenavd driver is running fine.");
			gtk_widget_show_all(dlg);
			g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		} else {	/* daemon dead */
			dlg = gtk_message_dialog_new(GTK_WINDOW(widgets.win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "The driver isn't running at the moment.\n"
				"You can still modify the configuration through this panel though.");
			gtk_widget_show_all(dlg);
			g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		}
	}

}

