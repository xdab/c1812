#include "pow.h"
#include <math.h>

double pow2(double x, double y)
{
	if (y < 0) 
		return 1 / pow(x, -y);
	return pow(x, y);
}