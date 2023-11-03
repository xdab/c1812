#include "calculation.h"
#include "rf.h"

void c1812_calculate(c1812_parameters_t *parameters, c1812_results_t *results)
{
	results->error = RESULTS_ERR_UNKNOWN;

	// Validate parameters
	c1812_parameters_error_t param_err = c1812_validate_parameters(parameters);
	if (param_err != PARAM_ERR_NONE)
	{
		results->error = param_err;
		return;
	}

	// Calculate basic transmission loss
	double Lfspl = fspl(parameters->d[parameters->n - 1], parameters->f);
	results->Lb = Lfspl;

	results->error = RESULTS_ERR_NONE;
}