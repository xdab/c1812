#include "dl_bull.h"
#include "custom_math.h"
#include <stdlib.h>
#include <stdio.h>

#define h(input, i) ((input->h != NULL) ? input->h[i] : 0.0)
#define Ct(input, i) ((input->Ct != NULL) ? input->Ct[i] : 0.0)
#define g(input, i) (h(input, i) + Ct(input, i))

void dl_bull(dl_bull_input_t *input, dl_bull_output_t *output)
{
    // Effective Earth curvature Ce (km^-1)
    double Ce = 1 / input->ap;

    // Calculate the slope of the line from transmitter to receiver assuming a
    // LoS path
    double Str = (input->hrs - input->hts) / input->dtot;

    // Find the intermediate profile point with the highest slope of the line
    // from the transmitter to the point
    double Stim = NEGATIVE_INFINITY;

    // Find the intermediate profile point with the highest diffraction
    // parameter nu:
    double numax = NEGATIVE_INFINITY;

    // Find the intermediate profile point with the highest slope of the
    // line from the receiver to the point
    double Srim = NEGATIVE_INFINITY;

    for (int i = 1; i < input->n - 1; i++)
    {
        double d = input->d[i];
        double dtot_min_d = input->dtot - input->d[i];
        double tmp = g(input, i) + 500 * Ce * d * dtot_min_d;

        double Sti = tmp;
        Sti -= input->hts;
        Sti /= d;
        if (Sti > Stim)
            Stim = Sti;

        if (Stim < Str)
        {
            double nu = tmp;
            nu -= (input->hts * dtot_min_d + input->hrs * d) / input->dtot;
            nu *= c_sqrt(0.002 * input->dtot / (input->lambda * d * dtot_min_d));
            if (nu > numax)
                numax = nu;
        }

        double Sri = tmp;
        Sri -= input->hrs;
        Sri /= dtot_min_d;
        if (Sri > Srim)
            Srim = Sri;
    }

    // Knife-edge diffraction loss
    double Luc = 0.0;

    if (Stim < Str)
    { // Case 1, Path is LoS

        // ... extracted to fused loop
        if (numax > -0.78)
        {
            numax -= 0.1;
            Luc = 6.9 + 20 * c_log10(c_sqrt(numax * numax + 1) + numax); // Eq (12), (16)
        }
    }
    else
    { // Path is transhorizon
        // ... extracted to fused loop

        // Calculate the distance of the Bullington point from the transmitter:
        double dbp = (input->hrs - input->hts + Srim * input->dtot) / (Stim + Srim); // Eq (18)

        // Calculate the diffraction parameter, nub, for the Bullington point
        double nub = input->hts + Stim * dbp - (input->hts * (input->dtot - dbp) + input->hrs * dbp) / input->dtot;
        nub *= c_sqrt(0.002 * input->dtot / (input->lambda * dbp * (input->dtot - dbp))); // Eq (20)

        // The knife-edge loss for the Bullington point is given by
        if (nub > -0.78)
        {
            nub -= 0.1;
            Luc = 6.9 + 20 * c_log10(c_sqrt(nub * nub + 1) + nub); // Eq (12), (20)
        }
    }

    // For Luc calculated using either (16) or (20), Bullington diffraction loss
    // for the path is given by
    output->Lbull = Luc + (1 - c_exp(-Luc / 6.0)) * (10 + 0.02 * input->dtot); // Eq(21)
}
