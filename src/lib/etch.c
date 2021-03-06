/* ETCH - Timeline Based Animation Library
 * Copyright (C) 2007-2008 Jorge Luis Zapata, Hisham Mardam-Bey
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include "Etch.h"
#include "etch_private.h"

/**
 * TODO
 * + maybe a function to call whenever the fps timer has match? like:
 * etc_timer_notify(Etch *)
 * + maybe remove the _timer_ prefix?
 * TODO remove every double and use Etch_Time
 */
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
static Etch_Interpolator _interpolators[ETCH_DATATYPES] = {
	[ETCH_UINT32] = etch_interpolator_uint32,
	[ETCH_INT32] = etch_interpolator_int32,
	[ETCH_ARGB] = etch_interpolator_argb,
	[ETCH_STRING] = etch_interpolator_string,
	[ETCH_FLOAT] = etch_interpolator_float,
	[ETCH_DOUBLE] = etch_interpolator_double,
	[ETCH_EXTERNAL] = NULL,
};

#define DEFAULT_FPS 30

static int _init_count = 0;

static void _fps_to_time(unsigned long frame, unsigned long *time)
{
	/* giving a frame transform it to secs|usec representation */
}

static void _process(Etch *e)
{
	Etch_Animation *a;

	/* iterate over the list of animations */
	EINA_INLIST_FOREACH(e->animations, a)
	{
		etch_animation_process(a);
	}
}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
int etch_log_dom_global = -1;

