#include "calculation.h"
#include "rf.h"

#include "beta0.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

const double ER = 6371.0; // Earth radius [km]

void smooth_earth_heights(c1812_calculation_context_t *ctx)
{
	ctx->hts = ctx->h[0] + ctx->htg;
	ctx->hrs = ctx->h[ctx->n - 1] + ctx->hrg;

	// Modify the path by adding representative clutter, according to Section 3.2
	// excluding the first and the last point
	double *g = (double *)malloc(sizeof(double) * ctx->n);
	for (int i = 1; i < ctx->n - 1; i++)
		g[i] = ctx->h[i] + ((ctx->Ct != NULL) ? ctx->Ct[i] : 0.0);
	g[0] = ctx->h[0];
	g[ctx->n - 1] = ctx->h[ctx->n - 1];
	ctx->g = g;

	// Compute htc and hrc as defined in Table 5 (P.1812-6)
	ctx->htc = ctx->hts;
	ctx->hrc = ctx->hrs;

	double v1 = 0.0;
	double v2 = 0.0;
	for (int i = 0; i < ctx->n - 1; i++)
	{
		double diff_d = ctx->d[i + 1] - ctx->d[i];
		v1 += diff_d * (ctx->h[i + 1] + ctx->h[i]);
		v2 += diff_d * (ctx->h[i + 1] * (2 * ctx->d[i + 1] + ctx->d[i]) + ctx->h[i] * (ctx->d[i + 1] + 2 * ctx->d[i]));
	}

	ctx->hst = (2 * v1 * ctx->dtot - v2) / pow(ctx->dtot, 2);
	ctx->hsr = (v2 - v1 * ctx->dtot) / pow(ctx->dtot, 2);

	// Section 5.6.2 Smooth-surface heights for the diffraction model

	double *HH = (double *)malloc(sizeof(double) * ctx->n);
	for (int i = 0; i < ctx->n; i++)
		HH[i] = ctx->h[i] - (ctx->htc * (ctx->dtot - ctx->d[i]) + ctx->hrc * ctx->d[i]) / ctx->dtot;

	double hobs = 0;
	for (int i = 1; i < ctx->n - 1; i++)
		hobs = fmax(hobs, HH[i]);

	double alpha_obt = 0;
	for (int i = 1; i < ctx->n - 1; i++)
		alpha_obt = fmax(alpha_obt, HH[i] / ctx->d[i]);

	double alpha_obr = 0;
	for (int i = 1; i < ctx->n - 1; i++)
		alpha_obr = fmax(alpha_obr, HH[i] / (ctx->dtot - ctx->d[i]));

	// Calculate provisional values for the Tx and Rx smooth surface heights

	double gt = alpha_obt / (alpha_obt + alpha_obr);
	double gr = alpha_obr / (alpha_obt + alpha_obr);

	if (hobs <= 0)
	{
		ctx->hstp = ctx->hst;
		ctx->hsrp = ctx->hsr;
	}
	else
	{
		ctx->hstp = ctx->hst - hobs * gt;
		ctx->hsrp = ctx->hsr - hobs * gr;
	}

	// calculate the final values as required by the diffraction model

	ctx->hstd = (ctx->hstp >= ctx->h[0]) ? ctx->h[0] : ctx->hstp;
	ctx->hsrd = (ctx->hsrp > ctx->h[ctx->n - 1]) ? ctx->h[ctx->n - 1] : ctx->hsrp;

	// Interfering antenna horizon elevation angle and distance

	double *theta = (double *)malloc(sizeof(double) * (ctx->n - 2));
	for (int i = 1; i < ctx->n - 1; i++)
		theta[i - 1] = 1000 * atan((ctx->h[i] - ctx->hts) / (1000 * ctx->d[i]) - ctx->d[i] / (2 * ER));
	double theta_td = 1000 * atan((ctx->hrs - ctx->hts) / (1000 * ctx->dtot) - ctx->dtot / (2 * ER));
	double theta_rd = 1000 * atan((ctx->hts - ctx->hrs) / (1000 * ctx->dtot) - ctx->dtot / (2 * ER));
	double theta_max = 0;
	for (int i = 0; i < ctx->n - 2; i++)
		theta_max = fmax(theta_max, theta[i]);

	int pathtype = (theta_max > theta_td) ? 2 : 1;

	double theta_t = fmax(theta_max, theta_td);

	int lt = 0;
	int lr = 0;

	double theta_r;
	if (pathtype == 2)
	{ // transhorizon
		theta_r = 0;
		for (int i = 1; i < ctx->n - 1; i++)
		{
			double temp = (ctx->h[i] - ctx->hrs) / (1000 * (ctx->dtot - ctx->d[i])) - (ctx->dtot - ctx->d[i]) / (2 * ER);
			theta[i - 1] = 1000 * atan(temp);
			theta_r = fmax(theta_r, theta[i - 1]);
		}

		int kindex = 0;
		double numax = 0;
		for (int i = 0; i < ctx->n - 2; i++)
		{
			double temp = (ctx->h[i + 1] + 500 * ER * (ctx->d[i + 1] * (ctx->dtot - ctx->d[i + 1]) - ctx->d[i] * (ctx->dtot - ctx->d[i])) - (ctx->hts * (ctx->dtot - ctx->d[i]) + ctx->hrs * ctx->d[i]) / ctx->dtot) * sqrt(0.002 * ctx->dtot / (ctx->lambda * ctx->d[i] * (ctx->dtot - ctx->d[i])));
			if (temp > numax)
			{
				numax = temp;
				kindex = i;
			}
		}

		lt = kindex + 1;
		ctx->dlt = ctx->d[lt];
		ctx->dlr = ctx->dtot - ctx->dlt;
		lr = lt;
	}
	else
	{ // pathtype == 1 (LoS)
		theta_r = theta_rd;
		double *nu = (double *)malloc(sizeof(double) * (ctx->n - 2));
		for (int i = 1; i < ctx->n - 1; i++)
		{
			double temp = (ctx->h[i] + 500 * ER * ctx->d[i] * (ctx->dtot - ctx->d[i]) - (ctx->hts * (ctx->dtot - ctx->d[i]) + ctx->hrs * ctx->d[i]) / ctx->dtot) * sqrt(0.002 * ctx->dtot / (ctx->lambda * ctx->d[i] * (ctx->dtot - ctx->d[i])));
			nu[i - 1] = temp;
		}

		int kindex = 0;
		double numax = 0;
		for (int i = 0; i < ctx->n - 2; i++)
		{
			if (nu[i] > numax)
			{
				numax = nu[i];
				kindex = i;
			}
		}

		lt = kindex + 2;
		ctx->dlt = ctx->d[lt - 1];
		ctx->dlr = ctx->dtot - ctx->dlt;
		lr = lt;
		free(nu);
	}

	// Angular distance
	double theta_tot = 1e3 * ctx->dtot / ER + theta_t + theta_r;
	ctx->theta = theta_tot;

	// Section 5.6.3 Ducting/layer-reflection model

	// Calculate the smooth-Earth heights at transmitter and receiver as
	// required for the roughness factor
	double hst_smooth = fmin(ctx->hst, ctx->h[0]);
	double hsr_smooth = fmin(ctx->hsr, ctx->h[ctx->n - 1]);

	// Slope of the smooth-Earth surface
	double m = (hsr_smooth - hst_smooth) / ctx->dtot;

	// The terminal effective heigts for the ducting/layer-reflection model
	ctx->hte = ctx->htg + ctx->h[0] - hst_smooth;
	ctx->hre = ctx->hrg + ctx->h[ctx->n - 1] - hsr_smooth;

	double hm = 0;
	for (int i = lt; i <= lr; i++)
		hm = fmax(hm, ctx->h[i] - (hst_smooth + m * ctx->d[i]));

	free(HH);
	free(theta);
}

