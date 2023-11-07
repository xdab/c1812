#include "c1812/parameters.h"
#include "c1812/calculation.h"
#include "c1812/rf.h"
#include "c1812/sunit.h"
#include "jobfile.h"
#include "datafile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MIN_ARGS 2
#define DEFAULT_STREET_WIDTH 27.0
#define DATAFILE_PATH_MAX 256
#define DATAFILE_EXT ".df"
#define DATAFILE_EXT_LEN 3

int open_datafiles(datafile_t *datafiles, char filenames[MAX_DATA_FILES][MAX_VALUE_LENGTH], int *datafile_count);
int prepare_point_to_point(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles);

int main(const int argc, const char *argv[])
{
    if (argc < MIN_ARGS)
    {
        fprintf(stderr, "Usage: %s <job_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Calculation parameters + some default values
    c1812_parameters_t parameters;
    parameters.ws = DEFAULT_STREET_WIDTH;
    parameters.Ct = NULL;

    // Job parameters to be read from specified file
    job_parameters_t job_parameters;
    if (jobfile_read(&job_parameters, &parameters, argv[1]) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: jobfile_read()\n");
        return EXIT_FAILURE;
    }

    // Open datafiles specified in the job file
    int datafile_count;
    datafile_t datafiles[MAX_DATA_FILES];
    if (open_datafiles(datafiles, job_parameters.data, &datafile_count) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: open_datafiles()\n");
        return EXIT_FAILURE;
    }

    // Prepare vector parameters for point-to-point calculation
    if (prepare_point_to_point(&job_parameters, &parameters, datafiles) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: prepare_point_to_point()\n");
        return EXIT_FAILURE;
    }

    // Free datafiles as they are no longer needed
    for (int i = 0; i < datafile_count; i++)
        datafile_free(&datafiles[i]);

    // Calculate loss
    c1812_results_t results;
    c1812_calculate(&parameters, &results);

    if (results.error == RESULTS_ERR_NONE)
    {
        // Translate loss to received signal strength
        double Prx = link_budget(job_parameters.txpwr, job_parameters.txgain, job_parameters.rxgain, results.Lb);

        // Translate received signal strength to S-units
        s_unit_t S;
        dBm_to_s_unit_hf(Prx, &S);

        // Print results
        printf("\nLoss = %.1f dB\n", results.Lb);
        if (S.dB_over >= 0.0)
            printf("Received power = %.1f dBm (S%d + %.1fdB)\n", Prx, S.full_units, S.dB_over);
        else
            printf("Received power = %.1f dBm (S%d - %.1fdB)\n", Prx, S.full_units, -S.dB_over);
    }
    else
    {
        fprintf(stderr, "c1812_calculate: %d\n", results.error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int prepare_point_to_point(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles)
{
    double x1 = job_parameters->txx;
    double y1 = job_parameters->txy;
    double x2 = job_parameters->rxx;
    double y2 = job_parameters->rxy;
    const double KM = 1000.0;
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