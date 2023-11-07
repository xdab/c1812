#include "c1812/parameters.h"
#include "c1812/calculation.h"
#include "c1812/rf.h"
#include "c1812/sunit.h"
#include "jobfile.h"
#include "datafile.h"
#include "outfile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MIN_ARGS 2
#define DEFAULT_STREET_WIDTH 27.0
#define DATAFILE_PATH_MAX 256
#define DATAFILE_EXT ".df"
#define DATAFILE_EXT_LEN 3
#define KM 1000.0
#define WRITE_BINARY "wb"

int validate_job_parameters(job_parameters_t *job_parameters);
int open_datafiles(datafile_t *datafiles, char filenames[MAX_DATA_FILES][MAX_VALUE_LENGTH], int *datafile_count);

int p2p(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles);
int p2p_prepare(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles);

int p2a(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles);

int main(const int argc, const char *argv[])
{
    if (argc < MIN_ARGS)
    {
        fprintf(stderr, "Usage: %s <job_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Calculation parameters and defaults
    c1812_parameters_t parameters;
    parameters.ws = DEFAULT_STREET_WIDTH;
    parameters.Ct = NULL;

    job_parameters_t job_parameters;
    if (jobfile_read(&job_parameters, &parameters, argv[1]) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: jobfile_read()\n");
        return EXIT_FAILURE;
    }

    if (validate_job_parameters(&job_parameters) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: validate_job_parameters()\n");
        return EXIT_FAILURE;
    }

    int datafile_count;
    datafile_t datafiles[MAX_DATA_FILES];
    if (open_datafiles(datafiles, job_parameters.data, &datafile_count) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: open_datafiles()\n");
        return EXIT_FAILURE;
    }

    if (datafile_count == 0)
    {
        fprintf(stderr, "main: no datafiles specified\n");
        return EXIT_FAILURE;
    }

    if (!isnan(job_parameters.rxx) && !isnan(job_parameters.rxy))
    {
        // Point-to-point calculation
        if (p2p(&job_parameters, &parameters, datafiles) != EXIT_SUCCESS)
        {
            fprintf(stderr, "main: p2p()\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        // Point-to-area calculation
        if (p2a(&job_parameters, &parameters, datafiles) != EXIT_SUCCESS)
        {
            fprintf(stderr, "main: p2a()\n");
            return EXIT_FAILURE;
        }
    }

    // Free datafiles
    for (int i = 0; i < datafile_count; i++)
        datafile_free(&datafiles[i]);

    return EXIT_SUCCESS;
}

int p2a(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles)
{
    int n = (int)ceil(job_parameters->radius / (job_parameters->xres * KM));

    parameters->d = malloc(n * sizeof(double));
    if (parameters->d == NULL)
    {
        fprintf(stderr, "p2a: malloc() parameters->d\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++)
        parameters->d[i] = job_parameters->radius * i / (KM * (n - 1));

    parameters->h = malloc(n * sizeof(double));
    if (parameters->h == NULL)
    {
        fprintf(stderr, "p2a: malloc() parameters->h\n");
        return EXIT_FAILURE;
    }

    double *Lb = malloc(n * sizeof(double));
    if (Lb == NULL)
    {
        fprintf(stderr, "p2a: malloc() Lb\n");
        return EXIT_FAILURE;
    }

    double x1 = job_parameters->txx, y1 = job_parameters->txy;
    double x2, y2;
    double xi, yi;
    double t;

    c1812_results_t results;

    outfile_t outfile;
    if (outfile_open(&outfile, job_parameters->out) != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a: outfile_open()\n");
        return EXIT_FAILURE;
    }

    int ret = outfile_write_header(
        &outfile,
        job_parameters->txx,
        job_parameters->txy,
        job_parameters->radius,
        job_parameters->ares,
        n);
    if (ret != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a: outfile_write_header()\n");
        return EXIT_FAILURE;
    }

    for (double angle = 0.0; angle < 360.0; angle += job_parameters->ares)
    {
        printf("Angle: %.1f\n", angle);

        x2 = job_parameters->txx + job_parameters->radius * cos(angle * M_PI / 180.0);
        y2 = job_parameters->txy + job_parameters->radius * sin(angle * M_PI / 180.0);

        for (int i = 0; i < n; i++)
        {
            t = i / (n - 1.0);
            xi = x1 + (x2 - x1) * t;
            yi = y1 + (y2 - y1) * t;
            parameters->h[i] = datafile_get_bilinear(&datafiles[0], xi, yi);
        }

        for (int i = 3; i < n; i++)
        {
            parameters->n = i;
            c1812_calculate(parameters, &results);
            if (results.error == RESULTS_ERR_NONE)
                Lb[i] = results.Lb;
            else
            {
                fprintf(stderr, "p2a: calculation error %d\n", results.error);
                Lb[i] = NAN;
            }
        }

        if (outfile_write_ray(&outfile, Lb) != EXIT_SUCCESS)
        {
            fprintf(stderr, "p2a: outfile_write_ray()\n");
            return EXIT_FAILURE;
        }
    }

    if (outfile_close(&outfile) != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a: outfile_close()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int p2p(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles)
{
    // Point-to-point calculation
    if (p2p_prepare(job_parameters, parameters, datafiles) != EXIT_SUCCESS)
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

    return EXIT_SUCCESS;
}

int validate_job_parameters(job_parameters_t *job_parameters)
{
    if (isnan(job_parameters->txx) || isnan(job_parameters->txy))
    {
        fprintf(stderr, "validate_job_parameters: txx and txy are required\n");
        return EXIT_FAILURE;
    }

    if (!isnan(job_parameters->txpwr) && job_parameters->txpwr <= 0.0)
    {
        fprintf(stderr, "validate_job_parameters: txpwr must be positive\n");
        return EXIT_FAILURE;
    }

    if (!isnan(job_parameters->rxx) && !isnan(job_parameters->rxy))
    {
        if (isnan(job_parameters->radius))
        {
            fprintf(stderr, "validate_job_parameters: radius is required for p2a calculation\n");
            return EXIT_FAILURE;
        }
        else if (job_parameters->radius <= 0.0)
        {
            fprintf(stderr, "validate_job_parameters: radius must be positive\n");
            return EXIT_FAILURE;
        }

        if (isnan(job_parameters->ares))
        {
            fprintf(stderr, "validate_job_parameters: ares is required for p2a calculation\n");
            return EXIT_FAILURE;
        }
        else if (job_parameters->ares <= 0.0)
        {
            fprintf(stderr, "validate_job_parameters: ares must be positive\n");
            return EXIT_FAILURE;
        }
    }

    if (isnan(job_parameters->xres))
    {
        fprintf(stderr, "validate_job_parameters: xres is required\n");
        return EXIT_FAILURE;
    }
    else if (job_parameters->xres <= 0.0)
    {
        fprintf(stderr, "validate_job_parameters: xres must be positive\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int p2p_prepare(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles)
{
    double x1 = job_parameters->txx;
    double y1 = job_parameters->txy;
    double x2 = job_parameters->rxx;
    double y2 = job_parameters->rxy;
    double distance = sqrt(pow((x2 - x1) / KM, 2) + pow((y2 - y1) / KM, 2)); // [km]

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
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++)
    {
        double xi = x1 + (x2 - x1) * i / (n - 1);
        double yi = y1 + (y2 - y1) * i / (n - 1);
        d[i] = distance * i / (n - 1);
        h[i] = datafile_get_bilinear(&datafiles[0], xi, yi);
    }

    parameters->n = n;
    parameters->d = d;
    parameters->h = h;

    return EXIT_SUCCESS;
}

int open_datafiles(datafile_t *datafiles, char filenames[MAX_DATA_FILES][MAX_VALUE_LENGTH], int *datafile_count)
{
    for (int i = 0; i < MAX_DATA_FILES; i++)
    {
        // Should not happen
        if (filenames[i] == NULL)
        {
            fprintf(stderr, "open_datafiles: filenames[%d] == NULL\n", i);
            return EXIT_FAILURE;
        }

        int len = strlen(filenames[i]);
        if (len == 0) // First empty filename marks the end of the list
        {
            *datafile_count = i;
            return EXIT_SUCCESS;
        }

        // If data[i] ends in .df, then it's an already processed datafile
        // that can be opened directly
        if (strcmp(filenames[i] + len - DATAFILE_EXT_LEN, DATAFILE_EXT) == 0)
        {
            if (datafile_open(&datafiles[i], filenames[i]) != EXIT_SUCCESS)
            {
                fprintf(stderr, "open_datafiles: datafile_open()\n");
                return EXIT_FAILURE;
            }
        }
        else // Otherwise, it's a text datafile that needs to be parsed
        {
            if (datafile_parse(&datafiles[i], filenames[i]) != EXIT_SUCCESS)
            {
                fprintf(stderr, "open_datafiles: datafile_parse()\n");
                return EXIT_FAILURE;
            }

            // The parsed datafile is stored for future use
            char *datafile_path = malloc(len + 1 + DATAFILE_EXT_LEN + 1);
            strcpy(datafile_path, filenames[i]);
            strcat(datafile_path, DATAFILE_EXT);

            if (datafile_store(&datafiles[i], datafile_path) != EXIT_SUCCESS)
            {
                fprintf(stderr, "open_datafiles: datafile_store()\n");
                return EXIT_FAILURE;
            }

            free(datafile_path);
        }
    }

    *datafile_count = MAX_DATA_FILES;
    return EXIT_SUCCESS;
}