#include "terrain_file.h"
#include "vec.h"
#include "nneighbor.h"
#include "c1812/custom_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ "r"
#define READ_BINARY "rb"
#define WRITE_BINARY "wb"
#define MAX_VALUE_LENGTH 20
#define MAX_LINE_LENGTH (3 * MAX_VALUE_LENGTH + 2)
#define SPLIT_CHARS " \n"
#define X_TOKEN_INDEX 0
#define Y_TOKEN_INDEX 1
#define H_TOKEN_INDEX 2

int _tf_parse_first_stage(terrain_file_t *tf, FILE *file);
int _tf_parse_second_stage(terrain_file_t *tf, FILE *file);

void tf_zero(terrain_file_t *tf)
{
    tf->x = NULL;
    tf->x_size = 0;

    tf->y = NULL;
    tf->y_size = 0;

    tf->h = NULL;
}

int tf_parse(terrain_file_t *tf, const char *path)
{
    tf_zero(tf);

    FILE *tf_fp = fopen(path, READ);
    if (tf_fp == NULL)
    {
        fprintf(stderr, "tf_parse: fopen()\n");
        return EXIT_FAILURE;
    }

    if (_tf_parse_first_stage(tf, tf_fp))
    {
        fprintf(stderr, "tf_parse: _tf_parse_first_stage()\n");
        return EXIT_FAILURE;
    }

    rewind(tf_fp);

    if (_tf_parse_second_stage(tf, tf_fp))
    {
        fprintf(stderr, "tf_parse: _tf_parse_second_stage()\n");
        return EXIT_FAILURE;
    }

    if (fclose(tf_fp))
    {
        fprintf(stderr, "tf_parse: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int _tf_parse_first_stage(terrain_file_t *tf, FILE *file)
{
    char line[MAX_LINE_LENGTH + 1];
    int line_index = 0;

    vec_double_t x_vec;
    vec_init(&x_vec);

    vec_double_t y_vec;
    vec_init(&y_vec);

    // Optimization:
    // Since the file is likely sorted by x or y, we can avoid
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
                fprintf(stderr, "tf_parse: invalid token index %d\n", token_index);
                return EXIT_FAILURE;
            }

            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }
    }

    tf->x_size = x_vec.length;
    tf->x = malloc(tf->x_size * sizeof(double));
    if (tf->x == NULL)
    {
        fprintf(stderr, "tf_parse: malloc() x\n");
        return EXIT_FAILURE;
    }
    memcpy(tf->x, x_vec.data, tf->x_size * sizeof(double));
    vec_deinit(&x_vec);

    tf->y_size = y_vec.length;
    tf->y = malloc(tf->y_size * sizeof(double));
    if (tf->y == NULL)
    {
        fprintf(stderr, "tf_parse: malloc() y\n");
        free(tf->x);
        return EXIT_FAILURE;
    }
    memcpy(tf->y, y_vec.data, tf->y_size * sizeof(double));
    vec_deinit(&y_vec);

    tf->h = malloc(tf->y_size * sizeof(double *));
    if (tf->h == NULL)
    {
        fprintf(stderr, "tf_parse: malloc() h\n");
        free(tf->x);
        free(tf->y);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < tf->y_size; i++)
    {
        tf->h[i] = malloc(tf->x_size * sizeof(double));
        if (tf->h[i] == NULL)
        {
            fprintf(stderr, "tf_parse: malloc() h @ y=%f\n", tf->y[i]);
            free(tf->x);
            free(tf->y);
            for (int j = 0; j <= i; j++)
                free(tf->h[j]);
            free(tf->h);
            return EXIT_FAILURE;
        }

        for (int j = 0; j < tf->x_size; j++)
            tf->h[i][j] = NAN;
    }

    return EXIT_SUCCESS;
}

int _tf_parse_second_stage(terrain_file_t *tf, FILE *file)
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
                    nneighbor(tf->x, tf->x_size, x, &last_x_index);
                    last_x = x;
                }
            }
            else if (token_index == Y_TOKEN_INDEX)
            {
                y = atof(token);
                if (y != last_y)
                {
                    nneighbor(tf->y, tf->y_size, y, &last_y_index);
                    last_y = y;
                }
            }
            else if (token_index == H_TOKEN_INDEX)
            {
                tf->h[last_y_index][last_x_index] = atof(token);
                break;
            }

            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }
    }

    return EXIT_SUCCESS;
}

