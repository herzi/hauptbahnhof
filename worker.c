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

#include "worker.h"

static gpointer
create_worker (gpointer data)
{
	Worker* worker = data;

	worker->context = g_main_context_new ();
	worker->loop    = g_main_loop_new (worker->context, FALSE);
	g_main_loop_run (worker->loop);
	return NULL;
}

/*
 * worker_new:
 * @id: an id for the thread
 * @error: return location for a #GError
 *
 * Create a new worker thread.
 *
 * Returns: the #Worker object.
 */
Worker*
worker_new (guint   id,
	    GError**error)
{
	Worker* worker;

	g_return_val_if_fail (!error || !*error, NULL);

	worker     = g_slice_new0 (Worker);
	worker->id = id;

	worker->thread = g_thread_create (create_worker,
					  worker,
					  TRUE,
					  error);

	return worker;
}

static gboolean
worker_main_quit (gpointer data)
{
	Worker* worker = data;

	g_printerr ("quitting main loop of thread %d\n",
		    worker->id);
	g_main_loop_quit (worker->loop);

	return FALSE;
}

/*
 * worker_shutdown:
 * @worker: a #Worker
 *
 * Quit the main loop of the worker thread and join the thread until the main
 * loop is quit; the release the assotiated ressources.
 */
void
worker_shutdown (Worker* worker)
{
	GSource* quit_source = g_idle_source_new ();
	g_source_set_callback (quit_source,
			       worker_main_quit,
			       worker, NULL);
	g_source_attach (quit_source, worker->context);
	g_source_unref (quit_source);

	g_thread_join (worker->thread);
	g_main_loop_unref (worker->loop);
	g_main_context_unref (worker->context);
	g_slice_free (Worker, worker);
}

