/*
spnavcfg - an interactive GUI configurator for the spacenavd daemon.
Copyright (C) 2007 John Tsiombikas <nuclear@siggraph.org>

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
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "cfgfile.h"
#include "cmd.h"

#define CFGFILE		"/etc/spnavrc"

static void update_cfg(void);
static void layout(void);
static void sig(int s);
static void chk_handler(GtkToggleButton *bn, void *data);
static void slider_handler(GtkRange *rng, void *data);

static void add_child(GtkWidget *parent, GtkWidget *child);
static GtkWidget *create_vbox(GtkWidget *parent);
static GtkWidget *create_hbox(GtkWidget *parent);

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

	signal(SIGUSR1, sig);
	signal(SIGUSR2, sig);

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

static void layout(void)
{
	int i;
	GtkWidget *w;
	GtkWidget *vbox, *hbox, *bbox, *tbl, *frm;

	vbox = create_vbox(win);

	frm = gtk_frame_new("invert axis");
	add_child(vbox, frm);

	tbl = gtk_table_new(3, 4, FALSE);
	add_child(frm, tbl);

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

	bbox = gtk_hbutton_box_new();
	add_child(vbox, bbox);

	/*w = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(bn_handler), 0);
	add_child(bbox, w);*/

	w = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(gtk_main_quit), 0);
	add_child(bbox, w);
}

static void sig(int s)
{
	GtkWidget *dlg;

	switch(s) {
	case SIGUSR1:	/* daemon alive */
		dlg = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "The spacenavd driver is running fine.");
		g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		break;

	case SIGUSR2:	/* daemon dead */
		dlg = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "The driver isn't running at the moment.\n"
				"You can still modify the configuration through this panel though.");
		g_signal_connect_swapped(dlg, "response", G_CALLBACK(gtk_widget_destroy), dlg);
		break;

	default:
		break;
	}
}

static void chk_handler(GtkToggleButton *bn, void *data)
{
	int which = (int)data;
	int state = gtk_toggle_button_get_active(bn);

	cfg.invert[which] = state;
	update_cfg();
}

static void slider_handler(GtkRange *rng, void *data)
{
	cfg.sensitivity = gtk_range_get_value(rng);
	update_cfg();
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

static GtkWidget *create_hbox(GtkWidget *parent)
{
	GtkWidget *box = gtk_hbox_new(FALSE, 5);
	add_child(parent, box);
	return box;
}
