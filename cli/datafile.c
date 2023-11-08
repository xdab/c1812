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

int _datafile_parse_first_stage(datafile_t *datafile, FILE *file);
int _datafile_parse_second_stage(datafile_t *datafile, FILE *file);

void datafile_zero(datafile_t *datafile)
{
    datafile->x = NULL;
    datafile->x_size = 0;

    datafile->y = NULL;
    datafile->y_size = 0;

    datafile->h = NULL;
}

int datafile_parse(datafile_t *datafile, const char *path)
{
    datafile_zero(datafile);

    FILE *datafile_fp = fopen(path, READ);
    if (datafile_fp == NULL)
    {
        fprintf(stderr, "datafile_parse: fopen()\n");
        return EXIT_FAILURE;
    }

    if (_datafile_parse_first_stage(datafile, datafile_fp))
    {
        fprintf(stderr, "datafile_parse: _datafile_parse_first_stage()\n");
        return EXIT_FAILURE;
    }

    rewind(datafile_fp);

    if (_datafile_parse_second_stage(datafile, datafile_fp))
    {
        fprintf(stderr, "datafile_parse: _datafile_parse_second_stage()\n");
        return EXIT_FAILURE;
    }

    if (fclose(datafile_fp))
    {
        fprintf(stderr, "datafile_parse: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int _datafile_parse_first_stage(datafile_t *datafile, FILE *file)
{
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

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
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
            switch (token_index)
            {
            case X_TOKEN_INDEX:
                x = atof(token);
                if (x != last_x)
                {
                    vec_double_sorted_unique_insert(&x_vec, x);
                    last_x = x;
                }
                break;

            case Y_TOKEN_INDEX:
                y = atof(token);
                if (y != last_y)
                {
                    vec_double_sorted_unique_insert(&y_vec, y);
                    last_y = y;
                }
                break;

            case H_TOKEN_INDEX:
                break; // Ignore for this pass

            default:
                fprintf(stderr, "datafile_parse: invalid token index %d\n", token_index);
                return EXIT_FAILURE;
            }

            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }
    }

    datafile->x_size = x_vec.length;
    datafile->x = malloc(datafile->x_size * sizeof(double));
    if (datafile->x == NULL)
    {
        fprintf(stderr, "datafile_parse: malloc() x\n");
        return EXIT_FAILURE;
    }
    memcpy(datafile->x, x_vec.data, datafile->x_size * sizeof(double));
    vec_deinit(&x_vec);

    datafile->y_size = y_vec.length;
    datafile->y = malloc(datafile->y_size * sizeof(double));
    if (datafile->y == NULL)
    {
        fprintf(stderr, "datafile_parse: malloc() y\n");
        return EXIT_FAILURE;
    }
    memcpy(datafile->y, y_vec.data, datafile->y_size * sizeof(double));
    vec_deinit(&y_vec);

    datafile->h = malloc(datafile->y_size * sizeof(double *));
    if (datafile->h == NULL)
    {
        fprintf(stderr, "datafile_parse: malloc() h\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < datafile->y_size; i++)
    {
        datafile->h[i] = malloc(datafile->x_size * sizeof(double));
        if (datafile->h[i] == NULL)
        {
            fprintf(stderr, "datafile_parse: malloc() h @ y=%f\n", datafile->y[i]);
            return EXIT_FAILURE;
        }

        for (int j = 0; j < datafile->x_size; j++)
            datafile->h[i][j] = NAN;
    }

    return EXIT_SUCCESS;
}

int _datafile_parse_second_stage(datafile_t *datafile, FILE *file)
{
    char line[MAX_LINE_LENGTH + 1];
    int line_index = 0;

    // Optimization of nneighbor() calls
    double last_x = NAN;
    double last_y = NAN;
    int last_x_index = -1;
    int last_y_index = -1;

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
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
                datafile->h[last_y_index][last_x_index] = atof(token);
                break;
            }

            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }
    }

    return EXIT_SUCCESS;
}

