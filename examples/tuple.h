#ifndef TUPLE_H
#define TUPLE_H

struct tuple
{
	double x;
	double y;
	double z;
	double w;
};

void tuple(struct tuple *, double, double, double, double);
void point(struct tuple *, double, double, double);
void vector(struct tuple *, double, double, double);

int tuple_is_point(struct tuple *);
int tuple_is_vector(struct tuple *);
int tuple_is_equal(struct tuple *, struct tuple *);

void tuple_add(struct tuple *, struct tuple *);
void tuple_sub(struct tuple *, struct tuple *);
void tuple_neg(struct tuple *);

#endif
