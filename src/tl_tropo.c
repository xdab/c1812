#include "tl_tropo.h"
#include "pow.h"
#include <math.h>

void tl_tropo(tl_tropo_input_t *input, tl_tropo_output_t *output)
{
	// Frequency dependent loss
	double Lf = 25 * log10(input->f) - 2.5 * pow(log10(input->f / 2.0), 2); // eq (45)

	// the basic transmission loss due to troposcatter not exceeded for any time
	// percentage p, below 50# is given
	output->Lbs = 190.1 + Lf + 20 * log10(input->dtot) + 0.573 * input->theta - 0.15 * input->N0 - 10.125 * pow2(log10(50.0 / input->p), 0.7);
}