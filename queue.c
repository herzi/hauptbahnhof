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

#include "queue.h"

#include "worker.h"

Queue*
queue_new (gint n_threads)
{
	Queue* queue = g_slice_new0 (Queue);
	gint thread;

	for (thread = 0; thread < n_threads; thread++) {
		GError* error = NULL;
		Worker* worker = worker_new (thread, &error);

		if (!error) {
			queue->threads = g_list_prepend (queue->threads, worker);
		} else {
			g_printerr ("error creating thread %d (%d of %d)\n",
				    worker->id, thread + 1, n_threads);
		}
	}
	queue->threads = g_list_reverse (queue->threads);

	return queue;

}

void
queue_free (Queue* queue)
{
	g_slice_free (Queue, queue);
}

