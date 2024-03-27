#include "GreyBitSystem.h"
#include "GreyBitType_Def.h"
#include "GreyBitRaster.h"

#ifdef ENABLE_GREYVECTORFILE
typedef struct TBand_
{
	int min;
	int max;
} TBand, *PBand;

#define RAS_ARG   PWorker  worker
#define RAS_ARG_  PWorker  worker,
#define RAS_VAR   worker
#define RAS_VAR_  worker,

#define PIXEL_BITS  8

#undef FLOOR
#undef CEILING
#undef TRUNC
#undef SCALED

#define ONE_PIXEL       ( 1L << PIXEL_BITS )
#define PIXEL_MASK      ( -1L << PIXEL_BITS )
#define TRUNC( x )      ( (TCoord)( (x) >> PIXEL_BITS ) )
#define SUBPIXELS( x )  ( (TPos)(x) << PIXEL_BITS )
#define FLOOR( x )      ( (x) & -ONE_PIXEL )
#define CEILING( x )    ( ( (x) + ONE_PIXEL - 1 ) & -ONE_PIXEL )
#define ROUND( x )      ( ( (x) + ONE_PIXEL / 2 ) & -ONE_PIXEL )
#define GB_ABS( a )     ( (a) < 0 ? -(a) : (a) )

#if PIXEL_BITS >= 6
#define UPSCALE( x )    ( (x) << ( PIXEL_BITS - 6 ) )
#define DOWNSCALE( x )  ( (x) >> ( PIXEL_BITS - 6 ) )
#else
#define UPSCALE( x )    ( (x) >> ( 6 - PIXEL_BITS ) )
#define DOWNSCALE( x )  ( (x) << ( 6 - PIXEL_BITS ) )
#endif

#define GB_BEGIN_STMNT  do {
#define GB_END_STMNT    } while ( 0 )
#define GB_DUMMY_STMNT  GB_BEGIN_STMNT GB_END_STMNT

#define GB_DIV_MOD( type, dividend, divisor, quotient, remainder ) \
  GB_BEGIN_STMNT                                                   \
    (quotient)  = (type)( (dividend) / (divisor) );                \
    (remainder) = (type)( (dividend) % (divisor) );                \
    if ( (remainder) < 0 )                                         \
    {                                                              \
      (quotient)--;                                                \
      (remainder) += (type)(divisor);                              \
    }                                                              \
  GB_END_STMNT

#ifdef __arm__
#undef GB_DIV_MOD
#define GB_DIV_MOD( type, dividend, divisor, quotient, remainder ) \
  GB_BEGIN_STMNT                                                   \
    (quotient)  = (type)( (dividend) / (divisor) );                \
    (remainder) = (type)( (dividend) - (quotient) * (divisor) );   \
    if ( (remainder) < 0 )                                         \
    {                                                              \
      (quotient)--;                                                \
      (remainder) += (type)(divisor);                              \
    }                                                              \
  GB_END_STMNT
#endif /* __arm__ */

typedef long  TCoord;   /* integer scanline/pixel coordinate */
typedef long  TPos;     /* sub-pixel coordinate              */

/* determine the type used to store cell areas.  This normally takes at */
/* least PIXEL_BITS*2 + 1 bits.  On 16-bit systems, we need to use      */
/* `long' instead of `int', otherwise bad things happen                 */

#if PIXEL_BITS <= 7

typedef int  TArea;

#else /* PIXEL_BITS >= 8 */

  /* approximately determine the size of integers using an ANSI-C header */
#if GB_UINT_MAX == 0xFFFFU
typedef long  TArea;
#else
typedef int   TArea;
#endif

#endif /* PIXEL_BITS >= 8 */

#define GB_MAX_GRAY_SPANS 32

typedef struct TCell_*  PCell;

typedef struct  TCell_
{
	TPos    x;     /* same with TWorker.ex    */
	TCoord  cover; /* same with TWorker.cover */
	TArea   area;
	PCell   next;

} TCell;

typedef struct  TWorker_
{
	TCoord  ex, ey;
	TPos    min_ex, max_ex;
	TPos    min_ey, max_ey;
	TPos    count_ex, count_ey;

	TArea   area;
	TCoord  cover;
	int     invalid;

	PCell       cells;
	GB_INT32  max_cells;
	GB_INT32  num_cells;

	TCoord  cx, cy;
	TPos    x, y;

	TPos    last_ey;

	RST_Vector  bez_stack[32 * 3 + 1];
	int         lev_stack[32];

	GB_OutlineRec  outline;
	GB_BitmapRec   target;
	GB_BBox     clip_box;

	GB_Span     gray_spans[GB_MAX_GRAY_SPANS];
	int         num_gray_spans;

	GB_Raster_Span_Func  render_span;
	void*                render_span_data;
	int                  span_y;

	int  band_size;
	int  band_shoot;

	int  conic_level;
	int  cubic_level;

	void*       buffer;
	long        buffer_size;

	PCell*     ycells;
	TPos       ycount;

} TWorker, *PWorker;

