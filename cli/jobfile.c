#include "jobfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define READ "r"

#define FIELD_TOKEN_INDEX 0
#define VALUE_TOKEN_INDEX 1
#define COMMENT_CHAR '#'
#define SPLIT_CHARS " \n"
#define EQUAL 0
#define FIELD_FREQ "freq"
#define FIELD_POL "pol"
#define FIELD_POL_HORIZONTAL "horizontal"
#define FIELD_POL_VERTICAL "vertical"
#define FIELD_ZONE "zone"
#define FIELD_ZONE_INLAND "inland"
#define FIELD_ZONE_COASTAL "coastal"
#define FIELD_ZONE_SEA "sea"
#define FIELD_LAT "lat"
#define FIELD_LON "lon"
#define FIELD_N0 "n0"
#define FIELD_DN "dn"
#define FIELD_TXX "txx"
#define FIELD_TXY "txy"
#define FIELD_TXH "txh"
#define FIELD_TXPWR "txpwr"
#define FIELD_TXGAIN "txgain"
#define FIELD_RXX "rxx"
#define FIELD_RXY "rxy"
#define FIELD_RXH "rxh"
#define FIELD_RXGAIN "rxgain"
#define FIELD_P "p"
#define FIELD_RADIUS "radius"
#define FIELD_XRES "xres"
#define FIELD_ARES "ares"
#define FIELD_DATA "data"

void jobfile_zero(job_parameters_t *job_parameters)
{
    job_parameters->txx = 0.0;
    job_parameters->txy = 0.0;
    job_parameters->txh = 0.0;
    job_parameters->txpwr = 0.0;
    job_parameters->txgain = 0.0;

    job_parameters->rxx = NAN;
    job_parameters->rxy = NAN;
    job_parameters->rxh = 0.0;
    job_parameters->rxgain = 0.0;

    job_parameters->radius = 0.0;
    job_parameters->xres = 0.0;
    job_parameters->ares = 0.0;

    memset(job_parameters->data, 0, sizeof(job_parameters->data));
}

void jobfile_read(job_parameters_t *job_parameters, c1812_parameters_t *parameters, const char *path)
{
    jobfile_zero(job_parameters);

    FILE *jobfile = fopen(path, READ);
    if (jobfile == NULL)
        exit(EXIT_FAILURE);

    char line[MAX_LINE_LENGTH + 1];
    char field[MAX_FIELD_LENGTH + 1];
    char value[MAX_VALUE_LENGTH + 1];
    int line_index = 0;
    while (fgets(line, MAX_LINE_LENGTH, jobfile) != NULL)
    {
        line_index++;
        int len = strlen(line);
        if (len <= 1)
            continue;
        if (line[0] == COMMENT_CHAR)
            continue;

        int token_index = 0;
        char *token = strtok(line, SPLIT_CHARS);
        while (token != NULL)
        {
            if (token_index == FIELD_TOKEN_INDEX)
                strncpy(field, token, MAX_FIELD_LENGTH);
            else if (token_index == VALUE_TOKEN_INDEX)
                strncpy(value, token, MAX_VALUE_LENGTH);
            else
            {
                fprintf(stderr, "Too many tokens on line %d\n", line_index);
                exit(EXIT_FAILURE);
            }
            token = strtok(NULL, SPLIT_CHARS);
            token_index++;
        }

        if (token_index != 2)
        {
            fprintf(stderr, "Too few tokens on line %d\n", line_index);
            exit(EXIT_FAILURE);
        }

        jobfile_set_field(job_parameters, parameters, field, value);
    }

    fclose(jobfile);
}

void jobfile_set_field(job_parameters_t *job_parameters, c1812_parameters_t *parameters, char *field, char *value)
{
    for (int i = 0; i < strlen(field); i++)
        field[i] = tolower(field[i]);

    if (strcmp(field, FIELD_FREQ) == EQUAL)
        parameters->f = atof(value);
    else if (strcmp(field, FIELD_POL) == EQUAL)
    {
        for (int i = 0; i < strlen(value); i++)
            value[i] = tolower(value[i]);
        if (strcmp(value, FIELD_POL_HORIZONTAL) == EQUAL)
            parameters->pol = POLARIZATION_HORIZONTAL;
        else if (strcmp(value, FIELD_POL_VERTICAL) == EQUAL)
            parameters->pol = POLARIZATION_VERTICAL;
        else
        {
            fprintf(stderr, "Polarization (%s) must be either horizontal or vertical, not %s\n", FIELD_POL, value);
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(field, FIELD_ZONE) == EQUAL)
    {
        for (int i = 0; i < strlen(value); i++)
            value[i] = tolower(value[i]);
        if (strcmp(value, FIELD_ZONE_INLAND) == EQUAL)
            parameters->zone = RC_ZONE_INLAND;
        else if (strcmp(value, FIELD_ZONE_COASTAL) == EQUAL)
            parameters->zone = RC_ZONE_COASTAL_LAND;
        else if (strcmp(value, FIELD_ZONE_SEA) == EQUAL)
            parameters->zone = RC_ZONE_SEA;
        else
        {
            fprintf(stderr, "Zone (%s) must be either inland, coastal or sea, not %s\n", FIELD_ZONE, value);
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(field, FIELD_LAT) == EQUAL)
        parameters->lat = atof(value);
    else if (strcmp(field, FIELD_LON) == EQUAL)
        parameters->lon = atof(value);
    else if (strcmp(field, FIELD_N0) == EQUAL)
        parameters->N0 = atof(value);
    else if (strcmp(field, FIELD_DN) == EQUAL)
        parameters->DN = atof(value);
    else if (strcmp(field, FIELD_TXX) == EQUAL)
        job_parameters->txx = atof(value);
    else if (strcmp(field, FIELD_TXY) == EQUAL)
        job_parameters->txy = atof(value);
    else if (strcmp(field, FIELD_TXH) == EQUAL)
        parameters->htg = atof(value);
    else if (strcmp(field, FIELD_TXPWR) == EQUAL)
        job_parameters->txpwr = atof(value);
    else if (strcmp(field, FIELD_TXGAIN) == EQUAL)
        job_parameters->txgain = atof(value);
    else if (strcmp(field, FIELD_RXX) == EQUAL)
        job_parameters->rxx = atof(value);
    else if (strcmp(field, FIELD_RXY) == EQUAL)
        job_parameters->rxy = atof(value);
    else if (strcmp(field, FIELD_RXH) == EQUAL)
        parameters->hrg = atof(value);
    else if (strcmp(field, FIELD_RXGAIN) == EQUAL)
        job_parameters->rxgain = atof(value);
    else if (strcmp(field, FIELD_P) == EQUAL)
        parameters->p = atof(value);
    else if (strcmp(field, FIELD_RADIUS) == EQUAL)
        job_parameters->radius = atof(value);
    else if (strcmp(field, FIELD_XRES) == EQUAL)
        job_parameters->xres = atof(value);
    else if (strcmp(field, FIELD_ARES) == EQUAL)
        job_parameters->ares = atof(value);
    else if (strcmp(field, FIELD_DATA) == EQUAL)
    {
        int i = 0;
        while (i < MAX_DATA_FILES && strlen(job_parameters->data[i]) > 0)
            i++;
        if (i == MAX_DATA_FILES)
        {
            fprintf(stderr, "Too many data files\n");
            exit(EXIT_FAILURE);
        }
        strncpy(job_parameters->data[i], value, MAX_VALUE_LENGTH);
    }
    else
    {
        fprintf(stderr, "Invalid field: %s\n", field);
        exit(EXIT_FAILURE);
    }
}