#include "rf.h"
#include "custom_math.h"

double watts_to_dBm(double P)
{
	return 10 * c_log10(P) + 30;
}

double link_budget(double P, double Gt, double Gr, double L)
{
	return watts_to_dBm(P) + Gt + Gr - L;
}
