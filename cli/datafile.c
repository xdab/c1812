#include "datafile.h"
#include "vec.h"
#include "nneighbor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define READ "r"
#define READ_BINARY "rb"
#define WRITE_BINARY "wb"
#define MAX_VALUE_LENGTH 20
#define MAX_LINE_LENGTH (3 * MAX_VALUE_LENGTH + 2)
#define SPLIT_CHARS " \n"
#define X_TOKEN_INDEX 0
#define Y_TOKEN_INDEX 1
#define H_TOKEN_INDEX 2

void datafile_zero(datafile_t *datafile)
{
    datafile->x = NULL;
    datafile->x_size = 0;

    datafile->y = NULL;
    datafile->y_size = 0;

    datafile->h = NULL;
}

// qsort-compatible comparator for doubles
int double_comparator(const void *p, const void *q)
{
    double p_d = *(double *)p;
    double q_d = *(double *)q;
    if (p_d == q_d)
        return 0;
    if (p_d > q_d)
        return 1;
    return -1;
}

void datafile_parse(datafile_t *datafile, const char *path)
{
    datafile_zero(datafile);

    FILE *datafile_fp = fopen(path, READ);
    if (datafile_fp == NULL)
        exit(EXIT_FAILURE);

    char line[MAX_LINE_LENGTH + 1];
    int line_index = 0;

    vec_double_t x_vec;
    vec_init(&x_vec);

    vec_double_t y_vec;
    vec_init(&y_vec);

    // Optimization:
    // Since the datafile is likely sorted by x or y, we can avoid
    // calling vec_double_sorted_unique_insert() for every x and y value by keeping track of the last.
    // When the current value is the same as the last, we know it's already in the vector.
    double last_x = NAN;
    double last_y = NAN;

    while (fgets(line, MAX_LINE_LENGTH, datafile_fp) != NULL)
    {
        line_index++;
        int len = strlen(line);
        if (len <= 1)
            continue;

        int token_index = 0;
        char *token = strtok(line, SPLIT_CHARS);

        double x, y;
        while (token != NULL)
        {
            if (token_index == X_TOKEN_INDEX)
            {
                x = atof(token);
                if (x != last_x)
                {
                    vec_double_sorted_unique_insert(&x_vec, x);
                    last_x = x;
                }
            }
            else if (token_index == Y_TOKEN_INDEX)
            {
                y = atof(token);
                if (y != last_y)
                {
                    vec_double_sorted_unique_insert(&y_vec, y);
                    last_y = y;
                }
            }
            else if (token_index == H_TOKEN_INDEX)
            {
                break; // Ignore for this pass
            }
            else
            {
                fprintf(stderr, "Too many tokens on line %d\n", line_index);
                exit(EXIT_FAILURE);
            }

            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }
    }

    datafile->x_size = x_vec.length;
    datafile->x = malloc(datafile->x_size * sizeof(double));
    memcpy(datafile->x, x_vec.data, datafile->x_size * sizeof(double));
    vec_deinit(&x_vec);

    datafile->y_size = y_vec.length;
    datafile->y = malloc(datafile->y_size * sizeof(double));
    memcpy(datafile->y, y_vec.data, datafile->y_size * sizeof(double));
    vec_deinit(&y_vec);

    // Now x and y dimensions are known, thus the h matrix can be allocated
    // and filled in the second pass
    rewind(datafile_fp);

    // Allocate h matrix and fill with nans
    datafile->h = malloc(datafile->y_size * sizeof(double *));
    for (int i = 0; i < datafile->y_size; i++)
    {
        datafile->h[i] = malloc(datafile->x_size * sizeof(double));
        for (int j = 0; j < datafile->x_size; j++)
            datafile->h[i][j] = NAN;
    }

    // Second pass
    line_index = 0;
    // Optimization of nneighbor() calls
    last_x = NAN;
    last_y = NAN;
    int last_x_index = -1;
    int last_y_index = -1;

    while (fgets(line, MAX_LINE_LENGTH, datafile_fp) != NULL)
    {
        line_index++;
        int len = strlen(line);
        if (len <= 1)
            continue;

        int token_index = 0;
        char *token = strtok(line, SPLIT_CHARS);

        double x, y;
        while (token != NULL)
        {
            if (token_index == X_TOKEN_INDEX)
            {
                x = atof(token);
                if (x != last_x)
                {
                    nneighbor(datafile->x, datafile->x_size, x, &last_x_index);
                    last_x = x;
                }
            }
            else if (token_index == Y_TOKEN_INDEX)
            {
                y = atof(token);
                if (y != last_y)
                {
                    nneighbor(datafile->y, datafile->y_size, y, &last_y_index);
                    last_y = y;
                }
            }
            else if (token_index == H_TOKEN_INDEX)
            {
                double h = atof(token);
                datafile->h[last_y_index][last_x_index] = h;
                break;
            }

            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }
    }

    fclose(datafile_fp);
}

