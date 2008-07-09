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

struct _Queue {
	GList* threads;
};

/*
 * queue_new:
 * n_threads: the number of threads to create
 *
 * Create a new workload queue.
 *
 * Returns the newly created #Queue.
 */
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

/*
 * queue_free:
 * queue: the #Queue to free
 *
 * Releases all ressources assotiated to the queue.
 */
void
queue_free (Queue* queue)
{
	while (queue->threads) {
		Worker* worker = queue->threads->data;
		worker_shutdown (worker);
		queue->threads = g_list_delete_link (queue->threads, queue->threads);
	}

	g_slice_free (Queue, queue);
}

/* START: QueueJob API */

struct QueueJob {
	Queue const* queue;
	GThreadFunc  async;
	gpointer     async_result;
	GFunc        destroy;
	gpointer     user_data;
	guint        id;
	guint        started : 1;
	guint        finished : 1;
};

static struct QueueJob*
queue_job_new (guint        id,
	       Queue const* queue,
	       GThreadFunc  async,
	       GFunc        destroy,
	       gpointer     user_data)
{
	struct QueueJob* self = g_slice_new0 (struct QueueJob);
	self->id        = id;
	self->queue     = queue;
	self->async     = async;
	self->destroy   = destroy;
	self->user_data = user_data;
	return self;
}

static void
queue_job_free (struct QueueJob* self)
{
	if (self->destroy) {
		self->destroy (self->async_result, self->user_data);
	}

	g_slice_free (struct QueueJob, self);
}

/* END: QueueJob API */

/*
 * queue_queue:
 * @queue: the #Queue to attach this job request to
 * @async: the function (running in a separate thread) to be executed
 * asynchronously
 * @destroy: the function (running in the main thread) to be executed
 * after the asynchronous function finished. The result of @async will be
 * passed as the first parameter, the @user_data as the second.
 * @user_data: the user data for the GThreadFunc and GFunc
 *
 * Queue a job request.
 *
 */
/* Returns: the id of the job within this queue */
guint
queue_queue (Queue      * queue,
	     GThreadFunc  async,
	     GFunc        destroy,
	     gpointer     user_data)
{
	struct QueueJob* job;

	g_return_if_fail (async != NULL);

	job = queue_job_new (0, queue, async, destroy, user_data);

	g_warning ("FIXME: queue instead of free");
	queue_job_free (job);

	return 0;
}