typedef struct TRaster_
{
	GB_Memory gbMem;
	void *buffer;
	long buffer_size;
	int band_size;
	PWorker worker;
}TRaster, *PRaster;

#define ras  (*worker)

void gray_init_cells(RAS_ARG_ void *buffer, int byte_size)
{
	worker->buffer = buffer;
	worker->buffer_size = byte_size;

	worker->ycells = (PCell*) buffer;
	worker->cells = 0;
	worker->max_cells = 0;
	worker->num_cells = 0;
	worker->area = 0;
	worker->cover = 0;
	worker->invalid = 1;
}

void gray_compute_cbox(RAS_ARG)
{
	GB_Outline  outline = &ras.outline;
	GB_Vector*   vec = outline->points;
	GB_Vector*   limit = vec + outline->n_points;


	if (outline->n_points <= 0)
	{
		ras.min_ex = ras.max_ex = 0;
		ras.min_ey = ras.max_ey = 0;
		return;
	}

	ras.min_ex = ras.max_ex = vec->x;
	ras.min_ey = ras.max_ey = vec->y;

	vec++;

	for (; vec < limit; vec++)
	{
		TPos  x = vec->x;
		TPos  y = vec->y;


		if (x < ras.min_ex) ras.min_ex = x;
		if (x > ras.max_ex) ras.max_ex = x;
		if (y < ras.min_ey) ras.min_ey = y;
		if (y > ras.max_ey) ras.max_ey = y;
	}

	/* truncate the bounding box to integer pixels */
	ras.min_ex = ras.min_ex >> 6;
	ras.min_ey = ras.min_ey >> 6;
	ras.max_ex = (ras.max_ex + 63) >> 6;
	ras.max_ey = (ras.max_ey + 63) >> 6;
}

PCell gray_find_cell(RAS_ARG_ int *ret)
{
	PCell  *pcell, cell;
	TPos    x = ras.ex;


	if (x > ras.count_ex)
		x = ras.count_ex;

	pcell = &ras.ycells[ras.ey];
	for (;;)
	{
		cell = *pcell;
		if (!cell || cell->x > x)
			break;

		if (cell->x == x)
			goto Exit;

		pcell = &cell->next;
	}

	if (ras.num_cells >= ras.max_cells)
	{
		*ret = -4;
		return cell;
	}

	cell = ras.cells + ras.num_cells++;
	cell->x = x;
	cell->area = 0;
	cell->cover = 0;

	cell->next = *pcell;
	*pcell = cell;

Exit:
	*ret = 0;
	return cell;
}

int gray_record_cell(RAS_ARG)
{
	int ret = 0;
	if (!ras.invalid && ras.area | ras.cover)
	{
		PCell cell = gray_find_cell(RAS_VAR_ &ret);
		if (ret)
			return ret;

		cell->area += ras.area;
		cell->cover += ras.cover;
	}
	return GB_SUCCESS;
}

void gray_set_cell(RAS_ARG_ TCoord ex, TCoord ey)
{
	/* Move the cell pointer to a new position.  We set the `invalid'      */
	/* flag to indicate that the cell isn't part of those we're interested */
	/* in during the render phase.  This means that:                       */
	/*                                                                     */
	/* . the new vertical position must be within min_ey..max_ey-1.        */
	/* . the new horizontal position must be strictly less than max_ex     */
	/*                                                                     */
	/* Note that if a cell is to the left of the clipping region, it is    */
	/* actually set to the (min_ex-1) horizontal position.                 */

	/* All cells that are on the left of the clipping region go to the */
	/* min_ex - 1 horizontal position.                                 */
	ey -= ras.min_ey;

	if (ex > ras.max_ex)
		ex = ras.max_ex;

	ex -= ras.min_ex;
	if (ex < 0)
		ex = -1;

	/* are we moving to a different cell ? */
	if (ex != ras.ex || ey != ras.ey)
	{
		/* record the current one if it is valid */
		if (!ras.invalid)
			gray_record_cell(RAS_VAR);

		ras.area = 0;
		ras.cover = 0;
		ras.ex = ex;
		ras.ey = ey;
	}

	ras.invalid = ((unsigned)ey >= (unsigned)ras.count_ey ||
		ex >= ras.count_ex);
}

void gray_start_cell(RAS_ARG_ TCoord ex, TCoord ey)
{
	if (ex > ras.max_ex)
		ex = (TCoord)(ras.max_ex);

	if (ex < ras.min_ex)
		ex = (TCoord)(ras.min_ex - 1);

	ras.area = 0;
	ras.cover = 0;
	ras.ex = ex - ras.min_ex;
	ras.ey = ey - ras.min_ey;
	ras.last_ey = SUBPIXELS(ey);
	ras.invalid = 0;

	gray_set_cell(RAS_VAR_ ex, ey);
}

