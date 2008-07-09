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
	GList * threads;
	GList * idle_threads;
	GQueue* jobs;
	guint   schedule_idle;
	guint32 last_job_id;
};

struct QueueJob {
	Queue      * queue;
	Worker     * worker;
	GThreadFunc  async;
	gpointer     async_result;
	GFunc        destroy;
	gpointer     user_data;
	guint        id;
	guint        started : 1;
	guint        finished : 1;
};

static void queue_schedule_job (Queue          * queue,
				struct QueueJob* job);

/*
 * queue_new:
 *
 * Create a new workload queue.
 *
 * Returns the newly created #Queue.
 */
Queue*
queue_new (void)
{
	Queue* queue = g_slice_new0 (Queue);
	gint thread;
	gint n_threads = 2;

	for (thread = 0; thread < n_threads; thread++) {
		GError* error = NULL;
		Worker* worker = worker_new (thread, &error);

		if (!error) {
			queue->threads = g_list_prepend (queue->threads, worker);
			queue->idle_threads = g_list_prepend (queue->idle_threads, worker);
		} else {
			g_printerr ("error creating thread %d (%d of %d)\n",
				    worker->id, thread + 1, n_threads);
		}
	}
	queue->threads = g_list_reverse (queue->threads);

	queue->jobs = g_queue_new ();

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
	if (queue->schedule_idle) {
		g_warning (G_STRLOC ": FIXME: cancel scheduling thread");
	}

	while (queue->threads) {
		Worker* worker = queue->threads->data;
		worker_shutdown (worker);
		queue->threads = g_list_delete_link (queue->threads, queue->threads);
	}
	g_list_free (queue->idle_threads);

	if (queue->jobs->length) {
		g_warning (G_STRLOC ": FIXME: cancel unfinished jobs first");
	}
	g_queue_free (queue->jobs);

	g_slice_free (Queue, queue);
}

/* START: QueueJob API */

static struct QueueJob*
queue_job_new (guint        id,
	       Queue      * queue,
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

static gboolean
job_done (gpointer user_data)
{
	struct QueueJob* job = user_data;
	struct QueueJob* next_job;

	if (job->destroy) {
		job->destroy (job->async_result,
			      job->user_data);
	}

	g_return_val_if_fail (job->queue, FALSE);

	/* return the worker into the list of idle threads */
	job->queue->idle_threads = g_list_prepend (job->queue->idle_threads,
						   job->worker);

	next_job = g_queue_pop_head (job->queue->jobs);
	if (next_job) {
		queue_schedule_job (job->queue, next_job);
	}

	queue_job_free (job);

	return FALSE;
}

static gboolean
execute_job (gpointer user_data)
{
	/* WARNING START: executed in worker thread */
	struct QueueJob* job = user_data;

	job->async_result = job->async (job->user_data);

	GSource* source = g_idle_source_new ();
	g_source_set_callback (source,
			       job_done,
			       job,
			       NULL);
	g_source_attach (source, g_main_context_default ());
	g_source_unref (source);

	return FALSE; /* execute once */
	/* WARNING END: executed in worker thread */
}

static void
queue_schedule_job (Queue          * queue,
		    struct QueueJob* job)
{
	Worker* worker = queue->idle_threads->data;
	g_return_if_fail (worker);

	GSource* source = g_idle_source_new ();
	g_source_set_callback (source,
			       execute_job,
			       job,
			       NULL);

	queue->idle_threads = g_list_delete_link (queue->idle_threads, queue->idle_threads);

	job->worker = worker;

	g_source_attach (source, worker->context);
	g_source_unref (source);
}

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
guint32
queue_queue (Queue      * queue,
	     GThreadFunc  async,
	     GFunc        destroy,
	     gpointer     user_data)
{
	struct QueueJob* job;

	g_return_if_fail (async != NULL);
	g_return_if_fail (queue != NULL);

	job = queue_job_new (queue->last_job_id++, queue, async, destroy, user_data);

	if (G_UNLIKELY (queue->idle_threads)) {
		/* schedule immediately */
		queue_schedule_job (queue, job);
	} else {
		/* queue for scheduling */
		g_queue_push_tail (queue->jobs,
				   job);
	}

	return job->id;
}

