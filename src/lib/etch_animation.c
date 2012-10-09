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
 * @todo
 * - make functions to interpolate between data types,
 * possible animations:
 * sin
 * exp
 * log
 * linear
 * bezier based (1 and 2 control points)
 * - make every interpolator work for every data type, so better a function table
 * - define animatinos based on two properties: PERIODIC, UNIQUE, PERIODIC_MIRROR
 * - the integer return values of the interpolators should be rounded?
 */
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
typedef double (*Etch_Animation_Interpolator_Calc)(double m, Etch_Interpolator_Type_Data *data);

typedef struct _Etch_Animation_Iterator
{
	Eina_Iterator iterator;
	Etch_Animation *a;
	Eina_Inlist *current;
} Etch_Animation_Iterator;

static void _keyframe_debug(Etch_Animation_Keyframe *k)
{
	DBG("Keyframe at %lld of type %d", k->time, k->type);
	switch (k->value.type)
	{
		case ETCH_UINT32:
		DBG("value = %u", k->value.data.u32);
		break;

		case ETCH_ARGB:
		DBG("value = 0x%8x", k->value.data.argb);
		break;

		case ETCH_STRING:
		DBG("value = %s", k->value.data.string);
		break;

		default:
		break;
	}
}

static void _animation_debug(Etch_Animation *a)
{
	Eina_Inlist *l;

	DBG("Animation that interpolates data of type %d, with the following keyframes:", a->dtype);
	l = (Eina_Inlist *)a->keys;
	while (l)
	{
		Etch_Animation_Keyframe *k = (Etch_Animation_Keyframe *)l;

		_keyframe_debug(k);
		l = l->next;
	}
}

/*----------------------------------------------------------------------------*
 *                               The calc types                               *
 *----------------------------------------------------------------------------*/
static double _calc_linear(double m, Etch_Interpolator_Type_Data *data)
{
	return m;
}

static double _calc_discrete(double m, Etch_Interpolator_Type_Data *data)
{
	return m < 1 ? 0 : 1;
}

static double _calc_cosin(double m, Etch_Interpolator_Type_Data *data)
{
	double m2;

	m2 = (1 - cos(m * M_PI))/2;
}

static double _calc_quadratic(double m, Etch_Interpolator_Type_Data *data)
{
	//res->data.d =  (1 - m) * (1 - m) * a + 2 * m * (1 - m) * (q->cp.data.d) + m * m * b;
	/* TODO */
	return m;
}

static double _calc_cubic(double m, Etch_Interpolator_Type_Data *data)
{
	/* TODO */
	return m;
}

static Etch_Animation_Interpolator_Calc _calcs[ETCH_INTERPOLATOR_TYPES] = {
	/* ETCH_INTERPOLATOR_DISCRETE 	*/ _calc_discrete,
	/* ETCH_INTERPOLATOR_LINEAR 	*/ _calc_linear,
	/* ETCH_INTERPOLATOR_COSIN 	*/ _calc_cosin,
	/* ETCH_INTERPOLATOR_QUADRATIC 	*/ _calc_quadratic,
	/* ETCH_INTERPOLATOR_CUBIC 	*/ _calc_cubic,
};
/*----------------------------------------------------------------------------*
 *                           The iterator interface                           *
 *----------------------------------------------------------------------------*/

static Eina_Bool _iterator_next(Etch_Animation_Iterator *it, void **data)
{
	if (!it->current) return EINA_FALSE;
	if (data) *data = (void*) it->current;

	it->current = it->current->next;

	return EINA_TRUE;
}

static void * _iterator_get_container(Etch_Animation_Iterator *it)
{
	return (void *)it->a;
}

static void _iterator_free(Etch_Animation_Iterator *it)
{
	free(it);
}

static void _update_start_end(Etch_Animation *a)
{
	Etch_Animation_Keyframe *start, *end;

	start = ((Etch_Animation_Keyframe *)a->keys);
	if (!start)
		return;
	end = ((Etch_Animation_Keyframe *)((Eina_Inlist *)(a->keys))->last);
	if (!end)
		return;

	a->start = start->time;
	a->end = end->time;
}

