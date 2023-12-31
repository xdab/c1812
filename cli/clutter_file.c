#include "clutter_file.h"
#include "nneighbor.h"
#include "c1812/custom_math.h"
#include <stdio.h>
#include <stdlib.h>

void cf_zero(clutter_file_t *ctfile)
{
	ctfile->y_size = 0;
	ctfile->x_size = 0;
	ctfile->y = NULL;
	ctfile->x = NULL;
	ctfile->Ct = NULL;
}

int cf_open(clutter_file_t *cf, const char *path)
{
	cf_zero(cf);

	FILE *fp = fopen(path, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "ctfile_open: fopen(%s) failed\n", path);
		return EXIT_FAILURE;
	}

	if (fread(&cf->y_size, sizeof(int), 1, fp) != 1)
	{
		fprintf(stderr, "ctfile_open: fread(y_size) failed\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	if (fread(&cf->x_size, sizeof(int), 1, fp) != 1)
	{
		fprintf(stderr, "ctfile_open: fread(x_size) failed\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	cf->y = malloc(cf->y_size * sizeof(double));
	if (cf->y == NULL)
	{
		fprintf(stderr, "ctfile_open: malloc(y) failed\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	if (fread(cf->y, sizeof(double), cf->y_size, fp) != cf->y_size)
	{
		fprintf(stderr, "ctfile_open: fread(y) failed\n");
		free(cf->y);
		fclose(fp);
		return EXIT_FAILURE;
	}

	cf->x = malloc(cf->x_size * sizeof(double));
	if (cf->x == NULL)
	{
		fprintf(stderr, "ctfile_open: malloc(x) failed\n");
		free(cf->y);
		fclose(fp);
		return EXIT_FAILURE;
	}

	if (fread(cf->x, sizeof(double), cf->x_size, fp) != cf->x_size)
	{
		fprintf(stderr, "ctfile_open: fread(x) failed\n");
		free(cf->y);
		free(cf->x);
		fclose(fp);
		return EXIT_FAILURE;
	}

	cf->Ct = malloc(cf->y_size * sizeof(uint8_t *));
	if (cf->Ct == NULL)
	{
		fprintf(stderr, "ctfile_open: malloc(Ct) failed\n");
		free(cf->y);
		free(cf->x);
		fclose(fp);
		return EXIT_FAILURE;
	}

	for (int y = 0; y < cf->y_size; y++)
	{
		cf->Ct[y] = malloc(cf->x_size * sizeof(uint8_t));
		if (cf->Ct[y] == NULL)
		{
			fprintf(stderr, "ctfile_open: malloc(Ct[y]) failed\n");
			for (int i = 0; i < y; i++)
				free(cf->Ct[i]);
			free(cf->Ct);
			free(cf->y);
			free(cf->x);
			fclose(fp);
			return EXIT_FAILURE;
		}

		if (fread(cf->Ct[y], sizeof(uint8_t), cf->x_size, fp) != cf->x_size)
		{
			fprintf(stderr, "ctfile_open: fread(Ct[y]) failed\n");
			for (int i = 0; i <= y; i++)
				free(cf->Ct[i]);
			free(cf->Ct);
			free(cf->y);
			free(cf->x);
			fclose(fp);
			return EXIT_FAILURE;
		}
	}

	fclose(fp);
	return EXIT_SUCCESS;
}

void cf_free(clutter_file_t *cf)
{
	for (int y = 0; y < cf->y_size; y++)
		free(cf->Ct[y]);
	free(cf->Ct);
	free(cf->y);
	free(cf->x);
	cf_zero(cf);
}

double cf_get_nn(clutter_file_t *cf, const double x, const double y)
{
	int closest_x_index;
	double closest_x = nneighbor(cf->x, cf->x_size, x, &closest_x_index);

	int closest_y_index;
	double closest_y = nneighbor(cf->y, cf->y_size, y, &closest_y_index);

	return (double) cf->Ct[closest_y_index][closest_x_index];
}

double cf_get_bilinear(clutter_file_t *cf, const double x, const double y)
{
	int x1_index;
	double x1 = nneighbor(cf->x, cf->x_size, x, &x1_index);
	if (x1 > x)
	{
		x1_index--;
		if (x1_index < 0)
			return 0;
		x1 = cf->x[x1_index];
	}

	int x2_index = x1_index + 1;
	if (x2_index >= cf->x_size)
		return 0;
	double x2 = cf->x[x2_index];

	int y1_index;
	double y1 = nneighbor(cf->y, cf->y_size, y, &y1_index);
	if (y1 > y)
	{
		y1_index--;
		if (y1_index < 0)
			return 0;
		y1 = cf->y[y1_index];
	}

	int y2_index = y1_index + 1;
	if (y2_index >= cf->y_size)
		return 0;
	double y2 = cf->y[y2_index];

	// Find the clutter heights at the corners of the rectangle
	double Ct11 = (double)cf->Ct[y1_index][x1_index];
	double Ct12 = (double)cf->Ct[y1_index][x2_index];
	double Ct21 = (double)cf->Ct[y2_index][x1_index];
	double Ct22 = (double)cf->Ct[y2_index][x2_index];

	// Find the heights at the edges of the rectangle
	double Ct1 = Ct11 + (Ct12 - Ct11) * (x - x1) / (x2 - x1);
	double Ct2 = Ct21 + (Ct22 - Ct21) * (x - x1) / (x2 - x1);

	// Find the height at the point
	double Ct = Ct1 + (Ct2 - Ct1) * (y - y1) / (y2 - y1);

	return Ct;
}