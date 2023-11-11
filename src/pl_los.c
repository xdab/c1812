#include "pl_los.h"
#include "custom_math.h"

void pl_los(pl_los_input_t *input, pl_los_output_t *output)
{
    double dfs, Esp, Esb;

    // Basic transmission loss due to free-space propagation
    dfs = c_sqrt(c_pow(input->d, 2) + c_pow((input->hts - input->hrs) / 1000.0, 2)); // (8a)
    output->Lbfs = 92.4 + 20.0 * c_log10(input->f) + 20.0 * c_log10(dfs);            // (8)

    // Corrections for multipath and focusing effects at p and b0
    Esp = 2.6 * (1 - c_exp(-0.1 * (input->dlt + input->dlr))) * c_log10(input->p / 50);  // (9a)
    Esb = 2.6 * (1 - c_exp(-0.1 * (input->dlt + input->dlr))) * c_log10(input->b0 / 50); // (9b)

    // Basic transmission loss not exceeded for time percentage p% due to LoS propagation
    output->Lb0p = output->Lbfs + Esp; // (10)

    // Basic transmission loss not exceeded for time percentage b0% due to LoS propagation
    output->Lb0b = output->Lbfs + Esb; // (11)
}
