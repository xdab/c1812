#include "smooth_earth_heights.h"
#include "constants.h"
#include <math.h>
#include <stdlib.h>

void smooth_earth_heights(seh_input_t *input, seh_output_t *output)
{
    output->hts = input->h[0] + input->htg;
    output->hrs = input->h[input->n - 1] + input->hrg;

    // Modify the path by adding representative clutter, according to Section 3.2
    // excluding the first and the last point
    double *g = (double *)malloc(sizeof(double) * input->n);
    for (int i = 1; i < input->n - 1; i++)
        g[i] = input->h[i] + ((input->Ct != NULL) ? input->Ct[i] : 0.0);
    g[0] = input->h[0];
    g[input->n - 1] = input->h[input->n - 1];
    output->g = g;

    // Compute htc and hrc as defined in Table 5 (P.1812-6)
    output->htc = output->hts;
    output->hrc = output->hrs;

    double v1 = 0.0;
    double v2 = 0.0;
    for (int i = 0; i < input->n - 1; i++)
    {
        double diff_d = input->d[i + 1] - input->d[i];
        v1 += diff_d * (input->h[i + 1] + input->h[i]);
        v2 += diff_d * (input->h[i + 1] * (2 * input->d[i + 1] + input->d[i]) + input->h[i] * (input->d[i + 1] + 2 * input->d[i]));
    }

    output->hst = (2 * v1 * input->dtot - v2) / pow(input->dtot, 2);
    output->hsr = (v2 - v1 * input->dtot) / pow(input->dtot, 2);

    // Section 5.6.2 Smooth-surface heights for the diffraction model

    double *HH = (double *)malloc(sizeof(double) * input->n);
    for (int i = 0; i < input->n; i++)
        HH[i] = input->h[i] - (output->htc * (input->dtot - input->d[i]) + output->hrc * input->d[i]) / input->dtot;

    double hobs = 0;
    for (int i = 1; i < input->n - 1; i++)
        hobs = fmax(hobs, HH[i]);

    double alpha_obt = 0;
    for (int i = 1; i < input->n - 1; i++)
        alpha_obt = fmax(alpha_obt, HH[i] / input->d[i]);

    double alpha_obr = 0;
    for (int i = 1; i < input->n - 1; i++)
        alpha_obr = fmax(alpha_obr, HH[i] / (input->dtot - input->d[i]));

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
    output->hstd = (output->hstp >= input->h[0]) ? input->h[0] : output->hstp;
    output->hsrd = (output->hsrp > input->h[input->n - 1]) ? input->h[input->n - 1] : output->hsrp;

    // Interfering antenna horizon elevation angle and distance
    double *theta = (double *)malloc(sizeof(double) * (input->n - 2));
    for (int i = 1; i < input->n - 1; i++)
        theta[i - 1] = 1000 * atan((input->h[i] - output->hts) / (1000 * input->d[i]) - input->d[i] / (2 * ER));
    double theta_td = 1000 * atan((output->hrs - output->hts) / (1000 * input->dtot) - input->dtot / (2 * ER));
    double theta_rd = 1000 * atan((output->hts - output->hrs) / (1000 * input->dtot) - input->dtot / (2 * ER));
    double theta_max = 0;
    for (int i = 0; i < input->n - 2; i++)
        theta_max = fmax(theta_max, theta[i]);

    int pathtype = (theta_max > theta_td) ? 2 : 1;

    double theta_t = fmax(theta_max, theta_td);

    int lt = 0;
    int lr = 0;

    double theta_r;
    if (pathtype == 2)
    { // transhorizon
        theta_r = 0;
        for (int i = 1; i < input->n - 1; i++)
        {
            double temp = (input->h[i] - output->hrs) / (1000 * (input->dtot - input->d[i])) - (input->dtot - input->d[i]) / (2 * ER);
            theta[i - 1] = 1000 * atan(temp);
            theta_r = fmax(theta_r, theta[i - 1]);
        }

        int kindex = 0;
        double numax = 0;
        for (int i = 0; i < input->n - 2; i++)
        {
            double temp = (input->h[i + 1] + 500 * ER * (input->d[i + 1] * (input->dtot - input->d[i + 1]) - input->d[i] * (input->dtot - input->d[i])) - (output->hts * (input->dtot - input->d[i]) + output->hrs * input->d[i]) / input->dtot) * sqrt(0.002 * input->dtot / (input->lambda * input->d[i] * (input->dtot - input->d[i])));
            if (temp > numax)
            {
                numax = temp;
                kindex = i;
            }
        }

        lt = kindex + 1;
        output->dlt = input->d[lt];
        output->dlr = input->dtot - output->dlt;
        lr = lt;
    }
    else
    { // pathtype == 1 (LoS)
        theta_r = theta_rd;
        double *nu = (double *)malloc(sizeof(double) * (input->n - 2));
        for (int i = 1; i < input->n - 1; i++)
        {
            double temp = (input->h[i] + 500 * ER * input->d[i] * (input->dtot - input->d[i]) - (output->hts * (input->dtot - input->d[i]) + output->hrs * input->d[i]) / input->dtot) * sqrt(0.002 * input->dtot / (input->lambda * input->d[i] * (input->dtot - input->d[i])));
            nu[i - 1] = temp;
        }

        int kindex = 0;
        double numax = 0;
        for (int i = 0; i < input->n - 2; i++)
        {
            if (nu[i] > numax)
            {
                numax = nu[i];
                kindex = i;
            }
        }

        lt = kindex + 2;
        output->dlt = input->d[lt - 1];
        output->dlr = input->dtot - output->dlt;
        lr = lt;
        free(nu);
    }

    // Angular distance
    double theta_tot = 1e3 * input->dtot / ER + theta_t + theta_r;
    output->theta = theta_tot;

    // Section 5.6.3 Ducting/layer-reflection model

    // Calculate the smooth-Earth heights at transmitter and receiver as
    // required for the roughness factor
    double hst_smooth = fmin(output->hst, input->h[0]);
    double hsr_smooth = fmin(output->hsr, input->h[input->n - 1]);

    // Slope of the smooth-Earth surface
    double m = (hsr_smooth - hst_smooth) / input->dtot;

    // The terminal effective heigts for the ducting/layer-reflection model
    output->hte = input->htg + input->h[0] - hst_smooth;
    output->hre = input->hrg + input->h[input->n - 1] - hsr_smooth;

    double hm = 0;
    for (int i = lt; i <= lr; i++)
        hm = fmax(hm, input->h[i] - (hst_smooth + m * input->d[i]));

    free(HH);
    free(theta);
}
