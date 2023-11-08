#include "dl_bull.h"
#include "pow.h"
#include <math.h>
#include <stdlib.h>

#define h(input, i) ((input->h != NULL) ? input->h[i] : 0.0)
#define Ct(input, i) ((input->Ct != NULL) ? input->Ct[i] : 0.0)
#define g(input, i) (h(input, i) + Ct(input, i))

void dl_bull(dl_bull_input_t *input, dl_bull_output_t *output)
{
    // Effective Earth curvature Ce (km^-1)
    double Ce = 1 / input->ap;

    // Find the intermediate profile point with the highest slope of the line
    // from the transmitter to the point
    double Stim = -INFINITY;
    for (int i = 1; i < input->n - 1; i++)
    {
        double temp = g(input, i);
        temp += 500 * Ce * input->d[i] * (input->dtot - input->d[i]);
        temp -= input->hts;
        temp /= input->d[i];

        if (temp > Stim)
            Stim = temp;
    }

    // Calculate the slope of the line from transmitter to receiver assuming a
    // LoS path
    double Str = (input->hrs - input->hts) / input->dtot;

    // Knife-edge diffraction loss
    double Luc = 0.0;

    if (Stim < Str)
    { // Case 1, Path is LoS
        // Find the intermediate profile point with the highest diffraction
        // parameter nu:
        double numax = -INFINITY;
        for (int i = 1; i < input->n - 1; i++)
        {
            double temp = g(input, i) + 500 * Ce * input->d[i] * (input->dtot - input->d[i]);
            temp -= (input->hts * (input->dtot - input->d[i]) + input->hrs * input->d[i]) / input->dtot;
            temp *= sqrt(0.002 * input->dtot / (input->lambda * input->d[i] * (input->dtot - input->d[i])));
            if (temp > numax)
                numax = temp;
        }

        if (numax > -0.78)
            Luc = 6.9 + 20 * log10(sqrt(pow2(numax - 0.1, 2) + 1) + numax - 0.1); // Eq (12), (16)
    }
    else
    { // Path is transhorizon
        // Find the intermediate profile point with the highest slope of the
        // line from the receiver to the point
        double Srim = -INFINITY;
        for (int i = 1; i < input->n - 2; i++)
        {
            double temp = (g(input, i) + 500 * Ce * input->d[i] * (input->dtot - input->d[i]) - input->hrs) / (input->dtot - input->d[i]);
            if (temp > Srim)
                Srim = temp;
        }

        // Calculate the distance of the Bullington point from the transmitter:
        double dbp = (input->hrs - input->hts + Srim * input->dtot) / (Stim + Srim); // Eq (18)

        // Calculate the diffraction parameter, nub, for the Bullington point
        double nub = input->hts + Stim * dbp - (input->hts * (input->dtot - dbp) + input->hrs * dbp) / input->dtot;
        nub *= sqrt(0.002 * input->dtot / (input->lambda * dbp * (input->dtot - dbp))); // Eq (20)

        // The knife-edge loss for the Bullington point is given by
        if (nub > -0.78)
            Luc = 6.9 + 20 * log10(sqrt(pow2(nub - 0.1, 2) + 1) + nub - 0.1); // Eq (12), (20)
    }

    // For Luc calculated using either (16) or (20), Bullington diffraction loss
    // for the path is given by
    output->Lbull = Luc + (1 - exp(-Luc / 6.0)) * (10 + 0.02 * input->dtot); // Eq(21)
}
