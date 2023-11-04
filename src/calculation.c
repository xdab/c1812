#include "calculation.h"
#include "rf.h"

#include "constants.h"
#include "beta0.h"
#include "smooth_earth_heights.h"
#include "pl_los.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

void copy_ctx_to_seh_input(c1812_calculation_context_t *ctx, seh_input_t *input)
{
	input->n = ctx->n;
	input->d = ctx->d;
	input->h = ctx->h;
	input->Ct = ctx->Ct;
	input->dtot = ctx->dtot;
	input->lambda = ctx->lambda;
	input->htg = ctx->htg;
	input->hrg = ctx->hrg;
}

void copy_seh_output_to_ctx(seh_output_t *output, c1812_calculation_context_t *ctx)
{
	ctx->htc = output->htc;
	ctx->hrc = output->hrc;
	ctx->hst = output->hst;
	ctx->hsr = output->hsr;
	ctx->hstp = output->hstp;
	ctx->hsrp = output->hsrp;
	ctx->hstd = output->hstd;
	ctx->hsrd = output->hsrd;
	ctx->hte = output->hte;
	ctx->hre = output->hre;
	ctx->theta = output->theta;
	ctx->dlt = output->dlt;
	ctx->dlr = output->dlr;
}

void copy_ctx_to_pl_los_input(c1812_calculation_context_t *ctx, pl_los_input_t *input)
{
	input->d = ctx->dtot;
	input->hts = ctx->hts;
	input->hrs = ctx->hrs;
	input->f = ctx->f;
	input->p = ctx->p;
	input->b0 = ctx->b0;
	input->dlt = ctx->dlt;
	input->dlr = ctx->dlr;
}

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

	c1812_calculation_context_t ctx;
	ctx.p = parameters->p;
	ctx.f = parameters->f;
	ctx.lambda = 299792458.0 / (parameters->f * 1e9);

	ctx.htg = parameters->htg;
	ctx.hrg = parameters->hrg;
	ctx.DN = parameters->DN;

	ctx.n = parameters->n;
	ctx.d = parameters->d;
	ctx.h = parameters->h;

	// Compute dtot - the total great-circle path distance (km)
	ctx.dtot = parameters->d[parameters->n - 1] - parameters->d[0];

	// Compute dtm - the longest continuous land (inland + coastal =34) section of the great-circle path (km)
	ctx.dtm = ((parameters->zone == RC_ZONE_INLAND) || (parameters->zone == RC_ZONE_COASTAL_LAND)) ? ctx.dtot : 0.0;

	// Compute dlm - the longest continuous inland section (4) of the great-circle path (km)
	ctx.dlm = (parameters->zone == RC_ZONE_INLAND) ? ctx.dtot : 0.0;

	// Compute b0
	ctx.b0 = beta0(parameters->lat, ctx.dtm, ctx.dlm);

	// Compute ae and ab
	ctx.ae = ER * (157 / (157 - ctx.DN)); // (6) (7a)
	ctx.ab = ER * 3;					  // (7b)

	// Compute the path fraction over sea Eq (1)
	double omega = 0.0;

	// Derive parameters for the path profile analysis
	seh_input_t seh_input;
	copy_ctx_to_seh_input(&ctx, &seh_input);

	seh_output_t seh_output;
	smooth_earth_heights(&seh_input, &seh_output);
	copy_seh_output_to_ctx(&seh_output, &ctx);

	// pl_los
	pl_los_input_t pl_los_input;
	copy_ctx_to_pl_los_input(&ctx, &pl_los_input);

	pl_los_output_t pl_los_output;
	pl_los(&pl_los_input, &pl_los_output);
	results->Lb = fmax(pl_los_output.Lb0p, pl_los_output.Lbfs); // TODO temporary

	// double Ldp, Ldb, Ld50, Lbulla50, Lbulls50, Ldsph50;
	// dl_p(&ctx, &Ldp, &Ldb, &Ld50, &Lbulla50, &Lbulls50, &Ldsph50);

	// // The median basic transmission loss associated with diffraction Eq (42)
	// double Lbd50 = Lbfs + Ld50;

	// // Basic transmission loss not exceeded for p% time
	// results->Lb = fmax(Lb0p, Lbd50); // eq (69)

	results->error = RESULTS_ERR_NONE;
}