void pl_los(c1812_calculation_context_t *ctx, double *Lbfs, double *Lb0p, double *Lb0b)
{
	double dfs = sqrt(pow(ctx->dtot, 2) + pow((ctx->hts - ctx->hrs) / 1000.0, 2)); // (8a)

	// Free space loss
	*Lbfs = 92.4 + 20.0 * log10(ctx->f) + 20.0 * log10(ctx->dtot); // (8)

	// Corrections for multipath and focusing effects at p and b0
	double Esp = 2.6 * (1 - exp(-0.1 * (ctx->dlt + ctx->dlr))) * log10(ctx->p / 50);  // (9a)
	double Esb = 2.6 * (1 - exp(-0.1 * (ctx->dlt + ctx->dlr))) * log10(ctx->b0 / 50); // (9b)

	// Basic transmission loss not exceeded for time percentage p% due to
	// LoS propagation
	*Lb0p = *Lbfs + Esp; // (10)

	// Basic transmission loss not exceeded for time percentage b0% due to
	// LoS propagation
	*Lb0b = *Lbfs + Esb; // (11)
}

void dl_p(c1812_calculation_context_t *ctx, double *Ldp, double *Ldb, double *Ld50, double *Lbulla50, double *Lbulls50, double *Ldsph50)
{
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
	smooth_earth_heights(&ctx);

	// // Calculate an interpolation factor Fj to take account of the path angular
	// // distance Eq (57)
	// double THETA = 0.3;
	// double KSI = 0.8;
	// double Fj = 1.0 - 0.5 * (1.0 + tanh(3.0 * KSI * (ctx.theta - THETA) / THETA));

	// // Calculate an interpolation factor, Fk, to take account of the great
	// // circle path distance:
	// double DSW = 20;
	// double KAPPA = 0.5;
	// double Fk = 1.0 - 0.5 * (1.0 + tanh(3.0 * KAPPA * (ctx.dtot - DSW) / DSW)); // eq (58)

	double Lbfs, Lb0p, Lb0b;
	pl_los(&ctx, &Lbfs, &Lb0p, &Lb0b);

	double Ldp, Ldb, Ld50, Lbulla50, Lbulls50, Ldsph50;
	dl_p(&ctx, &Ldp, &Ldb, &Ld50, &Lbulla50, &Lbulls50, &Ldsph50);

	// The median basic transmission loss associated with diffraction Eq (42)
	double Lbd50 = Lbfs + Ld50;

	// Basic transmission loss not exceeded for p% time
	results->Lb = fmax(Lb0p, Lbd50); // eq (69)

	results->error = RESULTS_ERR_NONE;
}