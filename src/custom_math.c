#include "custom_math.h"
#include <math.h>
#include <stdbool.h>

#define A1 0.99997726
#define A3 -0.33262347
#define A5 0.19354346
#define A7 -0.11643287
#define A9 0.05265332
#define A11 -0.01172120

double c_isnan(double x)
{
	return isnan(x);
}

double c_floor(double x)
{
	return floor(x);
}

double c_ceil(double x)
{
	return ceil(x);
}

double c_round(double x)
{
	return round(x);
}

double c_abs(double x)
{
	return fabs(x);
}

double c_min(double x, double y)
{
	return fmin(x, y);
}

double c_max(double x, double y)
{
	return fmax(x, y);
}

double c_pow(double x, double y)
{
	return pow(x, y);
}

double c_atan(double x)
{
	double x2 = x * x;
	return x * (A1 + x2 * (A3 + x2 * (A5 + x2 * (A7 + x2 * (A9 + x2 * A11)))));
}

double c_atan2(double y, double x)
{
	bool swap = c_abs(x) < c_abs(y);
	double atan_input = (swap ? x : y) / (swap ? y : x);
	double res = c_atan(atan_input);
	res = swap ? (atan_input >= 0.0 ? PI_2 : -PI_2) - res : res;
	if (x < 0.0 && y >= 0.0)
		res = PI + res; // 2nd quadrant
	else if (x < 0.0 && y < 0.0)
		res = -PI + res; // 3rd quadrant
	return res;
}

double c_atan2_exact(double y, double x)
{
	return atan2(y, x);
}

double c_exp(double x)
{
	return exp(x);
}

double c_log(double x)
{
	return log(x);
}

double c_sqrt(double x)
{
	return sqrt(x);
}

double c_cbrt(double x)
{
	return cbrt(x);
}

double c_log10(double x)
{
	return log10(x);
}

double c_tanh(double x)
{
	return tanh(x);
}

double c_sin(double x)
{
	return sin(x);
}

double c_cos(double x)
{
	return cos(x);
}

double c_acos(double x)
{
	return acos(x);
}