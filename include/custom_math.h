#ifndef CUSTOM_MATH_H
#define CUSTOM_MATH_H

#define NEGATIVE_INFINITY (-1.0 / 0.0)
#define INFINITY (1.0 / 0.0)
#define NAN (0.0 / 0.0)

#define PI 3.14159265358979323846
#define PI_2 1.57079632679489661923

double c_isnan(double x);
double c_floor(double x);
double c_ceil(double x);
double c_round(double x);
double c_abs(double x);
double c_min(double x, double y);
double c_max(double x, double y);
double c_pow(double x, double y);
double c_atan(double x);
double c_atan2(double y, double x);
double c_atan2_exact(double y, double x);
double c_exp(double x);
double c_log(double x);
double c_sqrt(double x);
double c_cbrt(double x);
double c_log10(double x);
double c_tanh(double x);
double c_sin(double x);
double c_cos(double x);
double c_acos(double x);

#endif