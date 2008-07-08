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
#include <glib.h>

typedef struct {
	gint          id;
	GThread     * thread;
	GMainContext* context;
	GMainLoop   * loop;
} Worker;

static GMainLoop* main_loop = NULL;

static void
sigint_action (int        signal,
	       siginfo_t* info,
	       void     * context)
{
	g_printerr ("<Ctrl>-C pressed; exiting...\n");
	g_main_loop_quit (main_loop);
}

static gpointer
create_worker (gpointer data)
{
	Worker* worker = data;

	worker->context = g_main_context_new ();
	worker->loop    = g_main_loop_new (worker->context, FALSE);
	return NULL;
}

int
main (int   argc,
      char**argv)
{
	struct sigaction new_handler = {0};
	gint n_threads = 2;
	gint thread;
	GList* threads = NULL;

	g_thread_init (NULL);

	new_handler.sa_sigaction = sigint_action;
	new_handler.sa_flags     = SA_RESETHAND | SA_SIGINFO;

	if (0 != sigaction (SIGINT, &new_handler, NULL)) {
		g_printerr ("error setting up system handler\n");
		return 1;
	}

	main_loop = g_main_loop_new (NULL, FALSE);

	for (thread = 0; thread < n_threads; thread++) {
		GError* error = NULL;
		Worker* worker = g_slice_new0 (Worker);
		worker->id     = thread;
		worker->thread = g_thread_create (create_worker,
						  worker,
						  TRUE,
						  &error);

		if (!error) {
			threads = g_list_prepend (threads, worker);
		} else {
			g_printerr ("error creating thread %d (%d of %d)\n",
				    worker->id, thread + 1, n_threads);
		}
	}
	threads = g_list_reverse (threads);

	g_main_loop_run (main_loop);

	g_main_loop_unref (main_loop);
	main_loop = NULL;

	while (threads) {
		Worker* worker = threads->data;
		g_thread_join (worker->thread);
		g_main_loop_unref (worker->loop);
		g_main_context_unref (worker->context);
		g_slice_free (Worker, worker);
		threads = g_list_delete_link (threads, threads);
	}

	return 0;
}

