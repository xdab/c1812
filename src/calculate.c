#include "calculate.h"
#include "rf.h"

#include "beta0.h"
#include "smooth_earth_heights.h"
#include "pl_los.h"
#include "dl_p.h"
#include "inv_cum_norm.h"
#include "tl_anomalous.h"
#include "tl_tropo.h"
#include "custom_math.h"

#include <stdlib.h>
#include <stdio.h>

#define ER 6371.0
#define DEBUG 0

typedef struct
{
	// Copied from parameters
	double p;	// time percentage [%]
	double f;	// frequency [GHz]
	int n;		// number of points
	double *d;	// path distances [km]
	double *h;	// path heights [m]
	double *Ct; // representative clutter heights [m]
	double htg; // transmitter height above ground [m]
	double hrg; // receiver height above ground [m]
	double DN;

	// Calculated in c1812_calculate
	double lambda; // wavelength [m]
	double dtot;   // total great-circle path distance [km]
	double dtm;	   // longest continuous land section of the great-circle path [km]
	double dlm;	   // longest continuous inland section of the great-circle path [km]
	double b0;	   // beta0
	double ae, ab;
	double omega; // fraction of the path over sea

	// Calculated in smooth_earth_heights
	double hts; // transmitter height above mean sea level [m]
	double hrs; // receiver height above mean sea level [m]
	double htc;
	double hrc;
	double hst;
	double hsr;
	double hstp;
	double hsrp;
	double hstd;
	double hsrd;
	double hte;
	double hre;
	double theta;
	double dlt;
	double dlr;
	double dct;
	double dcr;
	double hm;
	double theta_t;
	double theta_r;

	// Optional caches
	double *v1_cache;
	double *v2_cache;
	double *theta_max_cache;
	double *theta_r_cache;
	double *kindex_cache;
	double *numax_cache;

} c1812_calculate_ctx_t;

void copy_parameters_to_ctx(c1812_parameters_t *parameters, c1812_calculate_ctx_t *ctx)
{
	ctx->p = parameters->p;
	ctx->f = parameters->f;
	ctx->lambda = 0.2998 / ctx->f;

	ctx->htg = parameters->htg;
	ctx->hrg = parameters->hrg;
	ctx->DN = parameters->DN;

	ctx->n = parameters->n;
	ctx->d = parameters->d;
	ctx->h = parameters->h;
	ctx->Ct = parameters->Ct;

	ctx->v1_cache = parameters->v1_cache;
	ctx->v2_cache = parameters->v2_cache;
	ctx->theta_max_cache = parameters->theta_max_cache;
}

void copy_ctx_to_seh_input(c1812_calculate_ctx_t *ctx, seh_input_t *input)
{
	input->n = ctx->n;
	input->d = ctx->d;
	input->h = ctx->h;
	input->Ct = ctx->Ct;
	input->dtot = ctx->dtot;
	input->lambda = ctx->lambda;
	input->htg = ctx->htg;
	input->hrg = ctx->hrg;
	input->ae = ctx->ae;
	input->v1_cache = ctx->v1_cache;
	input->v2_cache = ctx->v2_cache;
	input->theta_max_cache = ctx->theta_max_cache;
}

void copy_seh_output_to_ctx(seh_output_t *output, c1812_calculate_ctx_t *ctx)
{
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
	ctx->theta_t = output->theta_t;
	ctx->theta_r = output->theta_r;
	ctx->dlt = output->dlt;
	ctx->dlr = output->dlr;
	ctx->hm = output->hm;
}

void copy_ctx_to_pl_los_input(c1812_calculate_ctx_t *ctx, pl_los_input_t *input)
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

