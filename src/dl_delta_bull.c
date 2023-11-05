#include "dl_delta_bull.h"
#include "dl_bull.h"
#include "dl_se.h"
#include <math.h>
#include <stdlib.h>

void dl_delta_bull(dl_delta_bull_input_t *input, dl_delta_bull_output_t *output)
{
    dl_bull_output_t dl_bull_output;
    dl_bull_input_t dl_bull_input;
    dl_bull_input.n = input->n;
    dl_bull_input.d = input->d;
    dl_bull_input.g = input->g;
    dl_bull_input.hts = input->hts;
    dl_bull_input.hrs = input->hrs;
    dl_bull_input.ap = input->ap;
    dl_bull_input.f = input->f;
    dl_bull_input.lambda = input->lambda;
    dl_bull_input.dtot = input->dtot;

    // Use the method in 4.3.1 for the actual terrain profile and antenna
    // heights. Set the resulting Bullington diffraction loss for the actual
    // path to Lbulla
    dl_bull(&dl_bull_input, &dl_bull_output);
    output->Lbulla = dl_bull_output.Lbull;

    // Use the method in 4.3.1 for a second time, with all profile heights gi
    // set to zero and modified antenna heights given by
    // hts1 = hts - hstd;   % eq (37a)
    // hrs1 = hrs - hsrd;   % eq (37b)
    // h1 = zeros(size(g));
    // where hstd and hsrd are given in 5.6.2 of Attachment 1.
    // Set the resulting Bullington diffraction loss for this smooth path to Lbulls
    double hts1 = input->hts - input->hstd;
    double hrs1 = input->hrs - input->hsrd;
    dl_bull_input.g = NULL;
    dl_bull_input.hts = hts1;
    dl_bull_input.hrs = hrs1;
    dl_bull(&dl_bull_input, &dl_bull_output);
    output->Lbulls = dl_bull_output.Lbull;

    // Use the method in 4.3.2 to calculate the spherical-Earth diffraction loss
    // for the actual path length (dtot) with
    // hte = hts1;             % eq (38a)
    // hre = hrs1;             % eq (38b)
    double hte = hts1;
    double hre = hrs1;
    dl_se_output_t dl_se_output;
    dl_se_input_t dl_se_input;
    dl_se_input.d = input->dtot;
    dl_se_input.hte = hte;
    dl_se_input.hre = hre;
    dl_se_input.ap = input->ap;
    dl_se_input.f = input->f;
    dl_se_input.lambda = input->lambda;
    dl_se_input.omega = input->omega;
    dl_se(&dl_se_input, &dl_se_output);
    output->Ldsph[0] = dl_se_output.Ldsph[0];
    output->Ldsph[1] = dl_se_output.Ldsph[1];

    // Diffraction loss for the general path is now given by
    // Ld(1) = Lbulla + max(Ldsph(1) - Lbulls, 0);  % eq (39)
    // Ld(2) = Lbulla + max(Ldsph(2) - Lbulls, 0);  % eq (39)
    output->Ld[0] = output->Lbulla + fmax(output->Ldsph[0] - output->Lbulls, 0);
    output->Ld[1] = output->Lbulla + fmax(output->Ldsph[1] - output->Lbulls, 0);
}
