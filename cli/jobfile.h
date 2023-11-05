#ifndef JOBFILE_H
#define JOBFILE_H

#include "c1812/parameters.h"

#define MAX_LINE_LENGTH 256
#define MAX_FIELD_LENGTH 32
#define MAX_VALUE_LENGTH (MAX_LINE_LENGTH - MAX_FIELD_LENGTH)
#define MAX_DATA_FILES 16

typedef struct
{
    double txx;
    double txy;
    double txh;
    double txpwr;
    double txgain;

    double rxx;
    double rxy;
    double rxh;
    double rxgain;

    double radius;
    double xres;
    double ares;

    char data[MAX_DATA_FILES][MAX_VALUE_LENGTH];

} job_parameters_t;

void jobfile_zero(job_parameters_t *job_parameters);
void jobfile_read(job_parameters_t *job_parameters, c1812_parameters_t *parameters, const char *path);
void jobfile_set_field(job_parameters_t *job_parameters, c1812_parameters_t *parameters, char *field, char *value);

#endif