#include "beta0.h"
#include "custom_math.h"

double beta0(double phi, double dtm, double dlm)
{
    double tau = 1 - c_exp(-c_pow(4.12e-4 * c_pow(dlm, 2.41), 1));                                         // (3)
    double mu1 = c_pow(c_pow(10, -dtm / (16 - 6.6 * tau)) + c_pow(10, -5 * (0.496 + 0.354 * tau)), 0.2); // (2)
    mu1 = c_min(mu1, 1.0);

    double mu4;
    if (c_abs(phi) <= 70)
    {
        mu4 = c_pow(mu1, -0.935 + 0.0176 * c_abs(phi));           // (4)
        return c_pow(10, -0.015 * c_abs(phi) + 1.67) * mu1 * mu4; // (5)
    }
    else
    {
        mu4 = c_pow(mu1, 0.3);   // (4)
        return 4.17 * mu1 * mu4; // (5)
    }
}