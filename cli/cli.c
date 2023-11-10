#include "c1812/parameters.h"
#include "c1812/calculation.h"
#include "c1812/rf.h"
#include "c1812/sunit.h"

#include "jobfile.h"
#include "terrain_file.h"
#include "clutter_file.h"
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
#define PARSED_TF_EXT ".tf"
#define PARSED_TF_EXT_LEN 3
#define WRITE_BINARY "wb"

int validate_job_parameters(job_parameters_t *job_parameters);
int open_terrain_files(terrain_file_t *tfs, char paths[MAX_TERRAIN_FILES][MAX_VALUE_LENGTH], int *tf_count);
int open_clutter_files(clutter_file_t *tfs, char paths[MAX_CLUTTER_FILES][MAX_VALUE_LENGTH], int *tf_count);

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

    int terrain_file_count;
    terrain_file_t terrain_files[MAX_TERRAIN_FILES];
    if (open_terrain_files(terrain_files, job_parameters.terrain, &terrain_file_count) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: open_terrain_files()\n");
        return EXIT_FAILURE;
    }

    int clutter_file_count;
    clutter_file_t clutter_files[MAX_CLUTTER_FILES];
    if (open_clutter_files(clutter_files, job_parameters.clutter, &clutter_file_count) != EXIT_SUCCESS)
    {
        fprintf(stderr, "main: open_clutter_files()\n");
        return EXIT_FAILURE;
    }

    if (terrain_file_count == 0)
    {
        fprintf(stderr, "main: no terrain data files specified\n");
        return EXIT_FAILURE;
    }

    if (!isnan(job_parameters.rxx) && !isnan(job_parameters.rxy))
    {
        // Point-to-point calculation
        if (p2p(&job_parameters, &parameters, terrain_files, clutter_files) != EXIT_SUCCESS)
        {
            fprintf(stderr, "main: p2p()\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        // Point-to-area calculation
        if (p2a(&job_parameters, &parameters, terrain_files, clutter_files) != EXIT_SUCCESS)
        {
            fprintf(stderr, "main: p2a()\n");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < terrain_file_count; i++)
        tf_free(&terrain_files[i]);

    for (int i = 0; i < clutter_file_count; i++)
        cf_free(&clutter_files[i]);

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

int open_terrain_files(terrain_file_t *tfs, char paths[MAX_TERRAIN_FILES][MAX_VALUE_LENGTH], int *tf_count)
{
    for (int i = 0; i < MAX_TERRAIN_FILES; i++)
    {
        // Should not happen
        if (paths[i] == NULL)
        {
            fprintf(stderr, "open_terrain_files: filenames[%d] == NULL\n", i);
            return EXIT_FAILURE;
        }

        int len = strlen(paths[i]);
        if (len == 0) // First empty filename marks the end of the list
        {
            *tf_count = i;
            return EXIT_SUCCESS;
        }

        // If data[i] ends in .df, then it's an already processed file
        // that can be opened directly
        if (strcmp(paths[i] + len - PARSED_TF_EXT_LEN, PARSED_TF_EXT) == 0)
        {
            if (tf_open(&tfs[i], paths[i]) != EXIT_SUCCESS)
            {
                fprintf(stderr, "open_terrain_files: tf_open()\n");
                return EXIT_FAILURE;
            }
        }
        else // Otherwise, it's a text file that needs to be parsed
        {
            if (tf_parse(&tfs[i], paths[i]) != EXIT_SUCCESS)
            {
                fprintf(stderr, "open_terrain_files: tf_parse()\n");
                return EXIT_FAILURE;
            }

            // The parsed file is stored for future use
            char *parsed_tf_path = malloc(len + 1 + PARSED_TF_EXT_LEN + 1);
            strcpy(parsed_tf_path, paths[i]);
            strcat(parsed_tf_path, PARSED_TF_EXT);

            if (tf_store(&tfs[i], parsed_tf_path) != EXIT_SUCCESS)
            {
                fprintf(stderr, "open_terrain_files: tf_store()\n");
                return EXIT_FAILURE;
            }

            free(parsed_tf_path);
        }
    }

    *tf_count = MAX_TERRAIN_FILES;
    return EXIT_SUCCESS;
}

int open_clutter_files(clutter_file_t *cfs, char paths[MAX_CLUTTER_FILES][MAX_VALUE_LENGTH], int *cf_count)
{
    for (int i = 0; i < MAX_CLUTTER_FILES; i++)
    {
        // Should not happen
        if (paths[i] == NULL)
        {
            fprintf(stderr, "open_clutter_files: filenames[%d] == NULL\n", i);
            return EXIT_FAILURE;
        }

        int len = strlen(paths[i]);
        if (len == 0) // First empty filename marks the end of the list
        {
            *cf_count = i;
            return EXIT_SUCCESS;
        }

        if (cf_open(&cfs[i], paths[i]) != EXIT_SUCCESS)
        {
            fprintf(stderr, "open_clutter_files: cf_open()\n");
            return EXIT_FAILURE;
        }
    }

    *cf_count = MAX_CLUTTER_FILES;
    return EXIT_SUCCESS;
}