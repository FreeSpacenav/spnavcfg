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

#define CFGFILE		"/etc/spnavrc"

enum {
	SLIDER_GLOBAL,
	SLIDER_TRANS_X,
	SLIDER_TRANS_Y,
	SLIDER_TRANS_Z,
	SLIDER_ROT_X,
	SLIDER_ROT_Y,
	SLIDER_ROT_Z,
	SLIDER_DEADZONE
};

enum {
	BN_START,
	BN_STOP,
	BN_CHECK,
	BN_PING
};

enum {
	CHK_INV_TX,
	CHK_INV_TY,
	CHK_INV_TZ,
	CHK_INV_RX,
	CHK_INV_RY,
	CHK_INV_RZ,

	CHK_X11,
	CHK_LED,
	CHK_SWAP_YZ
};
#define IS_CHK_INV(x)	((x) <= CHK_INV_RZ)


int get_daemon_pid(void);	/* back.c */

static const int def_axinv[] = {0, 1, 1, 0, 1, 1};

static void update_cfg(void);
static void layout(void);
static void chk_handler(GtkToggleButton *bn, void *data);
static void slider_handler(GtkRange *rng, void *data);
static void bn_handler(GtkButton *bn, void *data);

static void add_child(GtkWidget *parent, GtkWidget *child);
static GtkWidget *create_vbox(GtkWidget *parent);

static GtkWidget *win;

static struct cfg cfg;
static int pipe_fd;

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

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win), "spacenavd configuration");
	gtk_window_set_default_size(GTK_WINDOW(win), 250, 350);
	g_signal_connect(G_OBJECT(win), "delete_event", G_CALLBACK(gtk_main_quit), 0);
	gtk_container_set_border_width(GTK_CONTAINER(win), 4);

	layout();

	gtk_widget_show_all(win);

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

static void layout(void)
{
	int i;
	GtkWidget *w, *vbox, *vbox2, *bbox, *tbl, *frm;

	vbox = create_vbox(win);

	frm = gtk_frame_new("invert axis");
	add_child(vbox, frm);

	tbl = gtk_table_new(3, 4, FALSE);
	add_child(frm, tbl);

	gtk_table_set_row_spacings(GTK_TABLE(tbl), 2);
	gtk_table_set_col_spacings(GTK_TABLE(tbl), 2);

	gtk_table_attach_defaults(GTK_TABLE(tbl), gtk_label_new("X"), 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tbl), gtk_label_new("Y"), 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tbl), gtk_label_new("Z"), 3, 4, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tbl), gtk_label_new("translation"), 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tbl), gtk_label_new("rotation"), 0, 1, 2, 3);

	for(i=0; i<6; i++) {
		int x = i % 3 + 1;
		int y = i / 3 + 1;
		w = gtk_check_button_new();
		gtk_table_attach_defaults(GTK_TABLE(tbl), w, x, x + 1, y, y + 1);
		if(cfg.invert[i] != def_axinv[i]) {
			gtk_toggle_button_set_active(w, True);
		}
		g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)i);
	}

	w = gtk_check_button_new_with_label("swap y-z axes");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), cfg.map_axis[1] == 1);
	g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)CHK_SWAP_YZ);
	add_child(vbox, w);

	frm = gtk_frame_new("sensitivity global");
	add_child(vbox, frm);

	/* -- global sensitivity slider -- */
	
	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sensitivity);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_GLOBAL);
	add_child(frm, w);

	frm = gtk_frame_new("sensitivity translation");
	add_child(vbox, frm);

	tbl = gtk_table_new(2, 3, FALSE);
	add_child(frm, tbl);

	gtk_table_set_row_spacings(GTK_TABLE(tbl), 2);
	gtk_table_set_col_spacings(GTK_TABLE(tbl), 2);

	/* -- translation-x sensitivity slider -- */
	w = gtk_label_new("X");
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 0, 1);

	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sens_trans[0]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_TRANS_X);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 0, 1);

	/* -- translation-y sensitivity slider -- */
	w = gtk_label_new("Y");
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 1, 2);

	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sens_trans[1]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_TRANS_Y);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 1, 2);

	/* -- translation-z sensitivity slider -- */
	w = gtk_label_new("Z");
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 2, 3);

	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sens_trans[2]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_TRANS_Z);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 2, 3);

	frm = gtk_frame_new("sensitivity roation");
	add_child(vbox, frm);

	tbl = gtk_table_new(2, 3, FALSE);
	add_child(frm, tbl);

	gtk_table_set_row_spacings(GTK_TABLE(tbl), 2);
	gtk_table_set_col_spacings(GTK_TABLE(tbl), 2);

	/* -- rotation-x sensitivity slider -- */
	w = gtk_label_new("X");
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 0, 1);

	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sens_rot[0]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_ROT_X);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 0, 1);

	/* -- rotation-y sensitivity slider -- */
	w = gtk_label_new("Y");
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 1, 2);

	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sens_rot[1]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_ROT_Y);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 1, 2);

	/* -- rotation-z sensitivity slider -- */
	w = gtk_label_new("Z");
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 0, 1, 2, 3);

	w = gtk_hscale_new_with_range(0.0, 6.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sens_rot[2]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_ROT_Z);
	gtk_table_attach_defaults(GTK_TABLE(tbl), w, 1, 2, 2, 3);

	/* -- deadzone slider -- */
	frm = gtk_frame_new("deadzone");
	add_child(vbox, frm);
	
	w = gtk_hscale_new_with_range(0.0, 30.0, 1.0);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.dead_threshold[0]);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), (void*)SLIDER_DEADZONE);
	add_child(frm, w);

	/*
	frm = gtk_frame_new("X11 magellan API");
	add_child(vbox, frm);

	bbox = gtk_hbutton_box_new();
	add_child(frm, bbox);

	w = gtk_button_new_with_label("start");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)BN_START);
	add_child(bbox, w);

	w = gtk_button_new_with_label("stop");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)BN_STOP);
	add_child(bbox, w);

	w = gtk_button_new_with_label("check");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)BN_CHECK);
	add_child(bbox, w);
	*/

	/*w = gtk_check_button_new_with_label("enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), query_x11());
	g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)CHK_X11);
	add_child(frm, w);*/

	frm = gtk_frame_new("misc");
	add_child(vbox, frm);

	vbox2 = create_vbox(frm);

	w = gtk_check_button_new_with_label("enable LED");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), cfg.led);
	g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)CHK_LED);
	add_child(vbox2, w);

	bbox = gtk_hbutton_box_new();
	add_child(vbox2, bbox);

	w = gtk_button_new_with_label("ping daemon");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)BN_PING);
	add_child(bbox, w);

	bbox = gtk_hbutton_box_new();
	add_child(vbox, bbox);

	/*w = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), 0);
	add_child(bbox, w);*/

	w = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(gtk_main_quit), 0);
	add_child(bbox, w);
}

