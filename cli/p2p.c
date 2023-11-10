#include "p2p.h"
#include "c1812/parameters.h"
#include "c1812/calculate.h"
#include "c1812/pow.h"
#include "c1812/sunit.h"
#include "c1812/rf.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define KM_M 1000.0
#define M_CM 100.0

int p2p_prepare(job_parameters_t *job_parameters, c1812_parameters_t *parameters, terrain_file_t *tfs, clutter_file_t *cfs);

int p2p(job_parameters_t *job_parameters, c1812_parameters_t *parameters, terrain_file_t *tfs, clutter_file_t *cfs)
{
    // Point-to-point calculation
    if (p2p_prepare(job_parameters, parameters, tfs, cfs) != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2p: prepare_point_to_point()\n");
        return EXIT_FAILURE;
    }

    // Calculate loss
    c1812_results_t results;
    c1812_calculate(parameters, &results);

    if (results.error == RESULTS_ERR_NONE)
    {
        // Translate loss to received signal strength
        double Prx = link_budget(
            job_parameters->txpwr,
            job_parameters->txgain,
            job_parameters->rxgain,
            results.Lb);

        // Translate received signal strength to S-units
        s_unit_t S;
        dBm_to_s_unit_hf(Prx, &S);

        // Print results
        printf("Loss = %.1f dB\n", results.Lb);
        char dB_over_sign = (S.dB_over >= 0.0) ? '+' : '-';
        printf("Received power = %.1f dBm (S%d %c %.1fdB)\n", Prx, S.full_units, dB_over_sign, S.dB_over);
    }
    else
    {
        fprintf(stderr, "p2p: calculation error %d\n", results.error);
        return EXIT_FAILURE;
    }

    free(parameters->d);
    free(parameters->h);
    free(parameters->Ct);

    return EXIT_SUCCESS;
}

int p2p_prepare(job_parameters_t *job_parameters, c1812_parameters_t *parameters, terrain_file_t *tfs, clutter_file_t *cfs)
{
    double x1 = job_parameters->txx;
    double y1 = job_parameters->txy;
    double x2 = job_parameters->rxx;
    double y2 = job_parameters->rxy;
    double distance = sqrt(pow2((x2 - x1) / KM_M, 2) + pow2((y2 - y1) / KM_M, 2)); // [km]

    int n = (int)ceil(distance / job_parameters->xres);

    double *d = malloc(n * sizeof(double));
    if (d == NULL)
    {
        fprintf(stderr, "prepare_point_to_point: malloc() d\n");
        return EXIT_FAILURE;
    }

    double *h = malloc(n * sizeof(double));
    if (h == NULL)
    {
        fprintf(stderr, "prepare_point_to_point: malloc() h\n");
        free(d);
        return EXIT_FAILURE;
    }

    double *Ct = malloc(n * sizeof(double));
    if (Ct == NULL)
    {
        fprintf(stderr, "prepare_point_to_point: malloc() Ct\n");
        free(d);
        free(h);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++)
    {
        double xi = x1 + (x2 - x1) * i / (n - 1);
        double yi = y1 + (y2 - y1) * i / (n - 1);
        d[i] = distance * i / (n - 1);
        h[i] = tf_get_bilinear(&tfs[0], xi, yi);
        Ct[i] = (double)cf_get_bilinear(&cfs[0], xi, yi) / M_CM;
    }

    parameters->n = n;
    parameters->d = d;
    parameters->h = h;
    parameters->Ct = Ct;

    return EXIT_SUCCESS;
}
