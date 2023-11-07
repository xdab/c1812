#ifndef OUTFILE_H
#define OUTFILE_H

#include <stdio.h>

typedef struct
{
    FILE *file;
    int n;
} outfile_t;

int outfile_open(outfile_t *outfile, const char *filename);
int outfile_write_header(outfile_t *outfile, double txx, double txy, double radius, double ares, int n);
int outfile_write_ray(outfile_t *outfile, double *Lb);
int outfile_close(outfile_t *outfile);

#endif