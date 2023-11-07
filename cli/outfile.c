#include "outfile.h"
#include <stdio.h>
#include <stdlib.h>

#define WRITE_BINARY "wb"

int outfile_open(outfile_t *outfile, const char *filename)
{
    outfile->file = fopen(filename, WRITE_BINARY);
    if (outfile->file == NULL)
    {
        fprintf(stderr, "outfile_open: fopen()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int outfile_write_header(outfile_t *outfile, double txx, double txy, double radius, double ares, int n)
{
    outfile->n = n;

    if (fwrite(&txx, sizeof(double), 1, outfile->file) != 1)
    {
        fprintf(stderr, "outfile_write_header: fwrite() txx\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&txy, sizeof(double), 1, outfile->file) != 1)
    {
        fprintf(stderr, "outfile_write_header: fwrite() txy\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&radius, sizeof(double), 1, outfile->file) != 1)
    {
        fprintf(stderr, "outfile_write_header: fwrite() radius\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&ares, sizeof(double), 1, outfile->file) != 1)
    {
        fprintf(stderr, "outfile_write_header: fwrite() ares\n");
        return EXIT_FAILURE;
    }

    if (fwrite(&n, sizeof(int), 1, outfile->file) != 1)
    {
        fprintf(stderr, "outfile_write_header: fwrite() n\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int outfile_write_ray(outfile_t *outfile, double *ray)
{
    if (fwrite(ray, sizeof(double), outfile->n, outfile->file) != outfile->n)
    {
        fprintf(stderr, "outfile_write_ray: fwrite()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int outfile_close(outfile_t *outfile)
{
    if (fclose(outfile->file))
    {
        fprintf(stderr, "outfile_close: fclose()\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}