void gray_render_scanline(RAS_ARG_ TCoord ey, TPos x1, TCoord y1, TPos x2, TCoord y2)
{
	TCoord  ex1, ex2, fx1, fx2, delta, mod;
	long    p, first, dx;
	int     incr;


	dx = x2 - x1;

	ex1 = TRUNC(x1);
	ex2 = TRUNC(x2);
	fx1 = (TCoord)(x1 - SUBPIXELS(ex1));
	fx2 = (TCoord)(x2 - SUBPIXELS(ex2));

	/* trivial case.  Happens often */
	if (y1 == y2)
	{
		gray_set_cell(RAS_VAR_ ex2, ey);
		return;
	}

	/* everything is located in a single cell.  That is easy! */
	/*                                                        */
	if (ex1 == ex2)
	{
		delta = y2 - y1;
		ras.area += (TArea)((fx1 + fx2) * delta);
		ras.cover += delta;
		return;
	}

	/* ok, we'll have to render a run of adjacent cells on the same */
	/* scanline...                                                  */
	/*                                                              */
	p = (ONE_PIXEL - fx1) * (y2 - y1);
	first = ONE_PIXEL;
	incr = 1;

	if (dx < 0)
	{
		p = fx1 * (y2 - y1);
		first = 0;
		incr = -1;
		dx = -dx;
	}

	GB_DIV_MOD(TCoord, p, dx, delta, mod);

	ras.area += (TArea)((fx1 + first) * delta);
	ras.cover += delta;

	ex1 += incr;
	gray_set_cell(RAS_VAR_ ex1, ey);
	y1 += delta;

	if (ex1 != ex2)
	{
		TCoord  lift, rem;


		p = ONE_PIXEL * (y2 - y1 + delta);
		GB_DIV_MOD(TCoord, p, dx, lift, rem);

		mod -= (int)dx;

		while (ex1 != ex2)
		{
			delta = lift;
			mod += rem;
			if (mod >= 0)
			{
				mod -= (TCoord)dx;
				delta++;
			}

			ras.area += (TArea)(ONE_PIXEL * delta);
			ras.cover += delta;
			y1 += delta;
			ex1 += incr;
			gray_set_cell(RAS_VAR_ ex1, ey);
		}
	}

	delta = y2 - y1;
	ras.area += (TArea)((fx2 + ONE_PIXEL - first) * delta);
	ras.cover += delta;
}

void gray_render_line(RAS_ARG_ TPos to_x, TPos to_y)
{
	TCoord  ey1, ey2, fy1, fy2, mod;
	TPos    dx, dy, x, x2;
	long    p, first;
	int     delta, rem, lift, incr;


	ey1 = TRUNC(ras.last_ey);
	ey2 = TRUNC(to_y);     /* if (ey2 >= ras.max_ey) ey2 = ras.max_ey-1; */
	fy1 = (TCoord)(ras.y - ras.last_ey);
	fy2 = (TCoord)(to_y - SUBPIXELS(ey2));

	dx = to_x - ras.x;
	dy = to_y - ras.y;

	/* perform vertical clipping */
	{
		TCoord  min, max;


		min = ey1;
		max = ey2;
		if (ey1 > ey2)
		{
			min = ey2;
			max = ey1;
		}
		if (min >= ras.max_ey || max < ras.min_ey)
			goto End;
	}

	/* everything is on a single scanline */
	if (ey1 == ey2)
	{
		gray_render_scanline(RAS_VAR_ ey1, ras.x, fy1, to_x, fy2);
		goto End;
	}

	/* vertical line - avoid calling gray_render_scanline */
	incr = 1;

	if (dx == 0)
	{
		TCoord  ex = TRUNC(ras.x);
		TCoord  two_fx = (TCoord)((ras.x - SUBPIXELS(ex)) << 1);
		TArea   area;


		first = ONE_PIXEL;
		if (dy < 0)
		{
			first = 0;
			incr = -1;
		}

		delta = (int)(first - fy1);
		ras.area += (TArea)two_fx * delta;
		ras.cover += delta;
		ey1 += incr;

		gray_set_cell(RAS_VAR_ ex, ey1);

		delta = (int)(first + first - ONE_PIXEL);
		area = (TArea)two_fx * delta;
		while (ey1 != ey2)
		{
			ras.area += area;
			ras.cover += delta;
			ey1 += incr;

			gray_set_cell(RAS_VAR_ ex, ey1);
		}

		delta = (int)(fy2 - ONE_PIXEL + first);
		ras.area += (TArea)two_fx * delta;
		ras.cover += delta;

		goto End;
	}

	/* ok, we have to render several scanlines */
	p = (ONE_PIXEL - fy1) * dx;
	first = ONE_PIXEL;
	incr = 1;

	if (dy < 0)
	{
		p = fy1 * dx;
		first = 0;
		incr = -1;
		dy = -dy;
	}

	GB_DIV_MOD(int, p, dy, delta, mod);

	x = ras.x + delta;
	gray_render_scanline(RAS_VAR_ ey1, ras.x, fy1, x, (TCoord)first);

	ey1 += incr;
	gray_set_cell(RAS_VAR_ TRUNC(x), ey1);

	if (ey1 != ey2)
	{
		p = ONE_PIXEL * dx;
		GB_DIV_MOD(int, p, dy, lift, rem);
		mod -= (int)dy;

		while (ey1 != ey2)
		{
			delta = lift;
			mod += rem;
			if (mod >= 0)
			{
				mod -= (int)dy;
				delta++;
			}

			x2 = x + delta;
			gray_render_scanline(RAS_VAR_ ey1, x,
				(TCoord)(ONE_PIXEL - first), x2,
				(TCoord)first);
			x = x2;

			ey1 += incr;
			gray_set_cell(RAS_VAR_ TRUNC(x), ey1);
		}
	}

	gray_render_scanline(RAS_VAR_ ey1, x,
		(TCoord)(ONE_PIXEL - first), to_x,
		fy2);

End:
	ras.x = to_x;
	ras.y = to_y;
	ras.last_ey = SUBPIXELS(ey2);
}

