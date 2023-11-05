#include "dl_se_ft_inner.h"
#include <math.h>

// Eq (30)
#define BETA_DFT_FORMULA(k) ((1 + 1.6 * pow(k, 2) + 0.67 * pow(k, 4)) / (1 + 4.5 * pow(k, 2) + 1.53 * pow(k, 4)))

// Eq (31)
#define X_FORMULA(beta_dft, adft, f, d) (21.88 * beta_dft * pow(f / pow(adft, 2), 1.0 / 3.0) * d)

// Eq (32ab)
#define Y_FORMULA(beta_dft, adft, f, h) (0.9575 * beta_dft * pow(pow(f, 2) / adft, 1.0 / 3.0) * h)

void dl_se_ft_inner(dl_se_ft_inner_input_t *input, dl_se_ft_inner_output_t *output)
{
    // Normalized factor for surface admittance for horizontal (1) and vertical (2) polarizations
    double K[2];
    K[0] = 0.036 * pow(input->adft * input->f, -1.0 / 3.0) * pow(pow(input->epsr - 1, 2) + pow(18 * input->sigma / input->f, 2), -1.0 / 4.0); // Eq (29a)
    K[1] = K[0] * sqrt(pow(input->epsr, 2) + pow(18 * input->sigma / input->f, 2));                                                           // Eq (29b)

    // Earth ground/polarization parameter
    double beta_dft[2];
    beta_dft[0] = BETA_DFT_FORMULA(K[0]);
    beta_dft[1] = BETA_DFT_FORMULA(K[1]);

    // Normalized distance
    double X[2];
    X[0] = X_FORMULA(beta_dft[0], input->adft, input->f, input->d);
    X[1] = X_FORMULA(beta_dft[1], input->adft, input->f, input->d);

    // Normalized transmitter and receiver heights
    double Yt[2];
    double Yr[2];
    Yt[0] = Y_FORMULA(beta_dft[0], input->adft, input->f, input->hte);
    Yt[1] = Y_FORMULA(beta_dft[1], input->adft, input->f, input->hte);
    Yr[0] = Y_FORMULA(beta_dft[0], input->adft, input->f, input->hre);
    Yr[1] = Y_FORMULA(beta_dft[1], input->adft, input->f, input->hre);

    double Fx[2] = {0, 0};
    double GYt[2] = {0, 0};
    double GYr[2] = {0, 0};

    // Calculate the distance term given by:
    for (int ii = 0; ii < 2; ii++)
    {
        if (X[ii] >= 1.6)
        {
            Fx[ii] = 11 + 10 * log10(X[ii]) - 17.6 * X[ii];
        }
        else
        {
            Fx[ii] = -20 * log10(X[ii]) - 5.6488 * pow(X[ii], 1.425); // Eq (33)
        }
    }

    double Bt[2];
    double Br[2];
    for (int ii = 0; ii < 2; ii++)
    {
        Bt[ii] = beta_dft[ii] * Yt[ii]; // Eq (35)
        Br[ii] = beta_dft[ii] * Yr[ii]; // Eq (35)

        if (Bt[ii] > 2)
        {
            GYt[ii] = 17.6 * sqrt(Bt[ii] - 1.1) - 5 * log10(Bt[ii] - 1.1) - 8;
        }
        else
        {
            GYt[ii] = 20 * log10(Bt[ii] + 0.1 * pow(Bt[ii], 3));
        }

        if (Br[ii] > 2)
        {
            GYr[ii] = 17.6 * sqrt(Br[ii] - 1.1) - 5 * log10(Br[ii] - 1.1) - 8;
        }
        else
        {
            GYr[ii] = 20 * log10(Br[ii] + 0.1 * pow(Br[ii], 3));
        }

        if (GYr[ii] < 2 + 20 * log10(K[ii]))
        {
            GYr[ii] = 2 + 20 * log10(K[ii]);
        }

        if (GYt[ii] < 2 + 20 * log10(K[ii]))
        {
            GYt[ii] = 2 + 20 * log10(K[ii]);
        }
    }

    // Eq (36)
    output->Ldft[0] = -Fx[0] - GYt[0] - GYr[0];
    output->Ldft[1] = -Fx[1] - GYt[1] - GYr[1];
}