static void _keyframe_delete(Etch_Animation_Keyframe *k)
{
	if (k->data && k->data_free)
		k->data_free(k->data);
	free(k);
}

static void _keyframes_order(Etch_Animation *a, Etch_Animation_Keyframe *k)
{
	Eina_Inlist *l;
	Etch_Time t;

	t = k->time;
	/* find the greater element with the value less than the one to set */
	l = (Eina_Inlist *)(a->keys);
	while (l)
	{
		Etch_Animation_Keyframe *k2 = (Etch_Animation_Keyframe *)l;

		if (k2->time >= t)
			break;
		l = l->next;
	}
	/* if the element to remove is the same as the element to change, do
	 * nothing */
	if ((Etch_Animation_Keyframe*)l == k)
		goto update;
	a->keys = eina_inlist_remove(a->keys, EINA_INLIST_GET(k));
	/* k is the greatest */
	if (!l)
	{
		a->keys = eina_inlist_append(a->keys, EINA_INLIST_GET(k));
		/* TODO handle the iterator correctly */
	}
	/* k is between two keyframes */
	else
	{
		a->keys = eina_inlist_prepend_relative(a->keys, EINA_INLIST_GET(k), l);
		/* TODO handle the iterator correctly */
	}
	/* update the start and end values */
update:
	_update_start_end(a);
}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/**
 * To be documented
 * FIXME: To be fixed
 * This functions gets called on etch_process with curr time in an absolute
 * form, isnt better to pass a relative time (i.e relative to the start and end
 * of the animation) ?
 */
void etch_animation_animate(Etch_Animation *a, Etch_Time curr)
{
	Eina_Inlist *l;
	Etch_Animation_Keyframe *start;
	Etch_Animation_Keyframe *end;

	/* check that the time is between two keyframes */
	if (!a->keys)
		return;

	/* TODO instead of checking everytime every keyframe we can translate the
	 * keyframes based on the frame, when a keyframe has passed move it before
	 * like a circular list */
	l = (Eina_Inlist *)a->keys;
	while (l)
	{
		start = (Etch_Animation_Keyframe *)l;
		end = (Etch_Animation_Keyframe *)(l->next);
		if (!end)
			break;
		/* get the keyframe affected */
		//DBG("-> [%g] %g %g", curr, start->time, end->time);
		if ((start->time <= curr) && (curr <= end->time))
		{
			Etch_Data old;
			double m;
			void *data = NULL;

			/* get the interval between 0 and 1 based on current frame and two keyframes */
			if (curr == start->time)
				m = 0;
			else if (curr == end->time)
				m = 1;
			else
				m = (double)(curr - start->time)/(end->time - start->time);
			/* calc the new m */
			m = _calcs[start->type](m, &start->idata);
			/* accelerate the calculations if we get the same m as the previous call */
			if (m == a->m)
			{
				a->cb(start, &a->curr, &a->curr, a->data);
				return;
			}
			/* interpolate the value with the new m */
			a->interpolator(&(start->value), &(end->value), m, &a->curr, a->data);
			/* once the value has been set, call the callback */
			a->cb(start, &a->curr, &a->prev, a->data);
			/* swap the values */
			if (a->dtype == ETCH_EXTERNAL)
			{
				void *tmp;

				tmp = a->prev.data.external;
				if (tmp)
				{
					a->prev.data.external = a->curr.data.external;
					a->curr.data.external = tmp;
				}
			}
			else
			{
				a->prev = a->curr;
			}

			return;
		}
		l = l->next;
	}
}

