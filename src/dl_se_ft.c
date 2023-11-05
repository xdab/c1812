#include "dl_se_ft.h"
#include "dl_se_ft_inner.h"
#include <math.h>

void dl_se_ft(dl_se_ft_input_t *input, dl_se_ft_output_t *output)
{
    dl_se_ft_inner_output_t dl_se_ft_inner_output;
    dl_se_ft_inner_input_t dl_se_ft_inner_input;
    dl_se_ft_inner_input.d = input->d;
    dl_se_ft_inner_input.hte = input->hte;
    dl_se_ft_inner_input.hre = input->hre;
    dl_se_ft_inner_input.adft = input->ap;
    dl_se_ft_inner_input.f = input->f;

    // First-term part of the spherical-Earth diffraction loss over land
    dl_se_ft_inner_input.epsr = 22;
    dl_se_ft_inner_input.sigma = 0.003;
    dl_se_ft_inner(&dl_se_ft_inner_input, &dl_se_ft_inner_output);
    double Ldft_land[2];
    Ldft_land[0] = dl_se_ft_inner_output.Ldft[0];
    Ldft_land[1] = dl_se_ft_inner_output.Ldft[1];

    // First-term part of the spherical-Earth diffraction loss over sea
    dl_se_ft_inner_input.epsr = 80;
    dl_se_ft_inner_input.sigma = 5;
    dl_se_ft_inner(&dl_se_ft_inner_input, &dl_se_ft_inner_output);
    double Ldft_sea[2];
    Ldft_sea[0] = dl_se_ft_inner_output.Ldft[0];
    Ldft_sea[1] = dl_se_ft_inner_output.Ldft[1];

    // First-term spherical diffraction loss, Eq (28)
    output->Ldft[0] = input->omega * Ldft_sea[0] + (1 - input->omega) * Ldft_land[0];
    output->Ldft[1] = input->omega * Ldft_sea[1] + (1 - input->omega) * Ldft_land[1];
}
