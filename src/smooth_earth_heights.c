#include "smooth_earth_heights.h"
#include "constants.h"
#include "pow.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define KM 1000.0

void smooth_earth_heights(seh_input_t *input, seh_output_t *output)
{
    output->hts = input->h[0] + input->htg;
    output->hrs = input->h[input->n - 1] + input->hrg;

    // Compute htc and hrc as defined in Table 5 (P.1812-6)
    output->htc = output->hts;
    output->hrc = output->hrs;

    double v1 = 0.0;
    double v2 = 0.0;
    bool use_v1_v2_caches = (input->v1_cache != NULL) && (input->v2_cache != NULL);
    if (use_v1_v2_caches && !isnan(input->v1_cache[input->n]) && !isnan(input->v2_cache[input->n]))
    {
        v1 = input->v1_cache[input->n];
        v2 = input->v2_cache[input->n];
    }
    else
    {
        for (int i = 1; i < input->n; i++)
        {
            double diff_d = input->d[i] - input->d[i - 1];
            v1 += diff_d * (input->h[i] + input->h[i - 1]);
            v2 += diff_d * (input->h[i] * (2 * input->d[i] + input->d[i - 1]) + input->h[i - 1] * (input->d[i] + 2 * input->d[i - 1]));
            if (use_v1_v2_caches)
            {
                input->v1_cache[i] = v1;
                input->v2_cache[i] = v2;
            }
        }
    }

    output->hst = (2 * v1 * input->dtot - v2) / pow2(input->dtot, 2);
    output->hsr = (v2 - v1 * input->dtot) / pow2(input->dtot, 2);

    output->hst_n = output->hst;
    output->hsr_n = output->hsr;

    // Section 5.6.2 Smooth-surface heights for the diffraction model
    double hobs = -INFINITY;
    double alpha_obt = -INFINITY;
    double alpha_obr = -INFINITY;
    for (int i = 1; i < input->n - 1; i++)
    {
        double HHi = input->h[i] - (output->htc * (input->dtot - input->d[i]) + output->hrc * input->d[i]) / input->dtot;
        hobs = fmax(hobs, HHi);
        alpha_obt = fmax(alpha_obt, HHi / input->d[i]);
        alpha_obr = fmax(alpha_obr, HHi / (input->dtot - input->d[i]));
    }

    // Calculate provisional values for the Tx and Rx smooth surface heights
    double gt = alpha_obt / (alpha_obt + alpha_obr);
    double gr = alpha_obr / (alpha_obt + alpha_obr);

    if (hobs <= 0)
    {
        output->hstp = output->hst;
        output->hsrp = output->hsr;
    }
    else
    {
        output->hstp = output->hst - hobs * gt;
        output->hsrp = output->hsr - hobs * gr;
    }

    // calculate the final values as required by the diffraction model
    if (output->hstp >= input->h[0])
        output->hstd = input->h[0];
    else
        output->hstd = output->hstp;

    if (output->hsrp >= input->h[input->n - 1])
        output->hsrd = input->h[input->n - 1];
    else
        output->hsrd = output->hsrp;

    // Interfering antenna horizon elevation angle and distance
    double theta;
    double theta_max = -INFINITY;
    bool use_theta_max_cache = (input->theta_max_cache != NULL);
    if (use_theta_max_cache && !isnan(input->theta_max_cache[input->n]))
    {
        theta_max = input->theta_max_cache[input->n];
    }
    else
    {
        for (int i = 1; i < input->n - 1; i++)
        {
            theta = KM * atan((input->h[i] - output->hts) / (KM * input->d[i]) - input->d[i] / (2 * ER));
            theta_max = fmax(theta_max, theta);
            if (use_theta_max_cache)
            {
                input->theta_max_cache[i + 2] = theta_max;
            }
        }
    }

    double theta_td = KM * atan((output->hrs - output->hts) / (KM * input->dtot) - input->dtot / (2 * ER));
    double theta_rd = KM * atan((output->hts - output->hrs) / (KM * input->dtot) - input->dtot / (2 * ER));
    double theta_t = fmax(theta_max, theta_td);

    double theta_r;
    if (theta_max > theta_td) // Transhorizon path
    {
        theta_r = -INFINITY;

        double numax = -INFINITY;
        for (int i = 1; i < input->n - 1; i++)
        {
            theta = KM * atan((input->h[i] - output->hrs) / (KM * (input->dtot - input->d[i])) - (input->dtot - input->d[i]) / (2 * ER));
            theta_r = fmax(theta_r, theta);

            double nu = (input->h[i] + 500 * ER * (input->d[i] * (input->dtot - input->d[i]) - input->d[i - 1] * (input->dtot - input->d[i - 1])) - (output->hts * (input->dtot - input->d[i - 1]) + output->hrs * input->d[i - 1]) / input->dtot) * sqrt(0.002 * input->dtot / (input->lambda * input->d[i - 1] * (input->dtot - input->d[i - 1])));
            if (nu > numax)
            {
                numax = nu;
                output->dlt = input->d[i + 1];
            }
        }
    }
    else
    { // Line-of-sight path
        theta_r = theta_rd;

        double numax = -INFINITY;
        for (int i = 1; i < input->n - 1; i++)
        {
            double nu = (input->h[i] + 500 * ER * input->d[i] * (input->dtot - input->d[i]) - (output->hts * (input->dtot - input->d[i]) + output->hrs * input->d[i]) / input->dtot) * sqrt(0.002 * input->dtot / (input->lambda * input->d[i] * (input->dtot - input->d[i])));
            if (nu > numax)
            {
                numax = nu;
                output->dlt = input->d[i];
            }
        }
    }

    output->dlr = input->dtot - output->dlt;

    // Angular distance
    double theta_tot = KM * input->dtot / ER + theta_t + theta_r;
    output->theta = theta_tot;

    // Section 5.6.3 Ducting/layer-reflection model

    // Calculate the smooth-Earth heights at transmitter and receiver as
    // required for the roughness factor
    double hst_smooth = fmin(output->hst, input->h[0]);
    double hsr_smooth = fmin(output->hsr, input->h[input->n - 1]);

    // The terminal effective heigts for the ducting/layer-reflection model
    output->hte = input->htg + input->h[0] - hst_smooth;
    output->hre = input->hrg + input->h[input->n - 1] - hsr_smooth;
}
