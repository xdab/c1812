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

#define DATAFILE_PATH_MAX 256
#define DATAFILE_EXT ".df"
#define DATAFILE_EXT_LEN 3

int open_datafiles(datafile_t *datafiles, char filenames[MAX_DATA_FILES][MAX_VALUE_LENGTH]);
void prepare_point_to_point(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles);

int main(const int argc, const char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <job file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    job_parameters_t job_parameters;
    c1812_parameters_t parameters;
    parameters.ws = 27;
    parameters.Ct = NULL;
    jobfile_read(&job_parameters, &parameters, argv[1]);
    printf("Read job file %s\n", argv[1]);

    datafile_t datafiles[MAX_DATA_FILES];
    int datafile_count = open_datafiles(datafiles, job_parameters.data);
    printf("Opened %d datafiles\n", datafile_count);

    prepare_point_to_point(&job_parameters, &parameters, datafiles);

    for (int i = 0; i < datafile_count; i++)
        datafile_free(&datafiles[i]);

    c1812_results_t results;
    c1812_calculate(&parameters, &results);

    if (results.error == RESULTS_ERR_NONE)
    {
        double Prx = link_budget(job_parameters.txpwr, job_parameters.txgain, job_parameters.rxgain, results.Lb);
        s_unit_t S;
        dBm_to_s_unit_hf(Prx, &S);

        printf("\nLoss = %.1f dB\n", results.Lb);
        if (S.dB_over >= 0.0)
            printf("Received power = %.1f dBm (S%d + %.1fdB)\n", Prx, S.full_units, S.dB_over);
        else
            printf("Received power = %.1f dBm (S%d - %.1fdB)\n", Prx, S.full_units, -S.dB_over);
    }
    else
    {
        printf("Error: %d\n", results.error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void prepare_point_to_point(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles)
{
    double x1 = job_parameters->txx; // [m]
    double y1 = job_parameters->txy; // [m]
    double x2 = job_parameters->rxx; // [m]
    double y2 = job_parameters->rxy; // [m]
    const double KM = 1000.0;
    double distance = sqrt(pow((x2 - x1) / KM, 2) + pow((y2 - y1) / KM, 2)); // [km]

    int n = (int)ceil(distance / job_parameters->xres);
    double *d = malloc(n * sizeof(double));
    double *h = malloc(n * sizeof(double));
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
}

int open_datafiles(datafile_t *datafiles, char filenames[MAX_DATA_FILES][MAX_VALUE_LENGTH])
{
    for (int i = 0; i < MAX_DATA_FILES; i++)
    {
        if (filenames[i] == NULL)
            return i;
        
        int len = strlen(filenames[i]);
        if (len == 0)
            return i;

        // If data[i] ends in .df, then it's an already processed datafile
        // that can be opened directly
        if (strcmp(filenames[i] + len - DATAFILE_EXT_LEN, DATAFILE_EXT) == 0)
            datafile_open(&datafiles[i], filenames[i]);
        else
        {
            // Otherwise, it's a text datafile that needs to be parsed
            datafile_parse(&datafiles[i], filenames[i]);

            // The parsed datafile is stored for future use
            char *datafile_path = malloc(len + 1 + DATAFILE_EXT_LEN + 1);
            strcpy(datafile_path, filenames[i]);
            strcat(datafile_path, DATAFILE_EXT);
            datafile_store(&datafiles[i], datafile_path);
            free(datafile_path);
        }
    }

    return MAX_DATA_FILES;
}