void gray_split_conic(RST_Vector* base)
{
	GB_Pos  a, b;


	base[4].x = base[2].x;
	b = base[1].x;
	a = base[3].x = (base[2].x + b) / 2;
	b = base[1].x = (base[0].x + b) / 2;
	base[2].x = (a + b) / 2;

	base[4].y = base[2].y;
	b = base[1].y;
	a = base[3].y = (base[2].y + b) / 2;
	b = base[1].y = (base[2].y + b) / 2;
	base[2].y = (a + b) / 2;
}

void gray_render_conic(RAS_ARG_ const GB_Vector* control, const GB_Vector* to)
{
	GB_Pos      dx, dy;
	GB_Pos      min, max, y;
	int         top, level;
	int*        levels;
	RST_Vector* arc;


	levels = ras.lev_stack;

	arc = ras.bez_stack;
	arc[0].x = UPSCALE(to->x);
	arc[0].y = UPSCALE(to->y);
	arc[1].x = UPSCALE(control->x);
	arc[1].y = UPSCALE(control->y);
	arc[2].x = ras.x;
	arc[2].y = ras.y;
	top = 0;

	dx = GB_ABS(arc[2].x + arc[0].x - 2 * arc[1].x);
	dy = GB_ABS(arc[2].y + arc[0].y - 2 * arc[1].y);
	if (dx < dy)
		dx = dy;

	if (dx < ONE_PIXEL / 4)
		goto Draw;

	/* short-cut the arc that crosses the current band */
	min = max = arc[0].y;

	y = arc[1].y;
	if (y < min) min = y;
	if (y > max) max = y;

	y = arc[2].y;
	if (y < min) min = y;
	if (y > max) max = y;

	if (TRUNC(min) >= ras.max_ey || TRUNC(max) < ras.min_ey)
		goto Draw;

	level = 0;
	do
	{
		dx >>= 2;
		level++;
	} while (dx > ONE_PIXEL / 4);

	levels[0] = level;

	do
	{
		level = levels[top];
		if (level > 0)
		{
			gray_split_conic(arc);
			arc += 2;
			top++;
			levels[top] = levels[top - 1] = level - 1;
			continue;
		}

	Draw:
		gray_render_line(RAS_VAR_ arc[0].x, arc[0].y);
		top--;
		arc -= 2;

	} while (top >= 0);
}

void gray_split_cubic(RST_Vector* base)
{
	GB_Pos  a, b, c, d;


	base[6].x = base[3].x;
	c = base[1].x;
	d = base[2].x;
	base[1].x = a = (base[0].x + c) / 2;
	base[5].x = b = (base[3].x + d) / 2;
	c = (c + d) / 2;
	base[2].x = a = (a + c) / 2;
	base[4].x = b = (b + c) / 2;
	base[3].x = (a + b) / 2;

	base[6].y = base[3].y;
	c = base[1].y;
	d = base[2].y;
	base[1].y = a = (base[0].y + c) / 2;
	base[5].y = b = (base[3].y + d) / 2;
	c = (c + d) / 2;
	base[2].y = a = (a + c) / 2;
	base[4].y = b = (b + c) / 2;
	base[3].y = (a + b) / 2;
}