void datafile_store(datafile_t *datafile, const char *path)
{
    FILE *datafile_fp = fopen(path, WRITE_BINARY);
    fwrite(&datafile->y_size, sizeof(int), 1, datafile_fp);
    fwrite(&datafile->x_size, sizeof(int), 1, datafile_fp);
    fwrite(datafile->y, sizeof(double), datafile->y_size, datafile_fp);
    fwrite(datafile->x, sizeof(double), datafile->x_size, datafile_fp);
    for (int i = 0; i < datafile->y_size; i++)
        fwrite(datafile->h[i], sizeof(double), datafile->x_size, datafile_fp);
    fclose(datafile_fp);
}

void datafile_open(datafile_t *datafile, const char *path)
{
    datafile_zero(datafile);
    FILE *datafile_fp = fopen(path, READ_BINARY);
    fread(&datafile->y_size, sizeof(int), 1, datafile_fp);
    fread(&datafile->x_size, sizeof(int), 1, datafile_fp);
    datafile->y = malloc(datafile->y_size * sizeof(double));
    datafile->x = malloc(datafile->x_size * sizeof(double));
    fread(datafile->y, sizeof(double), datafile->y_size, datafile_fp);
    fread(datafile->x, sizeof(double), datafile->x_size, datafile_fp);
    datafile->h = malloc(datafile->y_size * sizeof(double *));
    for (int i = 0; i < datafile->y_size; i++)
    {
        datafile->h[i] = malloc(datafile->x_size * sizeof(double));
        fread(datafile->h[i], sizeof(double), datafile->x_size, datafile_fp);
    }
    fclose(datafile_fp);
}

void datafile_free(datafile_t *datafile)
{
    if (datafile->x != NULL)
        free(datafile->x);

    if (datafile->y != NULL)
        free(datafile->y);

    if (datafile->h != NULL)
    {
        for (int i = 0; i < datafile->x_size; i++)
            free(datafile->h[i]);
        free(datafile->h);
    }

    datafile_zero(datafile);
}

double datafile_get_nn(datafile_t *datafile, const double x, const double y)
{
    int closest_x_index;
    double closest_x = nneighbor(datafile->x, datafile->x_size, x, &closest_x_index);

    int closest_y_index;
    double closest_y = nneighbor(datafile->y, datafile->y_size, y, &closest_y_index);

    return datafile->h[closest_y_index][closest_x_index];
}

double datafile_get_bilinear(datafile_t *datafile, const double x, const double y)
{
    // Find adjacent x1 and x2 such that x1 <= x and x < x2
    int x1_index, x2_index;
    double x1, x2;
    for (int i = 0; i < datafile->x_size - 1; i++)
    {
        x1_index = i;
        x2_index = i + 1;
        x1 = datafile->x[x1_index];
        x2 = datafile->x[x2_index];
        if (x1 <= x && x < x2)
            break;
    }

    // Find adjacent y1 and y2 such that y1 <= y and y < y2
    int y1_index, y2_index;
    double y1, y2;
    for (int i = 0; i < datafile->y_size - 1; i++)
    {
        y1_index = i;
        y2_index = i + 1;
        y1 = datafile->y[y1_index];
        y2 = datafile->y[y2_index];
        if (y1 <= y && y < y2)
            break;
    }

    // Find the four heights at the corners of the rectangle
    double h11 = datafile->h[y1_index][x1_index];
    double h12 = datafile->h[y1_index][x2_index];
    double h21 = datafile->h[y2_index][x1_index];
    double h22 = datafile->h[y2_index][x2_index];

    // Find the bilinearly interpolated height
    double h = (h11 * (x2 - x) * (y2 - y) + h21 * (x - x1) * (y2 - y) + h12 * (x2 - x) * (y - y1) + h22 * (x - x1) * (y - y1)) / ((x2 - x1) * (y2 - y1));

    return h;
}