int tf_store(terrain_file_t *tf, const char *path)
{
    FILE *tf_fp = fopen(path, WRITE_BINARY);
    if (tf_fp == NULL)
    {
        fprintf(stderr, "tf_store: fopen()\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&tf->y_size, sizeof(int), 1, tf_fp) != 1)
    {
        fprintf(stderr, "tf_store: fwrite() y_size\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&tf->x_size, sizeof(int), 1, tf_fp) != 1)
    {
        fprintf(stderr, "tf_store: fwrite() x_size\n");
        return EXIT_FAILURE;
    }

    if (fwrite(tf->y, sizeof(double), tf->y_size, tf_fp) != tf->y_size)
    {
        fprintf(stderr, "tf_store: fwrite() y\n");
        return EXIT_FAILURE;
    }

    if (fwrite(tf->x, sizeof(double), tf->x_size, tf_fp) != tf->x_size)
    {
        fprintf(stderr, "tf_store: fwrite() x\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < tf->y_size; i++)
    {
        if (fwrite(tf->h[i], sizeof(double), tf->x_size, tf_fp) != tf->x_size)
        {
            fprintf(stderr, "tf_store: fwrite() h @ y=%f\n", tf->y[i]);
            return EXIT_FAILURE;
        }
    }

    if (fclose(tf_fp))
    {
        fprintf(stderr, "tf_store: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int tf_open(terrain_file_t *tf, const char *path)
{
    tf_zero(tf);

    FILE *tf_fp = fopen(path, READ_BINARY);
    if (tf_fp == NULL)
    {
        fprintf(stderr, "tf_open: fopen()\n");
        return EXIT_FAILURE;
    }

    if (fread(&tf->y_size, sizeof(int), 1, tf_fp) != 1)
    {
        fprintf(stderr, "tf_open: fread() y_size\n");
        return EXIT_FAILURE;
    }

    if (fread(&tf->x_size, sizeof(int), 1, tf_fp) != 1)
    {
        fprintf(stderr, "tf_open: fread() x_size\n");
        return EXIT_FAILURE;
    }

    tf->y = malloc(tf->y_size * sizeof(double));
    if (tf->y == NULL)
    {
        fprintf(stderr, "tf_open: malloc() y\n");
        return EXIT_FAILURE;
    }

    tf->x = malloc(tf->x_size * sizeof(double));
    if (tf->x == NULL)
    {
        fprintf(stderr, "tf_open: malloc() x\n");
        return EXIT_FAILURE;
    }

    if (fread(tf->y, sizeof(double), tf->y_size, tf_fp) != tf->y_size)
    {
        fprintf(stderr, "tf_open: fread() y\n");
        return EXIT_FAILURE;
    }

    if (fread(tf->x, sizeof(double), tf->x_size, tf_fp) != tf->x_size)
    {
        fprintf(stderr, "tf_open: fread() x\n");
        return EXIT_FAILURE;
    }

    tf->h = malloc(tf->y_size * sizeof(double *));
    if (tf->h == NULL)
    {
        fprintf(stderr, "tf_open: malloc() h\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < tf->y_size; i++)
    {
        tf->h[i] = malloc(tf->x_size * sizeof(double));
        if (tf->h[i] == NULL)
        {
            fprintf(stderr, "tf_open: malloc() h @ y=%f\n", tf->y[i]);
            return EXIT_FAILURE;
        }

        if (fread(tf->h[i], sizeof(double), tf->x_size, tf_fp) != tf->x_size)
        {
            fprintf(stderr, "tf_open: fread() h @ y=%f\n", tf->y[i]);
            return EXIT_FAILURE;
        }
    }

    if (fclose(tf_fp))
    {
        fprintf(stderr, "tf_open: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void tf_free(terrain_file_t *tf)
{
    if (tf->x != NULL)
        free(tf->x);

    if (tf->y != NULL)
        free(tf->y);

    if (tf->h != NULL)
    {
        for (int i = 0; i < tf->x_size; i++)
            free(tf->h[i]);
        free(tf->h);
    }

    tf_zero(tf);
}

double tf_get_nn(terrain_file_t *tf, const double x, const double y)
{
    int closest_x_index;
    double closest_x = nneighbor(tf->x, tf->x_size, x, &closest_x_index);

    int closest_y_index;
    double closest_y = nneighbor(tf->y, tf->y_size, y, &closest_y_index);

    return tf->h[closest_y_index][closest_x_index];
}

double tf_get_bilinear(terrain_file_t *tf, const double x, const double y)
{
    int x1_index;
    double x1 = nneighbor(tf->x, tf->x_size, x, &x1_index);
    if (x1 > x)
    {
        x1_index--;
        if (x1_index < 0)
            return NAN;
        x1 = tf->x[x1_index];
    }

    int x2_index = x1_index + 1;
    if (x2_index >= tf->x_size)
        return NAN;
    double x2 = tf->x[x2_index];

    int y1_index;
    double y1 = nneighbor(tf->y, tf->y_size, y, &y1_index);
    if (y1 > y)
    {
        y1_index--;
        if (y1_index < 0)
            return NAN;
        y1 = tf->y[y1_index];
    }

    int y2_index = y1_index + 1;
    if (y2_index >= tf->y_size)
        return NAN;
    double y2 = tf->y[y2_index];

    double h11 = tf->h[y1_index][x1_index];
    double h12 = tf->h[y1_index][x2_index];
    double h21 = tf->h[y2_index][x1_index];
    double h22 = tf->h[y2_index][x2_index];

    double h1 = h11 + (h12 - h11) * (x - x1) / (x2 - x1);
    double h2 = h21 + (h22 - h21) * (x - x1) / (x2 - x1);

    double h = h1 + (h2 - h1) * (y - y1) / (y2 - y1);

    return h;
}

double _tf_cubic(double A, double B, double C, double D, double x)
{
    float p = -A / 2 + (3 * B) / 2 - (3 * C) / 2 + D / 2;
    float q = A - (5 * B) / 2 + 2 * C - D / 2;
    float r = -A / 2 + C / 2;
    float s = B;
    return x * (x * (p * x + q) + r) + s;
}

double tf_get_bicubic(terrain_file_t *tf, const double x, const double y)
{
    int x2_index;
    double x2 = nneighbor(tf->x, tf->x_size, x, &x2_index);
    if (x2 > x)
    {
        x2_index--;
        if (x2_index < 0)
            return NAN;
        x2 = tf->x[x2_index];
    }

    int x1_index = x2_index - 1;
    if (x1_index < 0)
        return NAN;
    double x1 = tf->x[x1_index];

    int x3_index = x2_index + 1;
    if (x3_index >= tf->x_size)
        return NAN;
    double x3 = tf->x[x3_index];

    int x4_index = x3_index + 1;
    if (x4_index >= tf->x_size)
        return NAN;
    double x4 = tf->x[x4_index];

    int y2_index;
    double y2 = nneighbor(tf->y, tf->y_size, y, &y2_index);
    if (y2 > y)
    {
        y2_index--;
        if (y2_index < 0)
            return NAN;
        y2 = tf->y[y2_index];
    }

    int y1_index = y2_index - 1;
    if (y1_index < 0)
        return NAN;
    double y1 = tf->y[y1_index];

    int y3_index = y2_index + 1;
    if (y3_index >= tf->y_size)
        return NAN;
    double y3 = tf->y[y3_index];

    int y4_index = y3_index + 1;
    if (y4_index >= tf->y_size)
        return NAN;
    double y4 = tf->y[y4_index];

    double h11 = tf->h[y1_index][x1_index];
    double h12 = tf->h[y2_index][x1_index];
    double h13 = tf->h[y3_index][x1_index];
    double h14 = tf->h[y4_index][x1_index];

    double h21 = tf->h[y1_index][x2_index];
    double h22 = tf->h[y2_index][x2_index];
    double h23 = tf->h[y3_index][x2_index];
    double h24 = tf->h[y4_index][x2_index];

    double h31 = tf->h[y1_index][x3_index];
    double h32 = tf->h[y2_index][x3_index];
    double h33 = tf->h[y3_index][x3_index];
    double h34 = tf->h[y4_index][x3_index];

    double h41 = tf->h[y1_index][x4_index];
    double h42 = tf->h[y2_index][x4_index];
    double h43 = tf->h[y3_index][x4_index];
    double h44 = tf->h[y4_index][x4_index];

    double tx = (x - x2) / (x3 - x2);
    double h1 = _tf_cubic(h11, h21, h31, h41, tx);
    double h2 = _tf_cubic(h12, h22, h32, h42, tx);
    double h3 = _tf_cubic(h13, h23, h33, h43, tx);
    double h4 = _tf_cubic(h14, h24, h34, h44, tx);

    double ty = (y - y2) / (y3 - y2);
    double h = _tf_cubic(h1, h2, h3, h4, ty);

    return h;
}
