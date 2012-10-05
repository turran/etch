#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>

#ifdef _WIN32
# include <windows.h>
#endif

#include "Etch.h"

/* Helper utility to test properties, animations, etc */

#ifdef _WIN32
HANDLE timer = NULL;
#else
int _timer_event = 0;
#endif

static void _uint32_cb(Etch_Animation_Keyframe *k, const Etch_Data *curr, const Etch_Data *prev, void *data)
{
	printf("[UINT32] curr %d old %d\n", curr->data.u32, prev->data.u32);
}

static void _color_cb(Etch_Animation_Keyframe *k, const Etch_Data *curr, const Etch_Data *prev, void *data)
{
	printf("[ARGB] curr %08x old %08x\n", curr->data.argb, prev->data.argb);
}

static void _string_cb(Etch_Animation_Keyframe *k, const Etch_Data *curr, const Etch_Data *prev, void *data)
{
	printf("[STRING] curr %s old %s\n", curr->data.string, prev->data.string);
}


#ifdef _WIN32
static void timer_setup(void)
{
	LARGE_INTEGER time;

	time.QuadPart = 100000 / 3;
	timer = CreateWaitableTimer(NULL, TRUE, "WaitableTimer");
	SetWaitableTimer(timer, &time, 0, NULL, NULL, 0);
}
#else
/* Timer function */
static void timer_signal_cb(int s)
{
	_timer_event = 1;
}

static void timer_setup(void)
{
	struct sigaction sact;
	struct itimerval value;

	/* create the timer callback */
	sact.sa_flags = 0;
	sact.sa_handler = timer_signal_cb;

	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 33333; /* every 33333 (1/30fps) usecs */
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 500000; /* wait 500 usecs, before triggering the first event */
	sigaction(SIGALRM, &sact, NULL);
	setitimer(ITIMER_REAL, &value, NULL);
}
#endif

static void animation_uint32_setup(Etch *e)
{
	Etch_Animation *ea;
	Etch_Animation_Keyframe *ek;
	Etch_Data data;

	ea = etch_animation_add(e, ETCH_UINT32, _uint32_cb, NULL, NULL, NULL);
	/* first keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_COSIN);
	data.data.u32 = 10;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 3 * ETCH_SECOND + 3015 * ETCH_MSECOND);
	/* second keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_LINEAR);
	data.data.u32 = 40;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 25 * ETCH_SECOND + 1237 * ETCH_MSECOND);
	/* third keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_LINEAR);
	data.data.u32 = 30;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 15 * ETCH_SECOND + 2530 * ETCH_MSECOND);
	/* fourth keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_DISCRETE);
	data.data.u32 = 25;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 1 * ETCH_SECOND);
	/* fifth keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_DISCRETE);
	data.data.u32 = 15;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 2 * ETCH_SECOND);

	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_DISCRETE);
	data.data.u32 = 25;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 3 * ETCH_SECOND);
	etch_animation_repeat_set(ea, 2);

	etch_animation_enable(ea);
}

static void animation_argb_setup(Etch *e)
{
	Etch_Animation *ea;
	Etch_Animation_Keyframe *ek;
	Etch_Data data;

	ea = etch_animation_add(e, ETCH_ARGB, _color_cb, NULL, NULL, NULL);
	/* first keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_LINEAR);
	data.data.argb = 0xff000000;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 1 * ETCH_SECOND);
	/* second keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_LINEAR);
	data.data.argb = 0x00ff00ff;
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 5 * ETCH_SECOND);

	etch_animation_enable(ea);
}

static void animation_string_setup(Etch *e)
{
	Etch_Animation *ea;
	Etch_Animation_Keyframe *ek;
	Etch_Data data;

	ea = etch_animation_add(e, ETCH_STRING, _string_cb, NULL, NULL, NULL);
	/* first keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_DISCRETE);
	data.data.string = "hello";
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 1 * ETCH_SECOND);
	/* second keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_DISCRETE);
	data.data.string = "bye";
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 5 * ETCH_SECOND);
	/* third keyframe */
	ek = etch_animation_keyframe_add(ea);
	etch_animation_keyframe_type_set(ek, ETCH_INTERPOLATOR_DISCRETE);
	data.data.string = "nothing";
	etch_animation_keyframe_value_set(ek, &data);
	etch_animation_keyframe_time_set(ek, 8 * ETCH_SECOND);

	etch_animation_enable(ea);
}

int main(void)
{
	Etch *e;

	etch_init();

	e = etch_new();
	etch_timer_fps_set(e, 30);
	animation_uint32_setup(e);
	//animation_argb_setup(e);
	//animation_string_setup(e);
	timer_setup();
	/* to exit the main loop we should check that the etch animation has finished */
	while (!(etch_timer_has_end(e)))
	{
#ifdef _WIN32
		WaitForSingleObject(timer, INFINITE);
		/* send a tick to etch :) and wait for events */
		etch_timer_tick(e);
#else
		if (_timer_event)
		{
			/* send a tick to etch :) and wait for events */
			etch_timer_tick(e);
			_timer_event = 0;
		}
#endif
	}
	etch_delete(e);
	etch_shutdown();

	return 0;
}
