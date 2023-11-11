#include "smooth_earth_heights.h"
#include "custom_math.h"
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
    if (use_v1_v2_caches && !c_isnan(input->v1_cache[input->n]) && !c_isnan(input->v2_cache[input->n]))
    {
        v1 = input->v1_cache[input->n];
        v2 = input->v2_cache[input->n];
    }
    else
    {
        for (int i = 1; i < input->n; i++)
        {
            double diff_d = input->d[i] - input->d[i - 1];
            double sum_h = input->h[i] + input->h[i - 1];
            v1 += diff_d * sum_h;
            v2 += diff_d * (input->h[i] * (2 * input->d[i] + input->d[i - 1]) + input->h[i - 1] * (input->d[i] + 2 * input->d[i - 1]));
            if (use_v1_v2_caches)
            {
                input->v1_cache[i + 1] = v1;
                input->v2_cache[i + 1] = v2;
            }
        }
    }

    output->hst = (2 * v1 * input->dtot - v2) / c_pow(input->dtot, 2);
    output->hsr = (v2 - v1 * input->dtot) / c_pow(input->dtot, 2);

    output->hst_n = output->hst;
    output->hsr_n = output->hsr;

    // Section 5.6.2 Smooth-surface heights for the diffraction model
    double hobs = NEGATIVE_INFINITY;
    double alpha_obt = NEGATIVE_INFINITY;
    double alpha_obr = NEGATIVE_INFINITY;
    for (int i = 1; i < input->n - 1; i++)
    {
        double dtot_min_d = input->dtot - input->d[i];
        double HHi = input->h[i] - (output->htc * dtot_min_d + output->hrc * input->d[i]) / input->dtot;
        hobs = c_max(hobs, HHi);
        alpha_obt = c_max(alpha_obt, HHi / input->d[i]);
        alpha_obr = c_max(alpha_obr, HHi / dtot_min_d);
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
    double theta_max = NEGATIVE_INFINITY;
    bool use_theta_max_cache = (input->theta_max_cache != NULL);
    if (use_theta_max_cache && !c_isnan(input->theta_max_cache[input->n]))
    {
        theta_max = input->theta_max_cache[input->n];
    }
    else
    {
        for (int i = 1; i < input->n - 1; i++)
        {
            theta = KM * c_atan((input->h[i] - output->hts) / (KM * input->d[i]) - input->d[i] / (2 * input->ae));
            theta_max = c_max(theta_max, theta);
            if (use_theta_max_cache)
            {
                input->theta_max_cache[i + 1] = theta_max;
            }
        }
    }

    double theta_td = KM * c_atan((output->hrs - output->hts) / (KM * input->dtot) - input->dtot / (2 * input->ae));
    double theta_rd = KM * c_atan((output->hts - output->hrs) / (KM * input->dtot) - input->dtot / (2 * input->ae));
    double theta_t = c_max(theta_max, theta_td);

    double theta_r;
    if (theta_max > theta_td) // Transhorizon path
    {
        theta_r = NEGATIVE_INFINITY;

        double numax = NEGATIVE_INFINITY;
        for (int i = 1; i < input->n - 1; i++)
        {
            double d = input->d[i];
            double dtot_min_d = input->dtot - d;
            theta = KM * c_atan((input->h[i] - output->hrs) / (KM * dtot_min_d) - dtot_min_d / (2 * input->ae));
            theta_r = c_max(theta_r, theta);

            double d_min_1 = input->d[i - 1];
            double dtot_min_d_min_1 = input->dtot - d_min_1;
            double nu = input->h[i] + 500 * input->ae * (d * dtot_min_d - d_min_1 * dtot_min_d_min_1) - (output->hts * dtot_min_d_min_1 + output->hrs * d_min_1) / input->dtot;
            nu *= c_sqrt(0.002 * input->dtot / (input->lambda * d_min_1 * dtot_min_d_min_1));
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

        double numax = NEGATIVE_INFINITY;
        for (int i = 1; i < input->n - 1; i++)
        {
            double Ce = 1.0 / input->ae;

            double d = input->d[i];
            double dtot_min_d = input->dtot - d;
            double nu = input->h[i] + 500 * Ce * input->d[i] * dtot_min_d - (output->hts * dtot_min_d + output->hrs * d) / input->dtot;
            nu *= c_sqrt(0.002 * input->dtot / (input->lambda * d * dtot_min_d));
            if (nu > numax)
            {
                numax = nu;
                output->dlt = d;
            }
        }
    }

    output->dlr = input->dtot - output->dlt;

    // Angular distance
    double theta_tot = KM * input->dtot / input->ae + theta_t + theta_r;
    output->theta = theta_tot;
    output->theta_t = theta_t;
    output->theta_r = theta_r;

    // Section 5.6.3 Ducting/layer-reflection model

    // Calculate the smooth-Earth heights at transmitter and receiver as
    // required for the roughness factor
    output->hst = c_min(output->hst, input->h[0]);
    output->hsr = c_min(output->hsr, input->h[input->n - 1]);

    // The terminal effective heigts for the ducting/layer-reflection model
    output->hte = input->htg + input->h[0] - output->hst;
    output->hre = input->hrg + input->h[input->n - 1] - output->hsr;

    // TODO
    output->hm = 0.0;
}
