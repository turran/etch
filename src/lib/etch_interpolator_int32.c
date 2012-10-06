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
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
void etch_interpolator_int32(Etch_Data *da, Etch_Data *db, double m, Etch_Data *res)
{
	int32_t a, b;

	a = da->data.i32;
	b = db->data.i32;
	/* handle specific case where a and b are equal (constant) */
	if (a == b)
	{
		res->data.i32 = a;
		return;
	}
	etch_interpolate_int32(a, b, m, &(res->data.i32));
}
