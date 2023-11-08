#include "tl_anomalous.h"
#include <math.h>

void tl_anomalous(tl_anomalous_input_t *input, tl_anomalous_output_t *output)
{
    // empirical correction to account for the increasing attenuation with
    // wavelength inducted propagation (47a)
    double Alf = 0;
    if (input->f < 0.5)
        Alf = 45.375 - input->f * (137.0 + 92.5 * input->f);

    // site-shielding diffraction losses for the interfering and interfered-with
    // stations (48)
    double theta_t2 = input->theta_t - 0.1 * input->dlt; // eq (48a)
    double Ast = 0;
    if (theta_t2 > 0)
        Ast = 20 * log10(1 + 0.361 * theta_t2 * sqrt(input->f * input->dlt)) + 0.264 * theta_t2 * cbrt(input->f);

    double theta_r2 = input->theta_r - 0.1 * input->dlr;
    double Asr = 0;
    if (theta_r2 > 0)
        Asr = 20 * log10(1 + 0.361 * theta_r2 * sqrt(input->f * input->dlr)) + 0.264 * theta_r2 * cbrt(input->f);

    // over-sea surface duct coupling correction for the interfering and
    // interfered-with stations (49) and (49a)

    double Act = 0;
    double Acr = 0;
    if (input->dct <= 5)
        if (input->dct <= input->dlt)
            if (input->omega >= 0.75)
                Act = -3 * exp(-0.25 * input->dct * input->dct) * (1 + tanh(0.07 * (50 - input->hts)));

    if (input->dcr <= 5)
        if (input->dcr <= input->dlr)
            if (input->omega >= 0.75)
                Acr = -3 * exp(-0.25 * input->dcr * input->dcr) * (1 + tanh(0.07 * (50 - input->hrs)));

    // specific attenuation (51)
    double gamma_d = 5e-5 * input->ae * cbrt(input->f);

    // angular distance (corrected where appropriate) (52-52a)
    double theta_t1 = fmin(input->theta_t, 0.1 * input->dlt);
    double theta_r1 = fmin(input->theta_r, 0.1 * input->dlr);
    double theta1 = 1e3 * input->dtot / input->ae + theta_t1 + theta_r1;

    double dI = fmin(input->dtot - input->dlt - input->dlr, 40); // eq (56a)
    double mu3 = 1;
    if (input->hm > 10)
        mu3 = exp(-4.6e-5 * (input->hm - 10) * (43 + 6 * dI)); // eq (56)

    const double epsilon = 3.5;
    double tau = 1 - exp(-(4.12e-4 * pow(input->dlm, 2.41)));           // eq (3)
    double alpha = -0.6 - epsilon * 1e-9 * pow(input->dtot, 3.1) * tau; // eq (55a)
    alpha = fmax(alpha, -3.4);

    // correction for path geometry:
    double mu2 = (500 / input->ae * pow(input->dtot, 2) / (sqrt(input->hte) + sqrt(input->hre)) * sqrt(input->f)) * alpha; // eq (55)
    mu2 = fmin(mu2, 1);

    double beta = input->b0 * mu2 * mu3;                                                                                                                        // eq (54)
    double Gamma = 1.076 / (2.0058 - log10(beta)) * pow(exp(-(9.51 - 4.8 * log10(beta) + 0.198 * pow(log10(beta), 2)) * 1e-6 * pow(input->dtot, 1.13)), 1.012); // eq (53a)

    // time percentage variablity (cumulative distribution):
    double Ap = -12 + (1.2 + 3.7e-3 * input->dtot) * log10(input->p / beta) + 12 * pow(input->p / beta, Gamma); // eq (53)

    // time percentage and angular-distance dependent losses within the
    // anomalous propagation mechanism
    double Adp = gamma_d * theta1 + Ap; // eq (50)

    // total of fixed coupling losses (except for local clutter losses) between
    // the antennas and the anomalous propagation structure within the
    // atmosphere (47)
    double Af = 102.45 + 20 * log10(input->f) + 20 * log10(input->dlt + input->dlr) + Alf + Ast + Asr + Act + Acr;

    // total basic transmission loss occuring during periods of anomalaous
    // propagation (46)
    output->Lba = Af + Adp;
}