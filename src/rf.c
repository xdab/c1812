#include "rf.h"
#include <math.h>

double fspl(double d, double f) {
	return 20 * log10(d) + 20 * log10(f) + 92.45;
}

double watts_to_dBm(double P)
{
	return 10 * log10(P) + 30;
}

double link_budget(double P, double Gt, double Gr, double L)
{
	return watts_to_dBm(P) + Gt + Gr - L;
}