void etch_animation_process(Etch_Animation *a)
{
	Etch_Time rcurr;
	Etch_Time atime; /* animation time */
	Etch_Time end;
	Etch_Time length;
	Etch *e;

	e = a->etch;
	/* TODO use e->start and e->end */
	DBG("[%" ETCH_TIME_FORMAT " %" ETCH_TIME_FORMAT "]"
			" %" ETCH_TIME_FORMAT \
			" %" ETCH_TIME_FORMAT \
			" %d",
			ETCH_TIME_ARGS (e->curr),
			ETCH_TIME_ARGS (a->offset),
			ETCH_TIME_ARGS (a->start),
			ETCH_TIME_ARGS (a->end),
			a->repeat);
	if (!a->enabled)
		return;
	/*  are we after the start ? */
	if (e->curr < a->start + a->offset)
		return;
	/* some sanity checks */
	if (!(a->end - a->start))
		return;

	/* check if we have finished */
	if (a->repeat < 0)
		goto infinite;
	/* are we before the end ? */
	end = (a->end * a->repeat);
	if (e->curr > end + a->offset)
	{
		if (a->started)
		{
			DBG("Stopping animation %p at %" ETCH_TIME_FORMAT
					" with end %" ETCH_TIME_FORMAT,
					a,
					ETCH_TIME_ARGS (e->curr),
					ETCH_TIME_ARGS (a->end));
			/* send the last tick that will trigger the animation
			 * for the end value
			 */
			etch_animation_animate(a, a->end);
			a->started = EINA_FALSE;
			if (a->stop_cb) a->stop_cb(a, a->data);
		}
		return;
	}
infinite:
	/* normalize to animation time */
	atime = e->curr - (a->start + a->offset);

	/* ok we are on the range */
	/* calculate the relative current time */
	length = a->end - a->start;
	rcurr = atime % length;
	rcurr += a->start;
	/* if the previous tick was outside the start-end
	 * means that we are going to repeat
	 */
	if (((rcurr - e->tpf) < a->start) && a->started)
	{
		DBG("Repeating animation %p", a);
		/* force it to pass through the last keyframe on the repeat */ 
		etch_animation_animate(a, a->end);
		if (a->repeat_cb) a->repeat_cb(a, a->data);
		return;
	}

	if (!a->started)
	{
		DBG("Starting animation %p", a);
		if (a->start_cb) a->start_cb(a, a->data);
		a->started = EINA_TRUE;
	}

	DBG("Animating at %" ETCH_TIME_FORMAT " for [%" 
			ETCH_TIME_FORMAT " %" ETCH_TIME_FORMAT "]",
			ETCH_TIME_ARGS (rcurr),
			ETCH_TIME_ARGS (a->start),
			ETCH_TIME_ARGS (a->end));
	etch_animation_animate(a, rcurr);
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * Initialize Etch. You must call this function before any other call to any
 * etch function.
 */
EAPI void etch_init(void)
{
	if (_init_count) goto done;
	eina_init();
	etch_log_dom_global = eina_log_domain_register("etch", ETCH_LOG_COLOR_DEFAULT);
	if (etch_log_dom_global < 0)
	{
		EINA_LOG_ERR("Etch: Can not create a general log domain.");
		goto shutdown_eina;
	}
done:
	_init_count++;
	return;

shutdown_eina:
	eina_shutdown();
}
/**
 * Shutdown Etch.
 */
EAPI void etch_shutdown(void)
{
	if (_init_count != 1) goto done;
	eina_log_domain_unregister(etch_log_dom_global);
        etch_log_dom_global = -1;
	eina_shutdown();
done:
	_init_count--;
}
/**
 * Create a new Etch instance.
 * @return The Etch instance
 */
EAPI Etch * etch_new(void)
{
	Etch *e;

	e = calloc(1, sizeof(Etch));
	etch_timer_fps_set(e, DEFAULT_FPS);
	return e;
}

/**
 * Delete the Etch instance
 * @param e The Etch instance
 */
EAPI void etch_delete(Etch *e)
{
	assert(e);
	/* remove every object */
	/* TODO remove every animation */
	free(e);
}
/**
 * Sets the frames per second
 * @param e The Etch instance
 * @param fps Frames per second
 */
EAPI void etch_timer_fps_set(Etch *e, unsigned int fps)
{
	double spf;

	assert(e);

	e->fps = fps;
	spf = (double)1.0/fps;
	e->tpf = spf * ETCH_SECOND;
}
/**
 * Sets the frames per second
 * @param e The Etch instance
 * @return Number of frames per second
 */
EAPI unsigned int etch_timer_fps_get(Etch *e)
{
	assert(e);
	return e->fps;
}
/**
 * Advance the global time by one unit of seconds per frame
 * @param e The Etch instance
 */
EAPI void etch_timer_tick(Etch *e)
{
	assert(e);
	/* TODO check for overflow */
	e->frame++;
	e->curr += e->tpf;
	_process(e);
}
/**
 * Query whenever all animations are done
 * @param e The Etch instance
 */
EAPI int etch_timer_has_end(Etch *e)
{
	/* we need a function to get the last frame/marker to know when the
	 * animations have finished, on the application we can make it run again,
	 * stop, whatever */

	return 0;
}
/**
 * Get the current global time of an Etch
 * @param e The Etch instance
 * @param t The time to get
 */
EAPI void etch_timer_get(Etch *e, Etch_Time *t)
{
	*t = e->curr;
}

/**
 * Set the current global time of an Etch
 * @param e The Etch instance
 * @param t The time to set
 */
EAPI void etch_timer_set(Etch *e, Etch_Time t)
{
	e->curr = t;
	_process(e);
}

/**
 * Move the Etch global time to the specific frame
 * @param e The Etch instance
 * @param frame Frame to go
 */
EAPI void etch_timer_goto(Etch *e, unsigned long frame)
{
	Etch_Time t;

	e->frame = frame;
	t = e->tpf * frame;
	e->curr = t;
	_process(e);
}
/**
 * Create a new animation
 * @param e The Etch instance to add the animation to
 * @param dtype Data type the animation will animate
 * @param cb Function called whenever the value changes
 * @param start Function called whenever the animation starts
 * @param stop Function called whenever the animation stops
 * @param data User provided data that passed to the callbacks
 */
EAPI Etch_Animation * etch_animation_add(Etch *e, Etch_Data_Type dtype,
		Etch_Animation_Callback cb,
		Etch_Animation_State_Callback start,
		Etch_Animation_State_Callback stop,
		Etch_Animation_State_Callback repeat,
		void *data)
{
	Etch_Animation *a;
	Etch_Interpolator interpolator;

	if (dtype >= ETCH_EXTERNAL)
		return NULL; 

	interpolator = _interpolators[dtype];
	a = etch_animation_new(e, dtype, interpolator, cb, start, stop, repeat, NULL, NULL, data);
	if (!a) return NULL;
	e->animations = eina_inlist_append(e->animations, EINA_INLIST_GET(a));

	return a;
}

/**
 * Create a new external animation
 * @param e The Etch instance to add the animation to
 * @param interpolator The interpolator to use
 * @param cb Function called whenever the value changes
 * @param start Function called whenever the animation starts
 * @param stop Function called whenever the animation stops
 * @param prev User provided data to be used on the prev parameter of the cb
 * @param curr User provided data to be used on the curr parameter of the cb
 * @param data User provided data that passed to the callbacks
 */
EAPI Etch_Animation * etch_animation_external_add(Etch *e,
		Etch_Interpolator interpolator,
		Etch_Animation_Callback cb,
		Etch_Animation_State_Callback start,
		Etch_Animation_State_Callback stop,
		Etch_Animation_State_Callback repeat,
		void *prev,
		void *current,
		void *data)
{
	Etch_Animation *a;
	a = etch_animation_new(e, ETCH_EXTERNAL, interpolator, cb, start, stop, repeat, prev, current, data);
	if (!a) return NULL;
	e->animations = eina_inlist_append(e->animations, EINA_INLIST_GET(a));

	return a;
}

/**
 * Remove the animation from the Etch instance
 * @param e The Etch instance to remove the animation from
 * @param a The animation to remove
 */
EAPI void etch_animation_remove(Etch *e, Etch_Animation *a)
{
	e->animations = eina_inlist_remove(e->animations, EINA_INLIST_GET(a));
}
