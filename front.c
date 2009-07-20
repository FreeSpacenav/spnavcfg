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

int get_daemon_pid(void);	/* back.c */

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
	GtkWidget *w;
	GtkWidget *vbox, *vbox2, *bbox, *tbl, *frm;

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
		g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)i);
	}

	/*hbox = create_hbox(vbox);*/
	frm = gtk_frame_new("sensitivity");
	add_child(vbox, frm);

	w = gtk_hscale_new_with_range(0.0, 4.0, 0.1);
	gtk_range_set_update_policy(GTK_RANGE(w), GTK_UPDATE_DELAYED);
	gtk_range_set_value(GTK_RANGE(w), cfg.sensitivity);
	gtk_scale_set_value_pos(GTK_SCALE(w), GTK_POS_RIGHT);
	g_signal_connect(G_OBJECT(w), "value_changed", G_CALLBACK(slider_handler), 0);
	add_child(frm, w);

	frm = gtk_frame_new("X11 magellan API");
	add_child(vbox, frm);

	bbox = gtk_hbutton_box_new();
	add_child(frm, bbox);

	w = gtk_button_new_with_label("start");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)0);
	add_child(bbox, w);

	w = gtk_button_new_with_label("stop");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)1);
	add_child(bbox, w);

	w = gtk_button_new_with_label("check");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)2);
	add_child(bbox, w);

	/*
	w = gtk_check_button_new_with_label("enable X11 magellan API");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), query_x11());
	g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)10);
	add_child(frm, w);
	*/

	frm = gtk_frame_new("misc");
	add_child(vbox, frm);

	vbox2 = create_vbox(frm);

	w = gtk_check_button_new_with_label("enable LED");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), cfg.led);
	g_signal_connect(G_OBJECT(w), "toggled", G_CALLBACK(chk_handler), (void*)10);
	add_child(vbox2, w);

	bbox = gtk_hbutton_box_new();
	add_child(vbox2, bbox);

	w = gtk_button_new_with_label("ping daemon");
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), (void*)3);
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
	int which = (int)data;
	int state = gtk_toggle_button_get_active(bn);

	if(which < 6) {
		cfg.invert[which] = state;
		update_cfg();
	}

	if(which == 10) {
		cfg.led = state;
		update_cfg();
	}
}

static void slider_handler(GtkRange *rng, void *data)
{
	cfg.sensitivity = gtk_range_get_value(rng);
	update_cfg();
}

static void bn_handler(GtkButton *bn, void *data)
{
	GtkWidget *dlg;
	int id = (int)data;
	int tmp;

	switch(id) {
	case 0:
	case 1:
		tmp = id ? CMD_STOPX : CMD_STARTX;
		write(pipe_fd, &tmp, 1);
		break;

	case 2:
		dlg = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "The X11 Magellan (3dxsrv-compatible) API is %s.",
				query_x11() ? "enabled" : "disabled");
		gtk_widget_show_all(dlg);
		g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		break;

	case 3:
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