int datafile_store(datafile_t *datafile, const char *path)
{
    FILE *datafile_fp = fopen(path, WRITE_BINARY);
    if (datafile_fp == NULL)
    {
        fprintf(stderr, "datafile_store: fopen()\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&datafile->y_size, sizeof(int), 1, datafile_fp) != 1)
    {
        fprintf(stderr, "datafile_store: fwrite() y_size\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&datafile->x_size, sizeof(int), 1, datafile_fp) != 1)
    {
        fprintf(stderr, "datafile_store: fwrite() x_size\n");
        return EXIT_FAILURE;
    }

    if (fwrite(datafile->y, sizeof(double), datafile->y_size, datafile_fp) != datafile->y_size)
    {
        fprintf(stderr, "datafile_store: fwrite() y\n");
        return EXIT_FAILURE;
    }

    if (fwrite(datafile->x, sizeof(double), datafile->x_size, datafile_fp) != datafile->x_size)
    {
        fprintf(stderr, "datafile_store: fwrite() x\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < datafile->y_size; i++)
    {
        if (fwrite(datafile->h[i], sizeof(double), datafile->x_size, datafile_fp) != datafile->x_size)
        {
            fprintf(stderr, "datafile_store: fwrite() h @ y=%f\n", datafile->y[i]);
            return EXIT_FAILURE;
        }
    }

    if (fclose(datafile_fp))
    {
        fprintf(stderr, "datafile_store: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int datafile_open(datafile_t *datafile, const char *path)
{
    datafile_zero(datafile);

    FILE *datafile_fp = fopen(path, READ_BINARY);
    if (datafile_fp == NULL)
    {
        fprintf(stderr, "datafile_open: fopen()\n");
        return EXIT_FAILURE;
    }

    if (fread(&datafile->y_size, sizeof(int), 1, datafile_fp) != 1)
    {
        fprintf(stderr, "datafile_open: fread() y_size\n");
        return EXIT_FAILURE;
    }

    if (fread(&datafile->x_size, sizeof(int), 1, datafile_fp) != 1)
    {
        fprintf(stderr, "datafile_open: fread() x_size\n");
        return EXIT_FAILURE;
    }

    datafile->y = malloc(datafile->y_size * sizeof(double));
    if (datafile->y == NULL)
    {
        fprintf(stderr, "datafile_open: malloc() y\n");
        return EXIT_FAILURE;
    }

    datafile->x = malloc(datafile->x_size * sizeof(double));
    if (datafile->x == NULL)
    {
        fprintf(stderr, "datafile_open: malloc() x\n");
        return EXIT_FAILURE;
    }

    if (fread(datafile->y, sizeof(double), datafile->y_size, datafile_fp) != datafile->y_size)
    {
        fprintf(stderr, "datafile_open: fread() y\n");
        return EXIT_FAILURE;
    }

    if (fread(datafile->x, sizeof(double), datafile->x_size, datafile_fp) != datafile->x_size)
    {
        fprintf(stderr, "datafile_open: fread() x\n");
        return EXIT_FAILURE;
    }

    datafile->h = malloc(datafile->y_size * sizeof(double *));
    if (datafile->h == NULL)
    {
        fprintf(stderr, "datafile_open: malloc() h\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < datafile->y_size; i++)
    {
        datafile->h[i] = malloc(datafile->x_size * sizeof(double));
        if (datafile->h[i] == NULL)
        {
            fprintf(stderr, "datafile_open: malloc() h @ y=%f\n", datafile->y[i]);
            return EXIT_FAILURE;
        }

        if (fread(datafile->h[i], sizeof(double), datafile->x_size, datafile_fp) != datafile->x_size)
        {
            fprintf(stderr, "datafile_open: fread() h @ y=%f\n", datafile->y[i]);
            return EXIT_FAILURE;
        }
    }

    if (fclose(datafile_fp))
    {
        fprintf(stderr, "datafile_open: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
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
    int x1_index;
    double x1 = nneighbor(datafile->x, datafile->x_size, x, &x1_index);
    if (x1 > x)
    {
        x1_index--;
        if (x1_index < 0)
            return NAN;
        x1 = datafile->x[x1_index];
    }

    int x2_index = x1_index + 1;
    if (x2_index >= datafile->x_size)
        return NAN;
    double x2 = datafile->x[x2_index];

    int y1_index;
    double y1 = nneighbor(datafile->y, datafile->y_size, y, &y1_index);
    if (y1 > y)
    {
        y1_index--;
        if (y1_index < 0)
            return NAN;
        y1 = datafile->y[y1_index];
    }

    int y2_index = y1_index + 1;
    if (y2_index >= datafile->y_size)
        return NAN;
    double y2 = datafile->y[y2_index];

    // Find the heights at the corners of the rectangle
    double h11 = datafile->h[y1_index][x1_index];
    double h12 = datafile->h[y1_index][x2_index];
    double h21 = datafile->h[y2_index][x1_index];
    double h22 = datafile->h[y2_index][x2_index];

    // Find the heights at the edges of the rectangle
    double h1 = h11 + (h12 - h11) * (x - x1) / (x2 - x1);
    double h2 = h21 + (h22 - h21) * (x - x1) / (x2 - x1);

    // Find the height at the point
    double h = h1 + (h2 - h1) * (y - y1) / (y2 - y1);

    return h;
}