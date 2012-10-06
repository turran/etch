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
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
void etch_interpolator_uint32(Etch_Data *da, Etch_Data *db, double m, Etch_Data *res)
{
	double r;
	uint32_t a, b;

	a = da->data.u32;
	b = db->data.u32;

	/* handle specific case where a and b are equal (constant) */
	if (a == b)
	{
		res->data.u32 = a;
		return;
	}
	r = ((1 - m) * a) + (m * b);
	res->data.u32 = ceil(r);
}
