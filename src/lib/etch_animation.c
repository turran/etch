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
extern Etch_Interpolator etch_interpolator_uint32;
extern Etch_Interpolator etch_interpolator_int32;
extern Etch_Interpolator etch_interpolator_argb;
extern Etch_Interpolator etch_interpolator_string;
extern Etch_Interpolator etch_interpolator_float;
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
typedef struct _Etch_Animation_Iterator
{
	Eina_Iterator iterator;
	Etch_Animation *a;
	Eina_Inlist *current;
} Etch_Animation_Iterator;

static void _keyframe_debug(Etch_Animation_Keyframe *k)
{
	printf("Keyframe at %ld.%ld of type %d\n", k->time.secs, k->time.usecs, k->type);
	switch (k->value.type)
	{
		case ETCH_UINT32:
		printf("value = %u\n", k->value.data.u32);
		break;

		case ETCH_ARGB:
		printf("value = 0x%8x\n", k->value.data.argb);
		break;

		case ETCH_STRING:
		printf("value = %s\n", k->value.data.string);
		break;

		default:
		break;
	}
}

static void _animation_debug(Etch_Animation *a)
{
	Eina_Inlist *l;

	printf("Animation that interpolates data of type %d, with the following keyframes:\n", a->dtype);
	l = (Eina_Inlist *)a->keys;
	while (l)
	{
		Etch_Animation_Keyframe *k = (Etch_Animation_Keyframe *)l;

		_keyframe_debug(k);
		l = l->next;
	}
}

Etch_Interpolator *_interpolators[ETCH_DATATYPES] = {
	[ETCH_UINT32] = &etch_interpolator_uint32,
	[ETCH_INT32] = &etch_interpolator_int32,
	[ETCH_ARGB] = &etch_interpolator_argb,
	[ETCH_STRING] = &etch_interpolator_string,
	[ETCH_FLOAT] = &etch_interpolator_float,
};

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
void etch_animation_animate(Etch_Animation *a, Etch_Time *curr)
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
		//printf("-> [%g] %g %g\n", curr, start->time, end->time);
		if (etch_time_between(&start->time, &end->time, curr) == EINA_TRUE)
		{
			Etch_Interpolator_Func ifnc;
			Etch_Data old;
			double m;
			void *data = NULL;

			/* get the interval between 0 and 1 based on current frame and two keyframes */
			if (etch_time_equal(&start->time, curr) == EINA_TRUE)
				m = 0;
			else if (etch_time_equal(&end->time, curr) == EINA_TRUE)
				m = 1;
			else
				m = etch_time_interpolate(&start->time, &end->time, curr);
			/* accelerate the calculations if we get the same m as the previous call */
			if (m == a->m)
			{
				a->cb(&a->curr, &a->curr, a->data);
				return;
			}
			/* store old value */
			old = a->curr;
			/* interpolate the new value */
			if (!_interpolators[a->dtype])
			{
				WRN("No interpolator available for type %d\n", a->dtype);
				return;
			}
			ifnc = _interpolators[a->dtype]->funcs[start->type];
			if (!ifnc)
				return;

			/* pass the specific data */
			switch (start->type)
			{
				case ETCH_ANIMATION_QUADRATIC:
				data = &start->q;
				break;

				case ETCH_ANIMATION_CUBIC:
				data = &start->c;
				break;

				default:
				break;
			}
			ifnc(&(start->value), &(end->value), m, &a->curr, &data);
			/* once the value has been set, call the callback */
			a->cb(&a->curr, &old, a->data);
			return;
		}
		l = l->next;
	}
}

