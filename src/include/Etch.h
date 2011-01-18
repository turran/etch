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
#ifndef ETCH_H_
#define ETCH_H_

#include "Eina.h"

/**
 * @mainpage Etch
 * @section intro Introduction
 * Etch is an abstract time based animation library. It animates values
 * of different types through different methods
 * @file
 * @brief Etch API
 * @defgroup Etch_Core_Group Core
 * The main object to interact with the Etch library is an Etch object.
 * An Etch object handles the global time and serves as a container
 * of animations.
 * @{
 */
EAPI void etch_init(void);
EAPI void etch_shutdown(void);

typedef struct _Etch Etch; /**< Etch Opaque Handler */

EAPI Etch * etch_new(void);
EAPI void etch_delete(Etch *e);
EAPI void etch_timer_fps_set(Etch *e, unsigned int fps);
EAPI unsigned int etch_timer_fps_get(Etch *e);
EAPI void etch_timer_tick(Etch *e);
EAPI int etch_timer_has_end(Etch *e);
EAPI void etch_timer_goto(Etch *e, unsigned long frame);
EAPI void etch_timer_get(Etch *e, unsigned long *secs, unsigned long *usecs);

/**
 * Data types for a property
 * TODO add fixed point types too
 */
typedef enum _Etch_Data_Type
{
	ETCH_UINT32, /**< Unsigned integer of 32 bits */
	ETCH_INT32, /**< Signed integer of 32 bits */
	ETCH_FLOAT, /**< Single precision float */
	ETCH_DOUBLE, /**< Double precision float */
	ETCH_ARGB, /**< Color (Alpha, Red, Green, Blue) of 32 bits */
	ETCH_STRING, /**< String type */
	ETCH_DATATYPES, /**< Number of data types */
} Etch_Data_Type;

/**
 * Container of every property data type supported
 */
typedef struct _Etch_Data
{
	Etch_Data_Type type;
	union {
		uint32_t u32;
		int32_t i32;
		float f;
		double d;
		unsigned int argb;
		char *string;
	} data;
} Etch_Data;

/**
 * @}
 * @defgroup Etch_Animations_Group Animations
 * Each animation on Etch is done through an Etch_Animation object.
 * An Etch_Animation can only animate one value type, that is,
 * only an integer type, a float type, etc. Whenever an animation
 * is started, stopped or the value of the animation changes a
 * callback will be called.
 * An Etch_Animation is a container of keyframes which are named
 * Etch_Keyframes, every keyframe defines the way to interpolate
 * two values, the previous keyframe final value and the current
 * keyframe value.
 * @{
 */
typedef struct _Etch_Animation Etch_Animation; /**< Animation Opaque Handler */
typedef struct _Etch_Animation_Keyframe Etch_Animation_Keyframe; /**< Animation Keyframe Opaque Handler */

/**
 * Possible animation types
 */
typedef enum _Etch_Animation_Type
{
	ETCH_ANIMATION_DISCRETE, /**< The values are not interpolated, just discrete values */
	ETCH_ANIMATION_LINEAR, /**< Linear interpolation */
	ETCH_ANIMATION_COSIN, /***< Cosin interpolation */
	ETCH_ANIMATION_QUADRATIC, /**< Quadratic bezier interpolation */
	ETCH_ANIMATION_CUBIC, /**< Cubic bezier interpolation */
	ETCH_ANIMATION_TYPES
} Etch_Animation_Type;

/**
 * Callback function used when a property value changes
 * @param curr The current value
 * @param prev The previous value
 * @param data User provided data
 */
typedef void (*Etch_Animation_Callback)(const Etch_Data *curr, const Etch_Data *prev, void *data);
/**
 * Callback function used when an animation is started or stopped
 * @param a The animation that triggered the event
 * @param data User provided data
 */
typedef void (*Etch_Animation_State_Callback)(Etch_Animation *a, void *data);

EAPI Etch_Animation * etch_animation_add(Etch *e, Etch_Data_Type dtype,
		Etch_Animation_Callback cb,
		Etch_Animation_State_Callback start,
		Etch_Animation_State_Callback stop,
		void *data);
EAPI void etch_animation_delete(Etch_Animation *a);
EAPI void etch_animation_disable(Etch_Animation *a);
EAPI void etch_animation_enable(Etch_Animation *a);
EAPI Eina_Bool etch_animation_enabled(Etch_Animation *a);
EAPI Etch * etch_animation_etch_get(Etch_Animation *a);
EAPI Eina_Iterator * etch_animation_iterator_get(Etch_Animation *a);
EAPI void etch_animation_data_get(Etch_Animation *a, Etch_Data *v);
EAPI void etch_animation_repeat_set(Etch_Animation *a, unsigned int times);
EAPI int etch_animation_keyframe_count(Etch_Animation *a);
EAPI Etch_Animation_Keyframe * etch_animation_keyframe_add(Etch_Animation *a);
EAPI void etch_animation_keyframe_del(Etch_Animation *a, Etch_Animation_Keyframe *m);
EAPI void etch_animation_keyframe_type_set(Etch_Animation_Keyframe *m, Etch_Animation_Type t);
EAPI Etch_Animation_Type etch_animation_keyframe_type_get(Etch_Animation_Keyframe *m);
EAPI void etch_animation_keyframe_time_set(Etch_Animation_Keyframe *m, unsigned long secs, unsigned long usecs);
EAPI void etch_animation_keyframe_time_get(Etch_Animation_Keyframe *k, unsigned long *secs, unsigned long *usecs);
EAPI void etch_animation_keyframe_value_set(Etch_Animation_Keyframe *k, Etch_Data *v);
EAPI void etch_animation_keyframe_value_get(Etch_Animation_Keyframe *k, Etch_Data *v);
EAPI void etch_animation_keyframe_cubic_value_set(Etch_Animation_Keyframe *k, Etch_Data *cp1,
		Etch_Data *cp2);
EAPI void etch_animation_keyframe_quadratic_value_set(Etch_Animation_Keyframe *k, Etch_Data *cp1);
/**
 * @}
 */
#endif /*ETCH_H_*/
