#include "calculation.h"
#include "rf.h"

#include "constants.h"
#include "beta0.h"
#include "smooth_earth_heights.h"
#include "pl_los.h"
#include "dl_p.h"
#include "inv_cum_norm.h"

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
	ctx->g = output->g;
	ctx->htc = output->htc;
	ctx->hrc = output->hrc;
	ctx->hts = output->hts;
	ctx->hrs = output->hrs;
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

void copy_ctx_to_dl_p_input(c1812_calculation_context_t *ctx, dl_p_input_t *input)
{
	input->n = ctx->n;
	input->d = ctx->d;
	input->g = ctx->g;
	input->hts = ctx->hts;
	input->hrs = ctx->hrs;
	input->hstd = ctx->hstd;
	input->hsrd = ctx->hsrd;
	input->ae = ctx->ae;
	input->ab = ctx->ab;
	input->f = ctx->f;
	input->lambda = ctx->lambda;
	input->dtot = ctx->dtot;
	input->omega = ctx->omega;
	input->p = ctx->p;
	input->b0 = ctx->b0;
	input->DN = ctx->DN;
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
	ctx.lambda = 0.2998 / ctx.f;

	ctx.htg = parameters->htg;
	ctx.hrg = parameters->hrg;
	ctx.DN = parameters->DN;

	ctx.n = parameters->n;
	ctx.d = parameters->d;
	ctx.h = parameters->h;
	ctx.Ct = parameters->Ct;

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
	ctx.omega = 0.0; // TEMPORARY

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

	// dl_p
	dl_p_input_t dl_p_input;
	copy_ctx_to_dl_p_input(&ctx, &dl_p_input);
	dl_p_output_t dl_p_output;
	dl_p(&dl_p_input, &dl_p_output);

	// The median basic transmission loss associated with diffraction Eq (42)
	double Lbd50[2];
	Lbd50[0] = pl_los_output.Lbfs + dl_p_output.Ld50[0];
	Lbd50[1] = pl_los_output.Lbfs + dl_p_output.Ld50[1];

	// The basic tranmission loss associated with diffraction not exceeded for
	// p% time Eq (43)
	double Lbd[2];
	Lbd[0] = pl_los_output.Lb0p + dl_p_output.Ldp[0];
	Lbd[1] = pl_los_output.Lb0p + dl_p_output.Ldp[1];

#ifdef EXTRA
	// A notional minimum basic transmission loss associated with LoS
	// propagation and over-sea sub-path diffraction
	double Lminb0p[2];
	Lminb0p[0] = pl_los_output.Lb0p + (1 - ctx.omega) * dl_p_output.Ldp[0];
	Lminb0p[1] = pl_los_output.Lb0p + (1 - ctx.omega) * dl_p_output.Ldp[1];

	// eq (40a)
	double Fi = 1;
	if (ctx.p >= ctx.b0)
	{
		Fi = inv_cum_norm(ctx.p / 100) / inv_cum_norm(ctx.b0 / 100);
		// eq(59)
		Lminb0p[0] = Lbd50[0] + (pl_los_output.Lb0b + (1 - ctx.omega) * dl_p_output.Ldp[0] - Lbd50[0]) * Fi;
		Lminb0p[1] = Lbd50[1] + (pl_los_output.Lb0b + (1 - ctx.omega) * dl_p_output.Ldp[1] - Lbd50[1]) * Fi;
	}
#endif

	results->Lb = fmax(pl_los_output.Lb0p, Lbd[(int)parameters->pol]);
	results->error = RESULTS_ERR_NONE;
}