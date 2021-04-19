#include "canvas.h"

#include <stdio.h>

void
ppm_write_canvas(FILE *fp, struct canvas *canvas)
{
	fprintf(fp, "P3\n");
	fprintf(fp, "%d %d\n", canvas->width, canvas->height);
	fprintf(fp, "255\n");
}
