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

#include <Eina.h>
#include <math.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef ETCH_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

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
typedef int64_t Etch_Time; /**< Etch Time abstraction in nano seconds */

#define ETCH_SECOND (1000000000LL)
#define ETCH_MSECOND (1000000LL)

#define ETCH_TIME_FORMAT "u:%02u:%02u.%09u"
#define ETCH_TIME_ARGS(t) \
	(uint32_t) (((Etch_Time)(t)) / (ETCH_SECOND * 60 * 60)), \
	(uint32_t) ((((Etch_Time)(t)) / (ETCH_SECOND * 60)) % 60), \
	(uint32_t) ((((Etch_Time)(t)) / ETCH_SECOND) % 60), \
	(uint32_t) (((Etch_Time)(t)) % ETCH_SECOND)

EAPI Etch * etch_new(void);
EAPI void etch_delete(Etch *e);
EAPI void etch_timer_fps_set(Etch *e, unsigned int fps);
EAPI unsigned int etch_timer_fps_get(Etch *e);
EAPI void etch_timer_tick(Etch *e);
EAPI int etch_timer_has_end(Etch *e);
EAPI void etch_timer_goto(Etch *e, unsigned long frame);
EAPI void etch_timer_get(Etch *e, Etch_Time *t);

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
	ETCH_EXTERNAL, /**< External (user provided) type */
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
		void *external;
	} data;
} Etch_Data;

/**
 * @}
 * @defgroup Etch_Interpolators_Group Interpolators
 * @{
 */

/**
 * Possible interpolator types
 */
typedef enum _Etch_Interpolator_Type
{
	ETCH_INTERPOLATOR_DISCRETE, /**< The values are not interpolated, just discrete values */
	ETCH_INTERPOLATOR_LINEAR, /**< Linear interpolation */
	ETCH_INTERPOLATOR_COSIN, /***< Cosin interpolation */
	ETCH_INTERPOLATOR_QUADRATIC, /**< Quadratic bezier interpolation */
	ETCH_INTERPOLATOR_CUBIC, /**< Cubic bezier interpolation */
	ETCH_INTERPOLATOR_TYPES
} Etch_Interpolator_Type;

static inline void etch_interpolate_argb(uint32_t a, uint32_t b, double m, uint32_t *r)
{
	uint32_t range;
	uint32_t ag, rb;

	/* b - a*m + a */
	range = rint(256 * m);
	/* FIXME this can be optimized with MMX  or even use the libargb */
	ag = ((((((b >> 8) & 0xff00ff) - ((a >> 8) & 0xff00ff)) * range) + (a & 0xff00ff00)) & 0xff00ff00);
	rb = ((((((b & 0xff00ff) - (a & 0xff00ff)) * (range)) >> 8) + (a & 0xff00ff)) & 0xff00ff);

	*r = ag + rb;
}

static inline void etch_interpolate_double(double a, double b, double m, double *r)
{
	*r = ((1 - m) * a) + (m * b);
}

static inline void etch_interpolate_float(float a, float b, double m, float *r)
{
	*r = ((1 - m) * a) + (m * b);
}

static inline void etch_interpolate_int32(int32_t a, int32_t b, double m, int32_t *r)
{
	double rr;

	rr = ((1 - m) * a) + (m * b);
	*r = ceil(rr);
}

static inline void etch_interpolate_uint32(uint32_t a, uint32_t b, double m, uint32_t *r)
{
	double rr;

	rr = ((1 - m) * a) + (m * b);
	*r = ceil(rr);
}

typedef void (*Etch_Interpolator)(Etch_Data *a, Etch_Data *b, double m, Etch_Data *res, void *data);

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
 * Function definition for freeing user data
 */
typedef void (*Etch_Free)(void *data);


/**
 * Callback function used when a property value changes
 * @param k The current keyframe
 * @param curr The current value
 * @param prev The previous value
 * @param data User provided data
 */
typedef void (*Etch_Animation_Callback)(Etch_Animation_Keyframe *k, const Etch_Data *curr, const Etch_Data *prev, void *data);
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
		Etch_Animation_State_Callback repeat,
		void *data);
EAPI Etch_Animation * etch_animation_external_add(Etch *e,
		Etch_Interpolator interpolator,
		Etch_Animation_Callback cb,
		Etch_Animation_State_Callback start,
		Etch_Animation_State_Callback stop,
		Etch_Animation_State_Callback repeat,
		void *prev,
		void *current,
		void *data);
EAPI void etch_animation_remove(Etch *e, Etch_Animation *a);

EAPI void etch_animation_delete(Etch_Animation *a);
EAPI Etch_Data_Type etch_animation_data_type_get(Etch_Animation *a);
EAPI void etch_animation_disable(Etch_Animation *a);
EAPI void etch_animation_enable(Etch_Animation *a);
EAPI Eina_Bool etch_animation_enabled(Etch_Animation *a);
EAPI void etch_animation_offset_add(Etch_Animation *a, Etch_Time inc);
EAPI Etch * etch_animation_etch_get(Etch_Animation *a);
EAPI Eina_Iterator * etch_animation_iterator_get(Etch_Animation *a);
EAPI void etch_animation_data_get(Etch_Animation *a, Etch_Data *v);
EAPI void etch_animation_repeat_set(Etch_Animation *a, int times);
EAPI int etch_animation_keyframe_count(Etch_Animation *a);
EAPI Etch_Animation_Keyframe * etch_animation_keyframe_get(Etch_Animation *a, unsigned int index);

EAPI Etch_Animation_Keyframe * etch_animation_keyframe_add(Etch_Animation *a);
EAPI void etch_animation_keyframe_remove(Etch_Animation *a, Etch_Animation_Keyframe *m);
EAPI void etch_animation_keyframe_type_set(Etch_Animation_Keyframe *m, Etch_Interpolator_Type t);
EAPI Etch_Interpolator_Type etch_animation_keyframe_type_get(Etch_Animation_Keyframe *m);
EAPI void etch_animation_keyframe_data_set(Etch_Animation_Keyframe *k, void *data, Etch_Free free);
EAPI void * etch_animation_keyframe_data_get(Etch_Animation_Keyframe *k);
EAPI void etch_animation_keyframe_time_set(Etch_Animation_Keyframe *m, Etch_Time t);
EAPI void etch_animation_keyframe_time_get(Etch_Animation_Keyframe *k, Etch_Time *t);
EAPI void etch_animation_keyframe_value_set(Etch_Animation_Keyframe *k, Etch_Data *v);
EAPI void etch_animation_keyframe_value_get(Etch_Animation_Keyframe *k, Etch_Data *v);
EAPI void etch_animation_keyframe_cubic_value_set(Etch_Animation_Keyframe *k, double x0, double y0, double x1, double y1);
EAPI void etch_animation_keyframe_quadratic_value_set(Etch_Animation_Keyframe *k, double x0, double y0);
/**
 * @}
 */
#endif /*ETCH_H_*/