void copy_ctx_to_dl_p_input(c1812_calculate_ctx_t *ctx, dl_p_input_t *input)
{
	input->n = ctx->n;
	input->d = ctx->d;
	input->h = ctx->h;
	input->Ct = ctx->Ct;
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

void copy_ctx_to_tl_anomalous_input(c1812_calculate_ctx_t *ctx, tl_anomalous_input_t *input)
{
	input->dtot = ctx->dtot;
	input->dlt = ctx->dlt;
	input->dlr = ctx->dlr;
	input->dct = ctx->dct;
	input->dcr = ctx->dcr;
	input->dlm = ctx->dlm;
	input->hts = ctx->hts;
	input->hrs = ctx->hrs;
	input->hte = ctx->hte;
	input->hre = ctx->hre;
	input->hm = ctx->hm;
	input->theta_t = ctx->theta_t;
	input->theta_r = ctx->theta_r;
	input->f = ctx->f;
	input->p = ctx->p;
	input->omega = ctx->omega;
	input->ae = ctx->ae;
	input->b0 = ctx->b0;
}

void copy_ctx_to_tl_tropo_input(c1812_calculate_ctx_t *ctx, tl_tropo_input_t *input)
{
	input->dtot = ctx->dtot;
	input->theta = ctx->theta;
	input->f = ctx->f;
	input->p = ctx->p;
	input->N0 = ctx->DN;
}

void c1812_calculate(c1812_parameters_t *parameters, c1812_results_t *results)
{
	results->error = RESULTS_ERR_UNKNOWN;

	c1812_calculate_ctx_t ctx;
	copy_parameters_to_ctx(parameters, &ctx);

	// Compute dtot - the total great-circle path distance (km)
	ctx.dtot = parameters->d[parameters->n - 1] - parameters->d[0];

	// Compute dtm - the longest continuous land (inland + coastal =34) section of the great-circle path (km)
	ctx.dtm = ((parameters->zone == RC_ZONE_INLAND) || (parameters->zone == RC_ZONE_COASTAL_LAND)) ? ctx.dtot : 0.0;

	// Compute dlm - the longest continuous inland section (4) of the great-circle path (km)
	ctx.dlm = (parameters->zone == RC_ZONE_INLAND) ? ctx.dtot : 0.0;

	// TODO distance to sea [km]
	ctx.dct = ctx.dcr = 500.0;

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

#if DEBUG == 1
	printf("d (km) = %f\n", ctx.dtot);
	printf("dlt (km) = %f\n", ctx.dlt);
	printf("dlr (km) = %f\n", ctx.dlr);
	printf("th_t (mrad) = %f\n", ctx.theta_t);
	printf("th_r (mrad) = %f\n", ctx.theta_r);
	printf("th (mrad) = %f\n", ctx.theta);
	printf("hts (m) = %f\n", ctx.hts);
	printf("hrs (m) = %f\n", ctx.hrs);
	printf("htc (m) = %f\n", ctx.htc);
	printf("hrc (m) = %f\n", ctx.hrc);
	printf("w = %f\n", ctx.omega);
	printf("dtm (km) = %f\n", ctx.dtm);
	printf("dlm (km) = %f\n", ctx.dlm);
	printf("phi (deg) = %f\n", parameters->lat);
	printf("b0 (%%) = %f\n", ctx.b0);
	printf("ae (km) = %f\n", ctx.ae);
	printf("hst (m) = %f\n", ctx.hst);
	printf("hsr (m) = %f\n", ctx.hsr);
	printf("hst (m) = %f\n", ctx.hst);
	printf("hsr (m) = %f\n", ctx.hsr);
	printf("hstd (m) = %f\n", ctx.hstd);
	printf("hsrd (m) = %f\n", ctx.hsrd);
	printf("htc' (m) = %f\n", ctx.htc - ctx.hstd);
	printf("hrc' (m) = %f\n", ctx.hrc - ctx.hsrd);
	printf("hte (m) = %f\n", ctx.hte);
	printf("hre (m) = %f\n", ctx.hre);
	printf("hm (m) = %f\n", ctx.hm);
	printf("\n");
#endif

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
	double Lbd50 = pl_los_output.Lbfs + dl_p_output.Ld50[parameters->pol];

	// The basic tranmission loss associated with diffraction not exceeded for
	// p% time Eq (43)
	double Lbd = pl_los_output.Lb0p + dl_p_output.Ldp[parameters->pol];

	// A notional minimum basic transmission loss associated with LoS
	// propagation and over-sea sub-path diffraction
	double Lminb0p = pl_los_output.Lb0p + (1 - ctx.omega) * dl_p_output.Ldp[parameters->pol];

	// eq (40a)
	double Fi = 1;
	if (ctx.p >= ctx.b0)
	{
		Fi = inv_cum_norm(ctx.p / 100) / inv_cum_norm(ctx.b0 / 100);
		Lminb0p = Lbd50 + (pl_los_output.Lb0b + (1 - ctx.omega) * dl_p_output.Ldp[0] - Lbd50) * Fi; // eq (59)
	}

	// Calculate an interpolation factor Fj to take account of the path angular
	// distance Eq (57)
	const double THETA = 0.3;
	const double KSI = 0.8;
	double Fj = 1.0 - 0.5 * (1.0 + c_tanh(3.0 * KSI * (ctx.theta - THETA) / THETA));

	// Calculate an interpolation factor, Fk, to take account of the great
	// circle path distance:
	const double dsw = 20;
	const double kappa = 0.5;
	double Fk = 1.0 - 0.5 * (1.0 + c_tanh(3.0 * kappa * (ctx.dtot - dsw) / dsw)); // eq (58)

	// Calculate the transmission loss due to anomalous propagation
	tl_anomalous_output_t tl_anomalous_output;
	tl_anomalous_input_t tl_anomalous_input;
	copy_ctx_to_tl_anomalous_input(&ctx, &tl_anomalous_input);

	tl_anomalous(&tl_anomalous_input, &tl_anomalous_output);
	double Lba = tl_anomalous_output.Lba;

	const double eta = 2.5;
	double Lminbap = eta * c_log(c_exp(Lba / eta) + c_exp(pl_los_output.Lb0p / eta)); // eq (60)
	double Lbda = Lbd;
	if (Lminbap <= Lbd)
		Lbda = Lminbap + (Lbd - Lminbap) * Fk;

	double Lbam = Lbda + (Lminb0p - Lbda) * Fj; // eq (62)

	tl_tropo_output_t tl_tropo_output;
	tl_tropo_input_t tl_tropo_input;
	copy_ctx_to_tl_tropo_input(&ctx, &tl_tropo_input);
	tl_tropo(&tl_tropo_input, &tl_tropo_output);

	double Lbc = -5 * c_log10(c_pow(10, -0.2 * tl_tropo_output.Lbs) + c_pow(10, -0.2 * Lbam));

	// Location variability of losses (Section 4.8)
	double Lloc = 0.0; // outdoors only (67a)
	if (parameters->zone != RC_ZONE_SEA)
	{
		// TODO parametrize
		double pL = 50;		 // 50% of locations
		double sigmaL = 5.5; // dB
		Lloc = -inv_cum_norm(pL / 100.0) * sigmaL;
	}

	// Basic transmission loss not exceeded for p% time and pL% locations
	// (Sections 4.8 and 4.9) not implemented
	results->Lb = c_max(pl_los_output.Lb0p, Lbc + Lloc); // eq (69)
	results->error = RESULTS_ERR_NONE;

#if DEBUG == 1
	printf("Fi = %g\n", Fi);
	printf("Fj = %f\n", Fj);
	printf("Fk = %f\n", Fk);
	printf("Lbfs = %f\n", pl_los_output.Lbfs);
	printf("Lb0p = %f\n", pl_los_output.Lb0p);
	printf("Lb0b = %f\n", pl_los_output.Lb0b);
	printf("Lbulla (dB) = %f\n", dl_p_output.Lbulla50);
	printf("Lbulls (dB) = %f\n", dl_p_output.Lbulls50);
	printf("Ldsph (dB) = %f\n", dl_p_output.Ldsph50);
	printf("Ld50 (dB) = %f\n", dl_p_output.Ld50[parameters->pol]);
	printf("Ldb (dB) = %f\n", dl_p_output.Ldb[parameters->pol]);
	printf("Ldp (dB) = %f\n", dl_p_output.Ldp[parameters->pol]);
	printf("Lbd50 (dB) = %f\n", Lbd50);
	printf("Lbd (dB) = %f\n", Lbd);
	printf("Lminb0p (dB) = %f\n", Lminb0p);
	printf("Lba (dB) = %f\n", Lba);
	printf("Lminbap (dB) = %f\n", Lminbap);
	printf("Lbda (dB) = %f\n", Lbda);
	printf("Lbam (dB) = %f\n", Lbam);
	printf("Lbs (dB) = %f\n", tl_tropo_output.Lbs);
	printf("Lbc (dB) = %f\n", Lbc);
	printf("Lb (dB) = %f\n", results->Lb);
#endif
}
