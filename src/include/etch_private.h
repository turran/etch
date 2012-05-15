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
#ifndef ETCH_PRIVATE_H_
#define ETCH_PRIVATE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdarg.h>
#include <math.h>
#include <sys/time.h>
#include <limits.h>

#include <assert.h>

#include "Eina.h"

#define ERR(...) EINA_LOG_DOM_ERR(etch_log, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(etch_log, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(etch_log, __VA_ARGS__)

extern int etch_log;

/**
 *
 */
struct _Etch
{
	Eina_Inlist *animations; /** List of objects */
	unsigned long frame; /** Current frame */
	unsigned int fps; /** Number of frames per second */
	Etch_Time tpf; /** Time per frame */
	Etch_Time curr; /** Current time in seconds */
	/* TODO do we need to cache the next animation to be animated here? */
};

/**
 * Specific data needed for cubic bezier animations
 */
typedef struct _Etch_Animation_Cubic
{
	Etch_Data cp1; /** First control point */
	Etch_Data cp2; /** Second control point */
} Etch_Animation_Cubic;

/**
 * Specific data needed for quadratic bezier animations
 */
typedef struct _Etch_Animation_Quadratic
{
	Etch_Data cp; /** Control point */
} Etch_Animation_Quadratic;

/**
 * An animation mark is a defined state on the timeline of an animation. It sets
 * that a given time a property should have the specified value.
 */
struct _Etch_Animation_Keyframe
{
	EINA_INLIST; /** A keyframe is always a list */
	Etch_Animation *animation; /** reference to the animation */
	Etch_Data value; /** the property value for this mark */
	Etch_Time time; /** the time where the keyframe is */
	/* TODO do we need to cache the diff between this keyframe and the next?
	 * to avoid substracting them every time?
	 */
	Etch_Animation_Type type; /** type of interpolation between this mark and the next */
	Etch_Animation_Quadratic q; /** quadratic interpolation specific data */
	Etch_Animation_Cubic c; /** cubic interpolation specific data */
	void *data;
	Etch_Free data_free;
};

/**
 * Many objects can use the same animation.
 */
struct _Etch_Animation
{
	EINA_INLIST; /** An animation is a list */
	Eina_Inlist *keys; /** list of keyframes ordered */
	/* we can not iterate through the keys and modify the time given that inmediately
	 * the keys will be ordered */
	Eina_List *unordered; /** list of keyframes unordered */
	Etch *etch; /** Etch having this animation */
	/* TODO if the marks are already ordered do we need to have the start
	 * and end time duplicated here? */
	Etch_Time start; /** initial time */
	Etch_Time end; /** end time already */
	/* TODO make m a fixed point var of type 1.31 */
	double m; /** last interpolator value in the range [0,1] */
	Etch_Data curr; /** current value in the whole animation */
	unsigned int repeat; /** number of times the animation will repeat, 0 for infinite */
	Etch_Data_Type dtype; /** animations only animates data types, no properties */
	Etch_Animation_Callback cb; /** function to call when a value has been set */
	Etch_Animation_State_Callback start_cb, stop_cb;
	void *data; /** user provided data */
	int count; /** number of keyframes this animation has */
	Eina_Bool enabled;/** easy way to disable/enable an animation */
	Eina_Bool started;
	Etch_Time offset; /*  the real offset */
};

typedef void (*Etch_Interpolator_Func)(Etch_Data *a, Etch_Data *b, double m, Etch_Data *res, void *data);
typedef struct _Etch_Interpolator
{
	Etch_Interpolator_Func funcs[ETCH_ANIMATION_TYPES];
} Etch_Interpolator;

void etch_animation_animate(Etch_Animation *a, Etch_Time curr);
Etch_Animation * etch_animation_new(Etch *e, Etch_Data_Type dtype, Etch_Animation_Callback cb, Etch_Animation_State_Callback start, Etch_Animation_State_Callback stop, void *data);

#endif /*ETCH_PRIVATE_H_*/
