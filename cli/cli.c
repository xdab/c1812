#include "c1812/parameters.h"
#include "c1812/calculation.h"
#include "c1812/rf.h"
#include "c1812/sunit.h"

#include "jobfile.h"
#include "datafile.h"
#include "outfile.h"
#include "p2a.h"
#include "p2p.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define MIN_ARGS 2
#define DEFAULT_STREET_WIDTH 27.0
#define DATAFILE_PATH_MAX 256
#define DATAFILE_EXT ".df"
#define DATAFILE_EXT_LEN 3
#define WRITE_BINARY "wb"

int validate_job_parameters(job_parameters_t *job_parameters);
int open_datafiles(datafile_t *datafiles, char filenames[MAX_DATA_FILES][MAX_VALUE_LENGTH], int *datafile_count);

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