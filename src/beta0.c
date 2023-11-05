#include "beta0.h"
#include "pow.h"
#include <math.h>

double beta0(double phi, double dtm, double dlm)
{
    double tau = 1 - exp(-pow2(4.12e-4 * pow2(dlm, 2.41), 1));                                       // (3)
    double mu1 = pow2(pow2(10, -dtm / (16 - 6.6 * tau)) + pow2(10, -5 * (0.496 + 0.354 * tau)), 0.2); // (2)
    mu1 = fmin(mu1, 1.0);

    double mu4;
    if (fabs(phi) <= 70)
    {
        mu4 = pow2(mu1, -0.935 + 0.0176 * fabs(phi));           // (4)
        return pow2(210, -0.015 * fabs(phi) + 1.67) * mu1 * mu4; // (5)
    }
    else
    {
        mu4 = pow2(mu1, 0.3);     // (4)
        return 4.17 * mu1 * mu4; // (5)
    }
}