void gray_render_cubic(RAS_ARG_ const GB_Vector* control1, const GB_Vector* control2, const GB_Vector* to)
{
	RST_Vector*  arc;
	GB_Pos       min, max, y;


	arc = ras.bez_stack;
	arc[0].x = UPSCALE(to->x);
	arc[0].y = UPSCALE(to->y);
	arc[1].x = UPSCALE(control2->x);
	arc[1].y = UPSCALE(control2->y);
	arc[2].x = UPSCALE(control1->x);
	arc[2].y = UPSCALE(control1->y);
	arc[3].x = ras.x;
	arc[3].y = ras.y;

	/* Short-cut the arc that crosses the current band. */
	min = max = arc[0].y;

	y = arc[1].y;
	if (y < min)
		min = y;
	if (y > max)
		max = y;

	y = arc[2].y;
	if (y < min)
		min = y;
	if (y > max)
		max = y;

	y = arc[3].y;
	if (y < min)
		min = y;
	if (y > max)
		max = y;

	if (TRUNC(min) >= ras.max_ey || TRUNC(max) < ras.min_ey)
		goto Draw;

	for (;;)
	{
		/* Decide whether to split or draw. See `Rapid Termination          */
		/* Evaluation for Recursive Subdivision of Bezier Curves' by Thomas */
		/* F. Hain, at                                                      */
		/* http://www.cis.southalabama.edu/~hain/general/Publications/Bezier/Camera-ready%20CISST02%202.pdf */

		{
			TPos  dx, dy, dx_, dy_;
			TPos  dx1, dy1, dx2, dy2;
			TPos  L, s, s_limit;


			/* dx and dy are x and y components of the P0-P3 chord vector. */
			dx = arc[3].x - arc[0].x;
			dy = arc[3].y - arc[0].y;

			/* L is an (under)estimate of the Euclidean distance P0-P3.       */
			/*                                                                */
			/* If dx >= dy, then r = sqrt(dx^2 + dy^2) can be overestimated   */
			/* with least maximum error by                                    */
			/*                                                                */
			/*   r_upperbound = dx + (sqrt(2) - 1) * dy  ,                    */
			/*                                                                */
			/* where sqrt(2) - 1 can be (over)estimated by 107/256, giving an */
			/* error of no more than 8.4%.                                    */
			/*                                                                */
			/* Similarly, some elementary calculus shows that r can be        */
			/* underestimated with least maximum error by                     */
			/*                                                                */
			/*   r_lowerbound = sqrt(2 + sqrt(2)) / 2 * dx                    */
			/*                  + sqrt(2 - sqrt(2)) / 2 * dy  .               */
			/*                                                                */
			/* 236/256 and 97/256 are (under)estimates of the two algebraic   */
			/* numbers, giving an error of no more than 8.1%.                 */

			dx_ = GB_ABS(dx);
			dy_ = GB_ABS(dy);

			/* This is the same as                     */
			/*                                         */
			/*   L = ( 236 * GB_MAX( dx_, dy_ )        */
			/*       + 97 * GB_MIN( dx_, dy_ ) ) >> 8; */
			L = (dx_ > dy_ ? 236 * dx_ + 97 * dy_
				: 97 * dx_ + 236 * dy_) >> 8;

			/* Avoid possible arithmetic overflow below by splitting. */
			if (L > 32767)
				goto Split;

			/* Max deviation may be as much as (s/L) * 3/4 (if Hain's v = 1). */
			s_limit = L * (TPos)(ONE_PIXEL / 6);

			/* s is L * the perpendicular distance from P1 to the line P0-P3. */
			dx1 = arc[1].x - arc[0].x;
			dy1 = arc[1].y - arc[0].y;
			s = GB_ABS(dy * dx1 - dx * dy1);

			if (s > s_limit)
				goto Split;

			/* s is L * the perpendicular distance from P2 to the line P0-P3. */
			dx2 = arc[2].x - arc[0].x;
			dy2 = arc[2].y - arc[0].y;
			s = GB_ABS(dy * dx2 - dx * dy2);

			if (s > s_limit)
				goto Split;

			/* Split super curvy segments where the off points are so far
			   from the chord that the angles P0-P1-P3 or P0-P2-P3 become
			   acute as detected by appropriate dot products. */
			if (dx1 * (dx1 - dx) + dy1 * (dy1 - dy) > 0 ||
				dx2 * (dx2 - dx) + dy2 * (dy2 - dy) > 0)
				goto Split;

			/* No reason to split. */
			goto Draw;
		}

	Split:
		gray_split_cubic(arc);
		arc += 3;
		continue;

	Draw:
		gray_render_line(RAS_VAR_ arc[0].x, arc[0].y);

		if (arc == ras.bez_stack)
			return;

		arc -= 3;
	}
}

int gray_move_to(const GB_Vector* to, PWorker worker)
{
	TPos  x, y;


	/* record current cell, if any */
	if (!ras.invalid)
		gray_record_cell(RAS_VAR);

	/* start to a new position */
	x = UPSCALE(to->x);
	y = UPSCALE(to->y);

	gray_start_cell(RAS_VAR_ TRUNC(x), TRUNC(y));

	worker->x = x;
	worker->y = y;
	return GB_SUCCESS;
}

int gray_line_to(const GB_Vector* to, PWorker worker)
{
	gray_render_line(RAS_VAR_ UPSCALE(to->x), UPSCALE(to->y));
	return GB_SUCCESS;
}

int gray_conic_to(const GB_Vector* control, const GB_Vector* to, PWorker worker)
{
	gray_render_conic(RAS_VAR_ control, to);
	return GB_SUCCESS;
}

int gray_cubic_to(const GB_Vector* control1, const GB_Vector* control2, const GB_Vector* to, PWorker worker)
{
	gray_render_cubic(RAS_VAR_ control1, control2, to);
	return GB_SUCCESS;
}

