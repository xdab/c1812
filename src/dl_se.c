#include "dl_se.h"
#include "dl_se_ft.h"
#include "pow.h"
#include <math.h>
#include <stdio.h>

void dl_se(dl_se_input_t *input, dl_se_output_t *output)
{
    dl_se_ft_output_t dl_se_ft_output;
    dl_se_ft_input_t dl_se_ft_input;
    dl_se_ft_input.d = input->d;
    dl_se_ft_input.hte = input->hte;
    dl_se_ft_input.hre = input->hre;
    dl_se_ft_input.f = input->f;
    dl_se_ft_input.lambda = input->lambda;
    dl_se_ft_input.omega = input->omega;

    // Calculate the marginal LoS distance for a smooth path
    double dlos = sqrt(2 * input->ap) * (sqrt(0.001 * input->hte) + sqrt(0.001 * input->hre)); // Eq (22)

    if (input->d >= dlos)
    {
        // calculate diffraction loss Ldft using the method in Sec. 4.3.3 for
        // adft = ap and set Ldsph to Ldft
        dl_se_ft_input.ap = input->ap;
        dl_se_ft(&dl_se_ft_input, &dl_se_ft_output);
        output->Ldsph[0] = dl_se_ft_output.Ldft[0];
        output->Ldsph[1] = dl_se_ft_output.Ldft[1];
        return;
    }

    // calculate the smallest clearance between the curved-Earth path and
    // the ray between the antennas, hse
    double c = (input->hte - input->hre) / (input->hte + input->hre);               // Eq (24d)
    double m = 250 * input->d * input->d / (input->ap * (input->hte + input->hre)); // Eq (24e)

    double b = 2 * sqrt((m + 1) / (3 * m)) * cos(M_PI / 3 + 1.0 / 3.0 * acos(3 * c / 2 * sqrt(3 * m / pow2((m + 1), 3)))); // Eq (24c)

    double dse1 = input->d / 2 * (1 + b); // Eq (24a)
    double dse2 = input->d - dse1;        // Eq (24b)

    double hse = (input->hte - 500 * dse1 * dse1 / input->ap) * dse2 + (input->hre - 500 * dse2 * dse2 / input->ap) * dse1;
    hse = hse / input->d; // Eq (23)

    // Calculate the required clearance for zero diffraction loss
    double hreq = 17.456 * sqrt(dse1 * dse2 * input->lambda / input->d); // Eq (26)
    if (hse > hreq)
    {
        output->Ldsph[0] = 0;
        output->Ldsph[1] = 0;
        return;
    }

    // calculate the modified effective Earth radius aem, which gives
    // marginal LoS at distance d
    double aem = 500 * pow2(input->d / (sqrt(input->hte) + sqrt(input->hre)), 2); // Eq (26)
    
    // Use the method in Sec. 4.3.3 for adft = aem to obtain Ldft
    dl_se_ft_input.ap = aem;
    dl_se_ft(&dl_se_ft_input, &dl_se_ft_output);

    dl_se_ft_output.Ldft[0] = fmax(dl_se_ft_output.Ldft[0], 0);
    dl_se_ft_output.Ldft[1] = fmax(dl_se_ft_output.Ldft[1], 0);

    output->Ldsph[0] = (1 - hse / hreq) * dl_se_ft_output.Ldft[0];
    output->Ldsph[1] = (1 - hse / hreq) * dl_se_ft_output.Ldft[1];
}