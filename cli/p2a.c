#include "p2a.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "outfile.h"
#include "c1812/calculation.h"

#define KM 1000.0

typedef struct
{
    int thread_id;

    job_parameters_t *job_parameters;
    c1812_parameters_t *parameters;
    datafile_t *datafiles;

    double *angles;
    int angle_count;
    int angle_start;
    int angle_increment;

    double **results;
} p2a_thread_argument_t;

void *p2a_thread_func(void *argument);

int p2a(job_parameters_t *job_parameters, c1812_parameters_t *parameters, datafile_t *datafiles)
{
    int n = (int)ceil(job_parameters->radius / (job_parameters->xres * KM));
    parameters->n = n;
    parameters->d = malloc(n * sizeof(double));
    if (parameters->d == NULL)
    {
        fprintf(stderr, "p2a: malloc() parameters->d\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++)
        parameters->d[i] = job_parameters->radius * i / (KM * (n - 1));

    int angles_count = (int)ceil(360.0 / job_parameters->ares);
    double *angles = malloc(angles_count * sizeof(double));
    if (angles == NULL)
    {
        fprintf(stderr, "p2a: malloc() angles\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < angles_count; i++)
    {
        // angles[i] = 360.0 * i / angles_count;
        angles[i] = job_parameters->ares * i;
    }

    double **results = malloc(angles_count * sizeof(double *));
    if (results == NULL)
    {
        fprintf(stderr, "p2a: malloc() results\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < angles_count; i++)
    {
        results[i] = malloc(n * sizeof(double));
        if (results[i] == NULL)
        {
            fprintf(stderr, "p2a: malloc() results[%d]\n", i);
            return EXIT_FAILURE;
        }
    }

    pthread_t *threads = malloc(job_parameters->threads * sizeof(pthread_t));
    if (threads == NULL)
    {
        fprintf(stderr, "p2a: malloc() threads\n");
        return EXIT_FAILURE;
    }

    p2a_thread_argument_t *thread_arguments = malloc(job_parameters->threads * sizeof(p2a_thread_argument_t));
    if (thread_arguments == NULL)
    {
        fprintf(stderr, "p2a: malloc() thread_arguments\n");
        return EXIT_FAILURE;
    }

    for (int t = 0; t < job_parameters->threads; t++)
    {
        thread_arguments[t].thread_id = t;

        thread_arguments[t].job_parameters = job_parameters;
        thread_arguments[t].parameters = parameters;
        thread_arguments[t].datafiles = datafiles;

        thread_arguments[t].angles = angles;
        thread_arguments[t].angle_count = angles_count;
        thread_arguments[t].angle_start = t;
        thread_arguments[t].angle_increment = job_parameters->threads;

        thread_arguments[t].results = results;
    }

    for (int t = 0; t < job_parameters->threads; t++)
    {
        if (pthread_create(&threads[t], NULL, p2a_thread_func, &thread_arguments[t]) != 0)
        {
            fprintf(stderr, "p2a: pthread_create() t=%d\n", t);
            return EXIT_FAILURE;
        }
    }

    for (int t = 0; t < job_parameters->threads; t++)
    {
        if (pthread_join(threads[t], NULL) != 0)
        {
            fprintf(stderr, "p2a: pthread_join() t=%d\n", t);
            return EXIT_FAILURE;
        }
    }

    free(threads);
    free(thread_arguments);

    outfile_t outfile;
    if (outfile_open(&outfile, job_parameters->out) != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a: outfile_open()\n");
        return EXIT_FAILURE;
    }

    int ret = outfile_write_header(&outfile, job_parameters->txx, job_parameters->txy, job_parameters->radius, job_parameters->ares, n);
    if (ret != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a: outfile_write_header()\n");
        return EXIT_FAILURE;
    }

    for (int ai = 0; ai < angles_count; ai++)
    {
        if (outfile_write_ray(&outfile, results[ai]) != EXIT_SUCCESS)
        {
            fprintf(stderr, "p2a: outfile_write_ray() angle=%.1f\n", angles[ai]);
            return EXIT_FAILURE;
        }

        free(results[ai]);
    }

    free(results);

    if (outfile_close(&outfile) != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a: outfile_close()\n");
        return EXIT_FAILURE;
    }

    free(angles);
    free(parameters->d);
    
    return EXIT_SUCCESS;
}


void *p2a_thread_func(void *argument)
{
    p2a_thread_argument_t *thread_argument = (p2a_thread_argument_t *)argument;
    job_parameters_t *job_parameters = thread_argument->job_parameters;
    datafile_t *datafiles = thread_argument->datafiles;

    c1812_parameters_t *master_parameters = thread_argument->parameters;
    c1812_parameters_t parameters;
    memcpy(&parameters, master_parameters, sizeof(c1812_parameters_t));
    int n = parameters.n;

    parameters.h = malloc(parameters.n * sizeof(double));
    if (parameters.h == NULL)
    {
        fprintf(stderr, "p2a_thread_func t=%d: malloc() parameters.h\n", thread_argument->thread_id);
        return (void *)EXIT_FAILURE;
    }

    double x1 = job_parameters->txx, y1 = job_parameters->txy;
    double x2, y2;
    double xi, yi;
    double t;

    c1812_results_t results;

    for (int ai = thread_argument->angle_start; ai < thread_argument->angle_count; ai += thread_argument->angle_increment)
    {
        double angle = thread_argument->angles[ai];

        x2 = job_parameters->txx + job_parameters->radius * cos(angle * M_PI / 180.0);
        y2 = job_parameters->txy + job_parameters->radius * sin(angle * M_PI / 180.0);

        for (int i = 0; i < n; i++)
        {
            t = i / (n - 1.0);
            xi = x1 + (x2 - x1) * t;
            yi = y1 + (y2 - y1) * t;
            parameters.h[i] = datafile_get_bilinear(&datafiles[0], xi, yi);
        }

        for (int i = 0; i < 3; i++)
            thread_argument->results[ai][i] = 0.0;

        for (int i = 3; i < n; i++)
        {
            parameters.n = i;
            c1812_calculate(&parameters, &results);
            if (results.error == RESULTS_ERR_NONE)
                thread_argument->results[ai][i] = results.Lb;
            else
            {
                fprintf(stderr, "p2a_thread_func t=%d: calculation error %d\n", thread_argument->thread_id, results.error);
                thread_argument->results[ai][i] = NAN;
            }
        }
    }

    free(parameters.h);
}