void gray_render_span(int y, int count, const GB_Span* spans, PWorker worker)
{
	unsigned char*  p;
	GB_BitmapRec*   map = &worker->target;


	/* first of all, compute the scanline offset */
	p = (unsigned char*)map->buffer + y * map->pitch;

	for (; count > 0; count--, spans++)
	{
		unsigned char  coverage = spans->coverage;


		if (coverage)
		{
			/* For small-spans it is faster to do it by ourselves than
			 * calling `memset'.  This is mainly due to the cost of the
			 * function call.
			 */
			if (spans->len >= 8)
				GB_MEMSET(p + spans->x, (unsigned char)coverage, spans->len);
			else
			{
				unsigned char*  q = p + spans->x;


				switch (spans->len)
				{
				case 7: *q++ = (unsigned char)coverage;
				case 6: *q++ = (unsigned char)coverage;
				case 5: *q++ = (unsigned char)coverage;
				case 4: *q++ = (unsigned char)coverage;
				case 3: *q++ = (unsigned char)coverage;
				case 2: *q++ = (unsigned char)coverage;
				case 1: *q = (unsigned char)coverage;
				default:
					;
				}
			}
		}
	}
}

void gray_hline(RAS_ARG_ TCoord x, TCoord y, TPos area, TCoord acount)
{
	int  coverage;


	/* compute the coverage line's coverage, depending on the    */
	/* outline fill rule                                         */
	/*                                                           */
	/* the coverage percentage is area/(PIXEL_BITS*PIXEL_BITS*2) */
	/*                                                           */
	coverage = (int)(area >> (PIXEL_BITS * 2 + 1 - 8));
	/* use range 0..256 */
	if (coverage < 0)
		coverage = -coverage;
	if (coverage >= 256)
		coverage = 255;

	y += (TCoord)ras.min_ey;
	x += (TCoord)ras.min_ex;

	/* GB_Span.x is a 16-bit short, so limit our coordinates appropriately */
	if (x >= 32767)
		x = 32767;

	if (coverage)
	{
		GB_Span*  span;
		int       count;


		/* see whether we can add this span to the current list */
		count = ras.num_gray_spans;
		span = ras.gray_spans + count - 1;
		if (count > 0 &&
			ras.span_y == y &&
			(int)span->x + span->len == (int)x &&
			span->coverage == coverage)
		{
			span->len = (unsigned short)(span->len + acount);
			return;
		}

		if (ras.span_y != y || count >= GB_MAX_GRAY_SPANS)
		{
			if (ras.render_span && count > 0)
				ras.render_span(ras.span_y, count, ras.gray_spans,
					ras.render_span_data);

			ras.num_gray_spans = 0;
			ras.span_y = (int)y;

			span = ras.gray_spans;
		}
		else
			span++;

		/* add a gray span to the current list */
		span->x = (short)x;
		span->len = (unsigned short)acount;
		span->coverage = (unsigned char)coverage;

		ras.num_gray_spans++;
	}
}

void gray_sweep(RAS_ARG)
{
	int yindex;

	if (ras.num_cells == 0)
		return;

	ras.num_gray_spans = 0;

	for (yindex = 0; yindex < ras.ycount; yindex++)
	{
		PCell   cell = ras.ycells[yindex];
		TCoord  cover = 0;
		TCoord  x = 0;


		while (cell)
		{
			TPos  area;


			if (cell->x > x && cover != 0)
				gray_hline(RAS_VAR_ x, yindex, cover * (ONE_PIXEL * 2),
					cell->x - x);

			cover += cell->cover;
			area = cover * (ONE_PIXEL * 2) - cell->area;

			if (area != 0 && cell->x >= 0)
				gray_hline(RAS_VAR_ cell->x, yindex, area, 1);

			x = cell->x + 1;
			cell = cell->next;
		}

		if (cover != 0)
			gray_hline(RAS_VAR_ x, yindex, cover * (ONE_PIXEL * 2),
				ras.count_ex - x);
	}

	if (ras.render_span && ras.num_gray_spans > 0)
		ras.render_span(ras.span_y, ras.num_gray_spans,
			ras.gray_spans, ras.render_span_data);
}

GB_Outline_Funcs func_interface = { &gray_move_to, &gray_line_to, &gray_conic_to, &gray_cubic_to, 0, 0 };

