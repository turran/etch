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
/* this file was copied from uint32, does it make sense to have both ?
 */
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
static void _discrete(Etch_Data *da, Etch_Data *db, double m, Etch_Data *res, void *data)
{
	res->data.d = da->data.d;
}
static void _linear(Etch_Data *da, Etch_Data *db, double m, Etch_Data *res, void *data)
{
	double r;
	double a, b;

	a = da->data.d;
	b = db->data.d;
	/* handle specific case where a and b are equal (constant) */
	if (a == b)
	{
		res->data.d = a;
		return;
	}
	r = ((1 - m) * a) + (m * b);
	res->data.d = r;
}

static void _cosin(Etch_Data *da, Etch_Data *db, double m, Etch_Data *res, void *data)
{
	double m2;
	double a, b;

	a = da->data.d;
	b = db->data.d;

	m2 = (1 - cos(m * M_PI))/2;

	res->data.d = ((double)(a * (1 - m2) + b * m2));
}

static void _bquad(Etch_Data *da, Etch_Data *db, double m, Etch_Data *res, void *data)
{
	Etch_Animation_Quadratic *q = data;
	double a, b;

	a = da->data.d;
	b = db->data.d;

	res->data.d =  (1 - m) * (1 - m) * a + 2 * m * (1 - m) * (q->cp.data.d) + m * m * b;
}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
Etch_Interpolator etch_interpolator_double = {
	.funcs[ETCH_ANIMATION_DISCRETE] = _discrete,
	.funcs[ETCH_ANIMATION_LINEAR] = _linear,
	.funcs[ETCH_ANIMATION_COSIN] = _cosin,
	.funcs[ETCH_ANIMATION_QUADRATIC] = _bquad,
};