/* TODO add the repeat callback with the an argument of the repeat count */
Etch_Animation * etch_animation_new(Etch *e,
		Etch_Data_Type dtype,
		Etch_Interpolator interpolator,
		Etch_Animation_Callback cb,
		Etch_Animation_State_Callback start,
		Etch_Animation_State_Callback stop,
		void *prev,
		void *curr,
		void *data)
{
	Etch_Animation *a;

	a = calloc(1, sizeof(Etch_Animation));
	/* common values */
	a->m = -1; /* impossible, so the first keyframe will overwrite this */
	a->start = UINT64_MAX;
	a->dtype = dtype;
	a->interpolator = interpolator;
	a->cb = cb;
	a->data = data;
	a->repeat = 1;
	a->etch = e;
	a->start_cb = start;
	a->stop_cb = stop;
	/* in case of external animations we need to keep
	 * the data
	 */
	a->prev.type = dtype;
	a->curr.type = dtype;
	if (dtype == ETCH_EXTERNAL)
	{
		a->prev.data.external = prev;
		a->curr.data.external = curr;
	}

	return a;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * Gets the current data value
 * @param a The Etch_Animation
 * @param v The data pointer to store the current value
 */
EAPI void etch_animation_data_get(Etch_Animation *a, Etch_Data *v)
{
	if (v) *v = a->curr;
}
/**
 * Deletes an animation
 * @param a The Etch_Animation
 */
EAPI void etch_animation_delete(Etch_Animation *a)
{
	Etch_Animation_Keyframe *k;
	Eina_Inlist *l2;

	assert(a);
	etch_animation_remove(a->etch, a);
	/* delete the list of keyframes */
	EINA_INLIST_FOREACH_SAFE(a->keys, l2, k)
	{
		a->keys = eina_inlist_remove(a->keys, EINA_INLIST_GET(k));
		_keyframe_delete(k);
	}
	free(a);
}

/**
 *
 */
EAPI Etch_Data_Type etch_animation_data_type_get(Etch_Animation *a)
{
	return a->dtype;
}

/**
 * Set the number of times the animation should repeat
 * @param a The Etch_Animation
 * @param times Number of times, -1 for infinite
 */
EAPI void etch_animation_repeat_set(Etch_Animation *a, unsigned int times)
{
	a->repeat = times;
}
/**
 * Add a new keyframe to the animation
 * @param a The Etch_Animation
 * @return The new Etch_Animation_Keyframe
 */
EAPI Etch_Animation_Keyframe * etch_animation_keyframe_add(Etch_Animation *a)
{
	Etch_Animation_Keyframe *k;

	assert(a);
	k = calloc(1, sizeof(Etch_Animation_Keyframe));
	k->animation = a;

	/* add the new keyframe to the list of keyframes */
	/* TODO we should always keep the animations ordered */
	a->keys = eina_inlist_append(a->keys, EINA_INLIST_GET(k));
	a->unordered = eina_list_append(a->unordered, k);
	a->count++;

	return k;
}
/**
 * Remove the keyframe from the animation
 * @param a The Etch_Animation
 * @param k The Etch_Animation_Keyframe
 */
EAPI void etch_animation_keyframe_remove(Etch_Animation *a, Etch_Animation_Keyframe *k)
{
	assert(a);
	assert(k);
	/* remove the keyframe from the list */
	a->keys = eina_inlist_remove(a->keys, EINA_INLIST_GET(k));
	a->unordered = eina_list_append(a->unordered, k);
	a->count--;
	/* TODO recalculate the start and end if necessary */
	_keyframe_delete(k);
}

/**
 *
 */
EAPI void etch_animation_keyframe_data_set(Etch_Animation_Keyframe *k, void *data, Etch_Free free)
{
	if (k->data && k->data_free)
		k->data_free(k->data);
	k->data = data;
	k->data_free = free;
}

/**
 *
 */
EAPI void * etch_animation_keyframe_data_get(Etch_Animation_Keyframe *k)
{
	return k->data;
}

/**
 * Get the number of keyframes an animation has
 * @param a The Etch_Animation
 */
EAPI int etch_animation_keyframe_count(Etch_Animation *a)
{
	return a->count;
}

/**
 *
 */
EAPI Etch_Animation_Keyframe * etch_animation_keyframe_get(Etch_Animation *a, unsigned int index)
{
	Etch_Animation_Keyframe *k;

	k = eina_list_nth(a->unordered, index);
	return k;
}


/**
 * Get the Etch instance this animation belongs to
 * @param a The Etch_Animation
 * @return The Etch instance
 */
EAPI Etch * etch_animation_etch_get(Etch_Animation *a)
{
	return a->etch;
}
/**
 * Disable an animation
 * @param a The Etch_Animation
 */
EAPI void etch_animation_disable(Etch_Animation *a)
{
	a->enabled = EINA_FALSE;
}
/**
 * Enable an animation
 * @param a The Etch_Animation
 */
EAPI void etch_animation_enable(Etch_Animation *a)
{
	a->enabled = EINA_TRUE;
}
/**
 * Query whenever an animation is atually enabled
 * @param a The Etch_Animation
 * @return EINA_TRUE or EINA_FALSE
 */
EAPI Eina_Bool etch_animation_enabled(Etch_Animation *a)
{
	return a->enabled;
}
/**
 * Add an offset to an animation. That will increment every animation's keyframe time
 * @param a The Etch_Animation
 * @param secs The number of seconds to increment
 * @param usecs The number of microsends to increment
 */
EAPI void etch_animation_offset_add(Etch_Animation *a, Etch_Time inc)
{
	assert(a);

	a->offset = inc;
}
/**
 * Set the type of an animation keyframe
 * @param k The Etch_Animation_Keyframe
 * @param t The type of the interpolation
 */
EAPI void etch_animation_keyframe_type_set(Etch_Animation_Keyframe *k, Etch_Interpolator_Type t)
{
	assert(k);
	k->type = t;
}
/**
 * Get the type of an animation keyframe
 * @param k The Etch_Animation_Keyframe
 */
EAPI Etch_Interpolator_Type etch_animation_keyframe_type_get(Etch_Animation_Keyframe *k)
{
	assert(k);
	return k->type;
}
/**
 * Get the time from a keyframe
 * @param k The Etch_Animation_Keyframe
 * @param secs The seconds
 * @param usecs The microseconds
 */
EAPI void etch_animation_keyframe_time_get(Etch_Animation_Keyframe *k, Etch_Time *t)
{
	*t = k->time;
}
/**
 * Set the time on a keyframe
 * @param k The Etch_Animation_Keyframe
 * @param secs The seconds
 * @param usecs The microseconds
 */
EAPI void etch_animation_keyframe_time_set(Etch_Animation_Keyframe *k, Etch_Time t)
{
	Etch_Animation *a;

	assert(k);

	/* if the time is the same, do nothing */
	if (k->time == t)
		return;
	a = k->animation;
	k->time = t;
	_keyframes_order(a, k);
}
/**
 * Get the value for a keyfame
 * @param k The Etch_Animation_Keyframe
 * @param v The Etch_Data to store the value to
 */
EAPI void etch_animation_keyframe_value_get(Etch_Animation_Keyframe *k, Etch_Data *v)
{
	assert(k);
	assert(v);

	*v = k->value;
}
/**
 * Set the value on a keyframe
 * @param k The Etch_Animation_Keyframe
 * @param v The Etch_Data to set the value from
 */
EAPI void etch_animation_keyframe_value_set(Etch_Animation_Keyframe *k, Etch_Data *v)
{

	assert(k);
	assert(v);

	k->value = *v;
}
/**
 * Sets the control point on a keyframe with a quadratic interpolation type
 * @param k The Etch_Animation_Keyframe
 * @param cp1 The value that defines the control point
 */
EAPI void etch_animation_keyframe_quadratic_value_set(Etch_Animation_Keyframe *k, double x0, double y0)
{
	k->idata.q.x0 = x0;
	k->idata.q.y0 = y0;
}
/**
 * Sets the control point on a keyframe with a cubic interpolation type
 * @param k The Etch_Animation_Keyframe
 * @param cp1 The value that defines the first control point
 * @param cp2 The value that defines the second control point
 */
EAPI void etch_animation_keyframe_cubic_value_set(Etch_Animation_Keyframe *k, double x0, double y0, double x1, double y1)
{
	k->idata.c.x0 = x0;
	k->idata.c.y0 = y0;
	k->idata.c.x1 = x1;
	k->idata.c.y1 = y1;
}

/**
 *
 */
EAPI Eina_Iterator * etch_animation_iterator_get(Etch_Animation *a)
{
	Etch_Animation_Iterator *it;

	it = calloc(1, sizeof (Etch_Animation_Iterator));
	if (!it) return NULL;

	it->a = a;
	it->current = a->keys;
	it->iterator.next = FUNC_ITERATOR_NEXT(_iterator_next);
	it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_iterator_get_container);
	it->iterator.free = FUNC_ITERATOR_FREE(_iterator_free);
	EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

	return &it->iterator;
}