int GB_Outline_Decompose(GB_Outline outline, const GB_Outline_Funcs* func_interface, void* user)
{
#undef SCALED
#define SCALED( x )  ( ( (x) << shift ) - delta )

	GB_Vector   v_last;
	GB_Vector   v_control;
	GB_Vector   v_start;

	GB_Vector*  point;
	GB_Vector*  limit;
	char*       tags;

	int    error;

	GB_INT32   n;         /* index of contour in outline     */
	GB_UINT32  first;     /* index of first point in contour */
	GB_INT32   tag;       /* current point's state           */

	GB_INT32   shift;
	GB_Pos   delta;


	if (!outline || !func_interface)
		return GB_FAILED;

	shift = func_interface->shift;
	delta = func_interface->delta;
	first = 0;

	for (n = 0; n < outline->n_contours; n++)
	{
		GB_INT32 last = outline->contours[n];
		if (last < 0)
			break;
		limit = outline->points + last;

		v_start = outline->points[first];
		v_start.x = SCALED(v_start.x);
		v_start.y = SCALED(v_start.y);

		v_last = outline->points[last];
		v_last.x = SCALED(v_last.x);
		v_last.y = SCALED(v_last.y);

		v_control = v_start;

		point = outline->points + first;
		tags = outline->tags + first;
		tag = GB_CURVE_TAG(tags[0]);

		/* A contour cannot start with a cubic control point! */
		if (tag == GB_CURVE_TAG_CUBIC)
			break;

		/* check first point to determine origin */
		if (tag == GB_CURVE_TAG_CONIC)
		{
			/* first point is conic control.  Yes, this happens. */
			if (GB_CURVE_TAG(outline->tags[last]) == GB_CURVE_TAG_ON)
			{
				/* start at last point if it is on the curve */
				v_start = v_last;
				limit--;
			}
			else
			{
				/* if both first and last points are conic,         */
				/* start at their middle and record its position    */
				/* for closure                                      */
				v_start.x = (v_start.x + v_last.x) / 2;
				v_start.y = (v_start.y + v_last.y) / 2;

				/* v_last = v_start; */
			}
			point--;
			tags--;
		}

		error = func_interface->move_to(&v_start, user);
		if (error)
			goto Exit;

		while (point < limit)
		{
			point++;
			tags++;

			tag = GB_CURVE_TAG(tags[0]);
			switch (tag)
			{
			case GB_CURVE_TAG_ON:  /* emit a single line_to */
			{
				GB_Vector  vec;


				vec.x = SCALED(point->x);
				vec.y = SCALED(point->y);

				error = func_interface->line_to(&vec, user);
				if (error)
					goto Exit;
				continue;
			}

			case GB_CURVE_TAG_CONIC:  /* consume conic arcs */
				v_control.x = SCALED(point->x);
				v_control.y = SCALED(point->y);

			Do_Conic:
				if (point < limit)
				{
					GB_Vector  vec;
					GB_Vector  v_middle;


					point++;
					tags++;
					tag = GB_CURVE_TAG(tags[0]);

					vec.x = SCALED(point->x);
					vec.y = SCALED(point->y);

					if (tag == GB_CURVE_TAG_ON)
					{
						error = func_interface->conic_to(&v_control, &vec, user);
						if (error)
							goto Exit;
						continue;
					}

					if (tag != GB_CURVE_TAG_CONIC)
						break;

					v_middle.x = (v_control.x + vec.x) / 2;
					v_middle.y = (v_control.y + vec.y) / 2;

					error = func_interface->conic_to(&v_control, &v_middle, user);
					if (error)
						goto Exit;

					v_control = vec;
					goto Do_Conic;
				}

				error = func_interface->conic_to(&v_control, &v_start, user);
				goto Close;

			default:  /* GB_CURVE_TAG_CUBIC */
			{
				GB_Vector  vec1, vec2;


				if (point + 1 > limit ||
					GB_CURVE_TAG(tags[1]) != GB_CURVE_TAG_CUBIC)
					break;

				point += 2;
				tags += 2;

				vec1.x = SCALED(point[-2].x);
				vec1.y = SCALED(point[-2].y);

				vec2.x = SCALED(point[-1].x);
				vec2.y = SCALED(point[-1].y);

				if (point <= limit)
				{
					GB_Vector  vec;


					vec.x = SCALED(point->x);
					vec.y = SCALED(point->y);

					error = func_interface->cubic_to(&vec1, &vec2, &vec, user);
					if (error)
						goto Exit;
					continue;
				}

				error = func_interface->cubic_to(&vec1, &vec2, &v_start, user);
				goto Close;
			}
			}
		}
		error = func_interface->line_to(&v_start, user);

	Close:
		if (error)
			goto Exit;

		first = last + 1;
	}
	return GB_SUCCESS;

Exit:
	return error;
}

int gray_convert_glyph_inner(RAS_ARG)
{
	volatile int  error;

	error = GB_Outline_Decompose(&worker->outline, &func_interface, worker);
	if (gray_record_cell(worker))
		error = -4;
	return error;
}