Etch_Animation * etch_animation_new(Etch *e, Etch_Data_Type dtype,
		Etch_Animation_Callback cb,
		Etch_Animation_State_Callback start,
		Etch_Animation_State_Callback stop,
		void *data)
{
	Etch_Animation *a;

	a = calloc(1, sizeof(Etch_Animation));
	/* common values */
	a->m = -1; /* impossible, so the first keyframe will overwrite this */
	etch_time_init_max(&a->start);
	a->dtype = dtype;
	a->cb = cb;
	a->data = data;
	a->repeat = 1;
	a->etch = e;
	a->start_cb = start;
	a->stop_cb = stop;

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
	assert(a);
	/* TODO delete the list of keyframes */
	free(a);
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
	a->keys = eina_inlist_prepend(a->keys, EINA_INLIST_GET(k));
	a->count++;

	return k;
}
/**
 * Delete the keyframe from the animation
 * @param a The Etch_Animation
 * @param k The Etch_Animation_Keyframe
 */
EAPI void etch_animation_keyframe_del(Etch_Animation *a, Etch_Animation_Keyframe *k)
{
	assert(a);
	assert(k);
	/* remove the keyframe from the list */
	a->keys = eina_inlist_remove(a->keys, EINA_INLIST_GET(k));
	a->count--;
	/* TODO recalculate the start and end if necessary */
	free(k);
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
EAPI void etch_animation_offset_add(Etch_Animation *a, unsigned long secs, unsigned long usecs)
{
	Etch_Time inc;
	Eina_Inlist *l;

	assert(a);

	etch_time_secs_from(&inc, secs, usecs);
	l = (Eina_Inlist *)(a->keys);

	if (!l)
		return;
	/* increment every keyframe by secs.usecs */
	while (l)
	{
		unsigned long secs, usecs;
		Etch_Animation_Keyframe *k = (Etch_Animation_Keyframe *)l;

		etch_time_increment(&k->time, &inc);
		l = l->next;
	}
	_update_start_end(a);
}/**
 * Set the type of an animation keyframe
 * @param k The Etch_Animation_Keyframe
 * @param t The type of the interpolation
 */
EAPI void etch_animation_keyframe_type_set(Etch_Animation_Keyframe *k, Etch_Animation_Type t)
{
	assert(k);
	k->type = t;
}
/**
 * Get the type of an animation keyframe
 * @param k The Etch_Animation_Keyframe
 */
EAPI Etch_Animation_Type etch_animation_keyframe_type_get(Etch_Animation_Keyframe *k)
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
EAPI void etch_animation_keyframe_time_get(Etch_Animation_Keyframe *k, unsigned long *secs, unsigned long *usecs)
{
	etch_time_secs_to(k, secs, usecs);
}
/**
 * Set the time on a keyframe
 * @param k The Etch_Animation_Keyframe
 * @param secs The seconds
 * @param usecs The microseconds
 */
EAPI void etch_animation_keyframe_time_set(Etch_Animation_Keyframe *k, unsigned long secs, unsigned long usecs)
{
	Etch_Animation *a;
	Etch_Time t;
	Eina_Inlist *l;

	assert(k);

	etch_time_secs_from(&t, secs, usecs);
	/* if the time is the same, do nothing */
	if (etch_time_equal(&t, &k->time))
		return;
	a = k->animation;
	/* find the greater element with the value less than the one to set */
	l = (Eina_Inlist *)(a->keys);
	while (l)
	{
		Etch_Animation_Keyframe *k2 = (Etch_Animation_Keyframe *)l;

		if (etch_time_ge(&k2->time, &t) == EINA_TRUE)
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
	k->time = t;
	_update_start_end(a);
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
EAPI void etch_animation_keyframe_quadratic_value_set(Etch_Animation_Keyframe *k, Etch_Data *cp1)
{
	k->q.cp = *cp1;
}
/**
 * Sets the control point on a keyframe with a cubic interpolation type
 * @param k The Etch_Animation_Keyframe
 * @param cp1 The value that defines the first control point
 * @param cp2 The value that defines the second control point
 */
EAPI void etch_animation_keyframe_cubic_value_set(Etch_Animation_Keyframe *k, Etch_Data *cp1,
		Etch_Data *cp2)
{
	k->c.cp1 = *cp1;
	k->c.cp2 = *cp2;
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