static void chk_handler(GtkToggleButton *bn, void *data)
{
	int tmp;
	int which = (int)data;
	int state = gtk_toggle_button_get_active(bn);

	if(IS_CHK_INV(which)) {
		cfg.invert[which] = !cfg.invert[which];
		update_cfg();
		return;
	}

	switch(which) {
	case CHK_X11:
		tmp = state ? CMD_STARTX : CMD_STOPX;
		write(pipe_fd, &tmp, 1);
		break;

	case CHK_LED:
		cfg.led = state;
		update_cfg();
		break;

	case CHK_SWAP_YZ:
		tmp = cfg.map_axis[1];
		cfg.map_axis[1] = cfg.map_axis[2];
		cfg.map_axis[2] = tmp;

		tmp = cfg.map_axis[4];
		cfg.map_axis[4] = cfg.map_axis[5];
		cfg.map_axis[5] = tmp;

		update_cfg();
		break;

	default:
		break;
	}
}

static void slider_handler(GtkRange *rng, void *data)
{
	int id = (int)data;
	int i;

	switch(id) {
	case SLIDER_GLOBAL:
		cfg.sensitivity = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_TRANS_X:
		cfg.sens_trans[0] = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_TRANS_Y:
		cfg.sens_trans[1] = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_TRANS_Z:
		cfg.sens_trans[2] = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_ROT_X:
		cfg.sens_rot[0] = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_ROT_Y:
		cfg.sens_rot[1] = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_ROT_Z:
		cfg.sens_rot[2] = gtk_range_get_value(rng);
		update_cfg();
		break;

	case SLIDER_DEADZONE:
		for(i=0; i<6; i++) {
			cfg.dead_threshold[i] = gtk_range_get_value(rng);
		}
		update_cfg();
		break;

	default:
		break;
	}
}

static void bn_handler(GtkButton *bn, void *data)
{
	GtkWidget *dlg;
	int id = (int)data;
	int tmp;

	switch(id) {
	case BN_START:
	case BN_STOP:
		tmp = id == BN_STOP ? CMD_STOPX : CMD_STARTX;
		write(pipe_fd, &tmp, 1);
		break;

	case BN_CHECK:
		dlg = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "The X11 Magellan (3dxsrv-compatible) API is %s.",
				query_x11() ? "enabled" : "disabled");
		gtk_widget_show_all(dlg);
		g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		break;

	case BN_PING:
		tmp = CMD_PING;
		write(pipe_fd, &tmp, 1);
		read(pipe_fd, &tmp, 1);

		if(tmp) {	/* daemon alive */
			dlg = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "The spacenavd driver is running fine.");
			gtk_widget_show_all(dlg);
			g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		} else {	/* daemon dead */
			dlg = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "The driver isn't running at the moment.\n"
				"You can still modify the configuration through this panel though.");
			gtk_widget_show_all(dlg);
			g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		}
		break;

	default:
		break;
	}
}

static void add_child(GtkWidget *parent, GtkWidget *child)
{
	if(GTK_IS_BOX(parent)) {
		gtk_box_pack_start(GTK_BOX(parent), child, TRUE, TRUE, 3);
	} else if(GTK_CONTAINER(parent)) {
		gtk_container_add(GTK_CONTAINER(parent), child);
	} else {
		fprintf(stderr, "failed to add child\n");
	}
}

static GtkWidget *create_vbox(GtkWidget *parent)
{
	GtkWidget *box = gtk_vbox_new(FALSE, 5);
	add_child(parent, box);
	return box;
}