int gray_convert_glyph(RAS_ARG)
{
	TBand            bands[40];
	TBand* volatile  band;
	int volatile          n, num_bands;
	TPos volatile         min, max, max_y;
	GB_BBox*              clip;


	/* Set up state in the raster object */
	gray_compute_cbox(RAS_VAR);

	/* clip to target bitmap, exit if nothing to do */
	clip = &ras.clip_box;

	if (ras.max_ex <= clip->xMin || ras.min_ex >= clip->xMax ||
		ras.max_ey <= clip->yMin || ras.min_ey >= clip->yMax)
		return GB_FAILED;

	if (ras.min_ex < clip->xMin) ras.min_ex = clip->xMin;
	if (ras.min_ey < clip->yMin) ras.min_ey = clip->yMin;

	if (ras.max_ex > clip->xMax) ras.max_ex = clip->xMax;
	if (ras.max_ey > clip->yMax) ras.max_ey = clip->yMax;

	ras.count_ex = ras.max_ex - ras.min_ex;
	ras.count_ey = ras.max_ey - ras.min_ey;

	/* set up vertical bands */
	num_bands = (int)((ras.max_ey - ras.min_ey) / ras.band_size);
	if (num_bands == 0)
		num_bands = 1;
	if (num_bands >= 39)
		num_bands = 39;

	ras.band_shoot = 0;

	min = ras.min_ey;
	max_y = ras.max_ey;

	for (n = 0; n < num_bands; n++, min = max)
	{
		max = min + ras.band_size;
		if (n == num_bands - 1 || max > max_y)
			max = max_y;

		bands[0].min = min;
		bands[0].max = max;
		band = bands;

		while (band >= bands)
		{
			TPos  bottom, top, middle;
			int   error;

			{
				PCell  cells_max;
				int    yindex;
				long   cell_start, cell_end, cell_mod;


				ras.ycells = (PCell*)ras.buffer;
				ras.ycount = band->max - band->min;

				cell_start = sizeof(PCell) * ras.ycount;
				cell_mod = cell_start % sizeof(TCell);
				if (cell_mod > 0)
					cell_start += sizeof(TCell) - cell_mod;

				cell_end = ras.buffer_size;
				cell_end -= cell_end % sizeof(TCell);

				cells_max = (PCell)((char*)ras.buffer + cell_end);
				ras.cells = (PCell)((char*)ras.buffer + cell_start);
				if (ras.cells >= cells_max)
					goto ReduceBands;

				ras.max_cells = cells_max - ras.cells;
				if (ras.max_cells < 2)
					goto ReduceBands;

				for (yindex = 0; yindex < ras.ycount; yindex++)
					ras.ycells[yindex] = 0;
			}

			ras.num_cells = 0;
			ras.invalid = 1;
			ras.min_ey = band->min;
			ras.max_ey = band->max;
			ras.count_ey = band->max - band->min;

			error = gray_convert_glyph_inner(RAS_VAR);

			if (!error)
			{
				gray_sweep(RAS_VAR);
				band--;
				continue;
			}
			else if (error != -4)
				return GB_FAILED;

		ReduceBands:
			/* render pool overflow; we will reduce the render band by half */
			bottom = band->min;
			top = band->max;
			middle = bottom + ((top - bottom) >> 1);

			/* This is too complex for a single scanline; there must */
			/* be some problems.                                     */
			if (middle == bottom)
			{
				return GB_FAILED;
			}

			if (bottom - top >= ras.band_size)
				ras.band_shoot++;

			band[1].min = bottom;
			band[1].max = middle;
			band[0].min = middle;
			band[0].max = top;
			band++;
		}
	}

	if (ras.band_shoot > 8 && ras.band_size > 16)
		ras.band_size = ras.band_size / 2;

	return GB_SUCCESS;
}

int GreyBit_Raster_Render(void *raster, GB_Bitmap tobitmap, GB_Outline fromoutline)
{
	PRaster me = (PRaster)raster;
	PWorker worker = me->worker;

	if (!fromoutline)
		return GB_FAILED;
	if (!fromoutline->n_points || fromoutline->n_contours <= 0)
		return GB_FAILED;
	if (!fromoutline->contours || !fromoutline->points)
		return GB_FAILED;
	if (fromoutline->n_points != fromoutline->contours[fromoutline->n_contours - 1] + 1)
		return GB_FAILED;
	if (!tobitmap)
		return GB_FAILED;
	if (!tobitmap->width || !tobitmap->height)
		return GB_FAILED;
	if (!tobitmap->buffer)
		return GB_FAILED;
	worker->clip_box.xMin = 0;
	worker->clip_box.yMin = 0;
	worker->clip_box.xMax = tobitmap->width;
	worker->clip_box.yMax = tobitmap->height;
	gray_init_cells(worker, me->buffer, me->buffer_size);
	worker->outline = *fromoutline;
	worker->num_cells = 0;
	worker->invalid = 1;
	worker->band_size = me->band_size;
	worker->num_gray_spans = 0;
	worker->target = *tobitmap;
	worker->render_span = gray_render_span;
	worker->render_span_data = worker;
	return gray_convert_glyph(worker);
}

void* GreyBit_Raster_New(GB_Library library, int nPoolSize)
{
	PRaster me;

	if (nPoolSize <= 0)
		nPoolSize = DEFAULT_POOL_SIZE;
	me = (PRaster)GreyBit_Malloc(library->gbMem, nPoolSize);
	if (me)
	{
		me->gbMem = library->gbMem;
		me->worker = (PWorker)((GB_BYTE*)me + sizeof(TRaster));
		me->buffer = (GB_BYTE*)me->worker + sizeof(TWorker);
		me->buffer_size = (nPoolSize - sizeof(TRaster) - sizeof(TWorker)) & 0xFFFFFFF0;
		me->band_size = me->buffer_size >> 7;
	}
	return (void*)me;
}

void GreyBit_Raster_Done(void *raster)
{
	PRaster me = (PRaster)raster;
	GreyBit_Free(me->gbMem, raster);
}
#endif //ENABLE_GREYVECTORFILE