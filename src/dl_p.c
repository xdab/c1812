#include "dl_p.h"
#include "dl_delta_bull.h"
#include "inv_cum_norm.h"

void dl_p(dl_p_input_t *input, dl_p_output_t *output)
{
    dl_delta_bull_output_t dl_delta_bull_output;
    dl_delta_bull_input_t dl_delta_bull_input;
    dl_delta_bull_input.n = input->n;
    dl_delta_bull_input.d = input->d;
    dl_delta_bull_input.g = input->g;
    dl_delta_bull_input.hts = input->hts;
    dl_delta_bull_input.hrs = input->hrs;
    dl_delta_bull_input.hstd = input->hstd;
    dl_delta_bull_input.hsrd = input->hsrd;
    dl_delta_bull_input.f = input->f;
    dl_delta_bull_input.lambda = input->lambda;
    dl_delta_bull_input.dtot = input->dtot;
    dl_delta_bull_input.omega = input->omega;

    // Use the method in 4.3.4 to calculate diffraction loss Ld for median effective
    // Earth radius ap = ae as given by equation (7a). Set median diffraction
    // loss to Ldp50
    dl_delta_bull_input.ap = input->ae;
    dl_delta_bull(&dl_delta_bull_input, &dl_delta_bull_output);

    output->Ld50[0] = dl_delta_bull_output.Ld[0];
    output->Ld50[1] = dl_delta_bull_output.Ld[1];
    output->Lbulla50 = dl_delta_bull_output.Lbulla;
    output->Lbulls50 = dl_delta_bull_output.Lbulls;
    output->Ldsph50[0] = dl_delta_bull_output.Ldsph[0];
    output->Ldsph50[1] = dl_delta_bull_output.Ldsph[1];

    if (input->p == 50)
    {
        output->Ldp[0] = output->Ld50[0];
        output->Ldp[1] = output->Ld50[1];

        dl_delta_bull_input.ap = input->ab;
        dl_delta_bull(&dl_delta_bull_input, &dl_delta_bull_output);

        output->Ldb[0] = dl_delta_bull_output.Ld[0];
        output->Ldb[1] = dl_delta_bull_output.Ld[1];
        output->Lbulla50 = dl_delta_bull_output.Lbulla;
        output->Lbulls50 = dl_delta_bull_output.Lbulls;
        output->Ldsph50[0] = dl_delta_bull_output.Ldsph[0];
        output->Ldsph50[1] = dl_delta_bull_output.Ldsph[1];
    }
    else if (input->p < 50)
    {
        // Use the method in 4.3.4 to calculate diffraction loss Ld for effective
        // Earth radius ap = abeta, as given in equation (7b). Set diffraction loss
        // not exceeded for beta0% time Ldb = Ld
        dl_delta_bull_input.ap = input->ab;
        dl_delta_bull(&dl_delta_bull_input, &dl_delta_bull_output);

        output->Ldb[0] = dl_delta_bull_output.Ld[0];
        output->Ldb[1] = dl_delta_bull_output.Ld[1];
        output->Lbulla50 = dl_delta_bull_output.Lbulla;
        output->Lbulls50 = dl_delta_bull_output.Lbulls;
        output->Ldsph50[0] = dl_delta_bull_output.Ldsph[0];
        output->Ldsph50[1] = dl_delta_bull_output.Ldsph[1];

        // Compute the interpolation factor Fi
        double Fi;
        if (input->p > input->b0)
            Fi = inv_cum_norm(input->p / 100) / inv_cum_norm(input->b0 / 100); // eq (40a)
        else
            Fi = 1; // eq (40a)

        // The diffraction loss Ldp not exceeded for p% of time is now given by
        // Ldp = Ld50 + Fi * (Ldb - Ld50) // eq (41)
        output->Ldp[0] = output->Ld50[0] + Fi * (output->Ldb[0] - output->Ld50[0]);
        output->Ldp[1] = output->Ld50[1] + Fi * (output->Ldb[1] - output->Ld50[1]);
    }
}
