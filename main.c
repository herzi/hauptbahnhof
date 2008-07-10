/* This file is part of herzi's playground
 *
 * AUTHORS
 *     Sven Herzberg  <sven@imendio.com>
 *
 * Copyright (C) 2008  Sven Herzberg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <signal.h>
#include <unistd.h>
#include <glib.h>

#include <stdio.h>

#include "queue.h"
#include "worker.h"

static GMainLoop* main_loop = NULL;

static void
sigint_action (int        signal,
	       siginfo_t* info,
	       void     * context)
{
	g_printerr ("<Ctrl>-C pressed; quitting main loop\n");
	g_main_loop_quit (main_loop);
}

static gpointer
sleep_job (gpointer user_data)
{
	gint i;
	guint64 j = 0;
	for (i = 0; i < 15000; i++)
		j += i;
	return NULL;
}

static void
sleep_done (gpointer data,
	    gpointer user_data)
{
	g_print ("\rjob %d done",
		 GPOINTER_TO_INT (user_data));
}

static void
sleep_done_last (gpointer data,
		 gpointer user_data)
{
	sleep_done (data, user_data);

	g_print ("\n");
}

int
main (int   argc,
      char**argv)
{
	Queue* queue;
	struct sigaction new_handler = {0};
	gint i;

	g_thread_init (NULL);

	new_handler.sa_sigaction = sigint_action;
	new_handler.sa_flags     = SA_RESETHAND | SA_SIGINFO;

	if (0 != sigaction (SIGINT, &new_handler, NULL)) {
		g_printerr ("error setting up system handler\n");
		return 1;
	}

	queue = queue_new ();
	for (i = 0; i < 30000; i++) {
		queue_queue (queue,
			     sleep_job,
			     sleep_done,
			     GINT_TO_POINTER (i));
	}
	queue_queue (queue,
		     sleep_job,
		     sleep_done_last,
		     GINT_TO_POINTER (i));

	main_loop = g_main_loop_new (NULL, FALSE);

	g_main_loop_run (main_loop);

	g_main_loop_unref (main_loop);
	main_loop = NULL;

	queue_free (queue);

	return 0;
}

