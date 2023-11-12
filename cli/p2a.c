#include "p2a.h"

#include <pthread.h>
#include <stdbool.h>
#include "outfile.h"
#include "image.h"
#include "colors.h"

typedef struct
{
    int thread_id;

    job_parameters_t *job_parameters;
    c1812_parameters_t *parameters;
    terrain_file_t *tfs;
    clutter_file_t *cfs;

    double *angles;
    int angle_count;
    int angle_start;
    int angle_increment;

    double **results;
} p2a_thread_argument_t;

void *p2a_thread_func(void *argument);
int malloc_caches(c1812_parameters_t *parameters, int n);
void clear_caches(c1812_parameters_t *parameters, int n);
void free_caches(c1812_parameters_t *parameters);
int output_image(job_parameters_t *job, double **results, double *angles, int angles_count, int n);
int output_rf_file(job_parameters_t *job, double **results, double *angles, int angles_count, int n);

int p2a(job_parameters_t *job, c1812_parameters_t *parameters, terrain_file_t *tfs, clutter_file_t *cfs)
{
    int n = (int)c_ceil(job->radius / (job->xres * KM_M));
    parameters->n = n;
    parameters->d = malloc(n * sizeof(double));
    if (parameters->d == NULL)
    {
        fprintf(stderr, "p2a: malloc() parameters->d\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++)
        parameters->d[i] = job->radius * i / (KM_M * (n - 1));

    int angles_count = (int)c_ceil(360.0 / job->ares);
    double *angles = malloc(angles_count * sizeof(double));
    if (angles == NULL)
    {
        fprintf(stderr, "p2a: malloc() angles\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < angles_count; i++)
        angles[i] = 360.0 * i / angles_count;

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

    pthread_t *threads = malloc(job->threads * sizeof(pthread_t));
    if (threads == NULL)
    {
        fprintf(stderr, "p2a: malloc() threads\n");
        return EXIT_FAILURE;
    }

    p2a_thread_argument_t *thread_arguments = malloc(job->threads * sizeof(p2a_thread_argument_t));
    if (thread_arguments == NULL)
    {
        fprintf(stderr, "p2a: malloc() thread_arguments\n");
        return EXIT_FAILURE;
    }

    for (int t = 0; t < job->threads; t++)
    {
        thread_arguments[t].thread_id = t;

        thread_arguments[t].job_parameters = job;
        thread_arguments[t].parameters = parameters;
        thread_arguments[t].tfs = tfs;
        thread_arguments[t].cfs = cfs;

        thread_arguments[t].angles = angles;
        thread_arguments[t].angle_count = angles_count;
        thread_arguments[t].angle_start = t;
        thread_arguments[t].angle_increment = job->threads;

        thread_arguments[t].results = results;
    }

    for (int t = 0; t < job->threads; t++)
    {
        if (pthread_create(&threads[t], NULL, p2a_thread_func, &thread_arguments[t]) != 0)
        {
            fprintf(stderr, "p2a: pthread_create() t=%d\n", t);
            return EXIT_FAILURE;
        }
    }

    for (int t = 0; t < job->threads; t++)
    {
        if (pthread_join(threads[t], NULL) != 0)
        {
            fprintf(stderr, "p2a: pthread_join() t=%d\n", t);
            return EXIT_FAILURE;
        }
    }

    free(threads);
    free(thread_arguments);

    bool generate_img = (strlen(job->img) > 0);
    if (generate_img)
    {
        if (output_image(job, results, angles, angles_count, n) != EXIT_SUCCESS)
        {
            fprintf(stderr, "p2a: output_image()\n");
            return EXIT_FAILURE;
        }
    }

    bool generate_out = (strlen(job->out) > 0);
    if (generate_out)
    {
        if (output_rf_file(job, results, angles, angles_count, n) != EXIT_SUCCESS)
        {
            fprintf(stderr, "p2a: output_rf_file()\n");
            return EXIT_FAILURE;
        }
    }

    for (int ai = 0; ai < angles_count; ai++)
        free(results[ai]);
    free(results);
    free(angles);
    free(parameters->d);

    return EXIT_SUCCESS;
}

int output_image(job_parameters_t *job, double **results, double *angles, int angles_count, int n)
{
    int W = job->img_size;
    int H = job->img_size;

    image_t image;
    if (image_create(&image, W, H) != EXIT_SUCCESS)
    {
        fprintf(stderr, "output_image: image_create()\n");
        return EXIT_FAILURE;
    }

    for (int im_y = 0; im_y < H; im_y++)
    {
        double y = job->txy + job->radius * (im_y - H / 2) / (H / 2);

        for (int im_x = 0; im_x < W; im_x++)
        {
            double x = job->txx + job->radius * (im_x - W / 2) / (W / 2);

            double distance = c_sqrt(c_pow(x - job->txx, 2) + c_pow(y - job->txy, 2));
            if (distance > job->radius)
                continue;

            double angle = c_atan2_exact(y - job->txy, x - job->txx) * 180.0 / PI;
            if (angle < 0.0)
                angle += 360.0;

            int ai = (int)c_round(angle / job->ares);
            if (ai >= angles_count)
                ai = 0;

            int ni = (int)c_floor(distance / (job->xres * KM_M));
            if (ni >= n)
                ni = n - 1;
            if (ni < 3)
                ni = 3;

            double loss = results[ai][ni];
            double value = NAN;

            switch (job->img_data_type)
            {
            case IMG_DATA_TYPE_S_UNITS:
            {
                double rx_pwr_dbm = link_budget(job->txpwr, job->txgain, job->rxgain, loss);
                s_unit_t S;
                dBm_to_s_unit_hf(rx_pwr_dbm, &S);
                value = S.full_units + S.dB_over / 6.0;
            }
            break;
            case IMG_DATA_TYPE_LOSS:
            case IMG_DATA_TYPE_TERRAIN:
            case IMG_DATA_TYPE_CLUTTER:
                value = loss;
                break;
            default:
                fprintf(stderr, "output_image: unknown image data type\n");
                return EXIT_FAILURE;
            }

            value = (value - job->img_scale_min) / (job->img_scale_max - job->img_scale_min);

            int rgb = cmap_get(job->img_colormap, value);
            unsigned char r, g, b;
            unpack_rgb(rgb, &r, &g, &b);

            if (image_set(&image, im_x, im_y, r, g, b) != EXIT_SUCCESS)
            {
                fprintf(stderr, "output_image: image_set()\n");
                return EXIT_FAILURE;
            }
        }
    }

    if (image_write(&image, job->img) != EXIT_SUCCESS)
    {
        fprintf(stderr, "output_image: image_write()\n");
        return EXIT_FAILURE;
    }

    image_free(&image);
    return EXIT_SUCCESS;
}

int output_rf_file(job_parameters_t *job, double **results, double *angles, int angles_count, int n)
{
    outfile_t outfile;
    if (outfile_open(&outfile, job->out) != EXIT_SUCCESS)
    {
        fprintf(stderr, "output_rf_file: outfile_open()\n");
        return EXIT_FAILURE;
    }

    if (outfile_write_header(&outfile, job->txx, job->txy, job->radius, job->ares, n) != EXIT_SUCCESS)
    {
        fprintf(stderr, "output_rf_file: outfile_write_header()\n");
        return EXIT_FAILURE;
    }

    for (int ai = 0; ai < angles_count; ai++)
    {
        if (outfile_write_ray(&outfile, results[ai]) != EXIT_SUCCESS)
        {
            fprintf(stderr, "output_rf_file: outfile_write_ray() angle=%.1f\n", angles[ai]);
            return EXIT_FAILURE;
        }
    }

    if (outfile_close(&outfile) != EXIT_SUCCESS)
    {
        fprintf(stderr, "output_rf_file: outfile_close()\n");
        return EXIT_FAILURE;
    }
}

void *p2a_thread_func(void *argument)
{
    p2a_thread_argument_t *thread_argument = (p2a_thread_argument_t *)argument;
    job_parameters_t *job = thread_argument->job_parameters;
    terrain_file_t *tfs = thread_argument->tfs;
    clutter_file_t *cfs = thread_argument->cfs;

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

    parameters.Ct = malloc(parameters.n * sizeof(double));
    if (parameters.Ct == NULL)
    {
        fprintf(stderr, "p2a_thread_func t=%d: malloc() parameters.Ct\n", thread_argument->thread_id);
        return (void *)EXIT_FAILURE;
    }

    if (malloc_caches(&parameters, n + 3) != EXIT_SUCCESS)
    {
        fprintf(stderr, "p2a_thread_func t=%d: malloc_caches()\n", thread_argument->thread_id);
        return (void *)EXIT_FAILURE;
    }

    double x1 = job->txx, y1 = job->txy;
    double angle;
    double x2, y2;
    double xi, yi;
    double t;

    c1812_results_t results;

    for (int ai = thread_argument->angle_start; ai < thread_argument->angle_count; ai += thread_argument->angle_increment)
    {
        angle = thread_argument->angles[ai];

        x2 = job->txx + job->radius * c_cos(angle * PI / 180.0);
        y2 = job->txy + job->radius * c_sin(angle * PI / 180.0);

        for (int i = 0; i < n; i++)
        {
            t = i / (n - 1.0);
            xi = x1 + (x2 - x1) * t;
            yi = y1 + (y2 - y1) * t;
            parameters.h[i] = tf_interpolation_func(&tfs[0], xi, yi);
            parameters.Ct[i] = cf_interpolation_func(&cfs[0], xi, yi) / M_DM;
        }

        for (int i = 0; i < 3; i++)
            thread_argument->results[ai][i] = 0.0;

        clear_caches(&parameters, n + 3);

        for (int i = n - 1; i >= 3; i--)
        {
            if (results.error == RESULTS_ERR_NONE)
            {
                if (job->img_data_type == IMG_DATA_TYPE_TERRAIN)
                    thread_argument->results[ai][i] = parameters.h[i];
                else if (job->img_data_type == IMG_DATA_TYPE_CLUTTER)
                    thread_argument->results[ai][i] = parameters.Ct[i];
                else
                {
                    parameters.n = i;
                    c1812_calculate(&parameters, &results);
                    thread_argument->results[ai][i] = results.Lb;
                }
            }
            else
            {
                fprintf(stderr, "p2a_thread_func t=%d: calculation error %d\n", thread_argument->thread_id, results.error);
                thread_argument->results[ai][i] = NAN;
            }
        }
    }

    free(parameters.h);
    free_caches(&parameters);
}

int malloc_caches(c1812_parameters_t *parameters, int n)
{
    parameters->v1_cache = malloc(n * sizeof(double));
    if (parameters->v1_cache == NULL)
    {
        fprintf(stderr, "malloc_caches: malloc() parameters->v1_cache\n");
        return EXIT_FAILURE;
    }

    parameters->v2_cache = malloc(n * sizeof(double));
    if (parameters->v2_cache == NULL)
    {
        free(parameters->v1_cache);
        fprintf(stderr, "malloc_caches: malloc() parameters->v2_cache\n");
        return EXIT_FAILURE;
    }

    parameters->theta_max_cache = malloc(n * sizeof(double));
    if (parameters->theta_max_cache == NULL)
    {
        free(parameters->v1_cache);
        free(parameters->v2_cache);
        fprintf(stderr, "malloc_caches: malloc() parameters->theta_max_cache\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void clear_caches(c1812_parameters_t *parameters, int n)
{
    for (int i = 0; i < n; i++)
    {
        parameters->v1_cache[i] = NAN;
        parameters->v2_cache[i] = NAN;
        parameters->theta_max_cache[i] = NAN;
    }
}

void free_caches(c1812_parameters_t *parameters)
{
    free(parameters->v1_cache);
    free(parameters->v2_cache);
    free(parameters->theta_max_cache);
}