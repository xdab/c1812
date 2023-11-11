#include "inv_cum_norm.h"
#include "custom_math.h"

double T(double y)
{
    return c_sqrt(-2 * c_log(y));
}

double C(double z)
{
    const double C0 = 2.515516698;
    const double C1 = 0.802853;
    const double C2 = 0.010328;
    const double D1 = 1.432788;
    const double D2 = 0.189269;
    const double D3 = 0.001308;
    return (((C2 * T(z) + C1) * T(z)) + C0) / (((D3 * T(z) + D2) * T(z) + D1) * T(z) + 1); // (97b)
}

double inv_cum_norm(double x)
{
    if (x < 0.000001)
        return 0.000001;

    if (x > 0.999999)
        return 0.999999;

    if (x <= 0.5)
        return T(x) - C(x); // (96a)

    return -(T(1 - x) - C(1 - x)); // (96b)
}