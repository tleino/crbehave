#include "tuple.h"

#include <string.h>

void
tuple(struct tuple *t, double x, double y, double z, double w)
{
	t->x = x;
	t->y = y;
	t->z = z;
	t->w = w;
}

void
tuple_add(struct tuple *t1, struct tuple *t2)
{
	t1->x += t2->x;
	t1->y += t2->y;
	t1->z += t2->z;
	t1->w += t2->w;
}

void
tuple_sub(struct tuple *t1, struct tuple *t2)
{
	t1->x -= t2->x;
	t1->y -= t2->y;
	t1->z -= t2->z;
	t1->w -= t2->w;
}

void
tuple_neg(struct tuple *t1)
{
	t1->x = -t1->x;
	t1->y = -t1->y;
	t1->z = -t1->z;
	t1->w = -t1->w;
}

void
point(struct tuple *t, double x, double y, double z)
{
	tuple(t, x, y, z, 1);
}

void
vector(struct tuple *t, double x, double y, double z)
{
	tuple(t, x, y, z, 0);
}

int
tuple_is_point(struct tuple *t)
{
	return t->w == 1;
}

int
tuple_is_vector(struct tuple *t)
{
	return t->w == 0;
}

int
tuple_is_equal(struct tuple *t1, struct tuple *t2)
{
	return memcmp(&t1, &t2, sizeof(struct tuple));
}
