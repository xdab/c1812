#include "image.h"

#include <stdio.h>
#include <stdlib.h>

#define WRITE_BINARY "wb"
#define BMP_FILE_HEADER_SIZE 14
#define BMP_INFO_HEADER_SIZE 40
#define BMP_PAD_SIZE 3

#define R 0
#define G 1
#define B 2

#define _dealloc_channel_up_to_i(image, channel, i) \
	for (int j = 0; j < i; j++)                     \
		free(image->rgb_data[channel][j]);

#define _dealloc_channels(image) \
	free(image->rgb_data[R]);    \
	free(image->rgb_data[G]);    \
	free(image->rgb_data[B]);

int image_create(image_t *image, int width, int height)
{
	image->width = width;
	image->height = height;

	image->rgb_data[R] = malloc(height * sizeof(unsigned int *));
	if (image->rgb_data[R] == NULL)
	{
		fprintf(stderr, "image_create: malloc() image->rgb_data[R]\n");
		return EXIT_FAILURE;
	}

	image->rgb_data[G] = malloc(height * sizeof(unsigned int *));
	if (image->rgb_data[G] == NULL)
	{
		free(image->rgb_data[R]);
		fprintf(stderr, "image_create: malloc() image->rgb_data[G]\n");
		return EXIT_FAILURE;
	}

	image->rgb_data[B] = malloc(height * sizeof(unsigned int *));
	if (image->rgb_data[B] == NULL)
	{
		free(image->rgb_data[R]);
		free(image->rgb_data[G]);
		fprintf(stderr, "image_create: malloc() image->rgb_data[B]\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < height; i++)
	{
		image->rgb_data[R][i] = malloc(width * sizeof(unsigned int));
		if (image->rgb_data[R][i] == NULL)
		{
			fprintf(stderr, "image_create: malloc() image->rgb_data[R][%d]\n", i);
			_dealloc_channel_up_to_i(image, R, i);
			_dealloc_channels(image);
			return EXIT_FAILURE;
		}

		image->rgb_data[G][i] = malloc(width * sizeof(unsigned int));
		if (image->rgb_data[G][i] == NULL)
		{
			fprintf(stderr, "image_create: malloc() image->rgb_data[G][%d]\n", i);
			_dealloc_channel_up_to_i(image, R, height);
			_dealloc_channel_up_to_i(image, G, i);
			_dealloc_channels(image);
			return EXIT_FAILURE;
		}

		image->rgb_data[B][i] = malloc(width * sizeof(unsigned int));
		if (image->rgb_data[B][i] == NULL)
		{
			fprintf(stderr, "image_create: malloc() image->rgb_data[B][%d]\n", i);
			_dealloc_channel_up_to_i(image, R, height);
			_dealloc_channel_up_to_i(image, G, height);
			_dealloc_channel_up_to_i(image, B, i);
			_dealloc_channels(image);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int image_set(image_t *image, int x, int y, unsigned int r, unsigned int g, unsigned int b)
{
	if (x < 0 || x >= image->width || y < 0 || y >= image->height)
	{
		fprintf(stderr, "image_set: x=%d y=%d\n", x, y);
		return EXIT_FAILURE;
	}

	image->rgb_data[R][y][x] = r;
	image->rgb_data[G][y][x] = g;
	image->rgb_data[B][y][x] = b;

	return EXIT_SUCCESS;
}

int image_write(image_t *image, const char *filename)
{
	FILE *fp = fopen(filename, WRITE_BINARY);
	if (fp == NULL)
	{
		fprintf(stderr, "image_write: fopen(%s)\n", filename);
		return EXIT_FAILURE;
	}

	unsigned char bmp_file_header[BMP_FILE_HEADER_SIZE] = {
		0, 0,		// signature
		0, 0, 0, 0, // image file size in bytes
		0, 0, 0, 0, // reserved
		0, 0, 0, 0, // start of pixel array
	};

	int filesize = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE + BMP_PAD_SIZE * image->width * image->height;
	bmp_file_header[0] = (unsigned char)('B');
	bmp_file_header[1] = (unsigned char)('M');
	bmp_file_header[2] = (unsigned char)(filesize);
	bmp_file_header[3] = (unsigned char)(filesize >> 8);
	bmp_file_header[4] = (unsigned char)(filesize >> 16);
	bmp_file_header[5] = (unsigned char)(filesize >> 24);
	bmp_file_header[10] = (unsigned char)(BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE);

	if (fwrite(bmp_file_header, 1, BMP_FILE_HEADER_SIZE, fp) != BMP_FILE_HEADER_SIZE)
	{
		fprintf(stderr, "image_write: fwrite() bmp_file_header\n");
		(void)fclose(fp);
		return EXIT_FAILURE;
	}

	unsigned char bmp_info_header[BMP_INFO_HEADER_SIZE] = {
		0, 0, 0, 0, // header size
		0, 0, 0, 0, // image width
		0, 0, 0, 0, // image height
		0, 0,		// number of color planes
		0, 0,		// bits per pixel
		0, 0, 0, 0, // compression
		0, 0, 0, 0, // image size
		0, 0, 0, 0, // horizontal resolution
		0, 0, 0, 0, // vertical resolution
		0, 0, 0, 0, // colors in color table
		0, 0, 0, 0, // important color count
	};

	unsigned char bmp_pad[BMP_PAD_SIZE] = {0, 0, 0};
	bmp_info_header[0] = (unsigned char)(BMP_INFO_HEADER_SIZE);
	bmp_info_header[4] = (unsigned char)(image->width);
	bmp_info_header[5] = (unsigned char)(image->width >> 8);
	bmp_info_header[6] = (unsigned char)(image->width >> 16);
	bmp_info_header[7] = (unsigned char)(image->width >> 24);
	bmp_info_header[8] = (unsigned char)(image->height);
	bmp_info_header[9] = (unsigned char)(image->height >> 8);
	bmp_info_header[10] = (unsigned char)(image->height >> 16);
	bmp_info_header[11] = (unsigned char)(image->height >> 24);
	bmp_info_header[12] = (unsigned char)(1);
	bmp_info_header[14] = (unsigned char)(BYTES_PER_PIXEL * 8);

	if (fwrite(bmp_info_header, 1, BMP_INFO_HEADER_SIZE, fp) != BMP_INFO_HEADER_SIZE)
	{
		fprintf(stderr, "image_write: fwrite() bmp_info_header\n");
		(void)fclose(fp);
		return EXIT_FAILURE;
	}

	int padding = (4 - (image->width * BYTES_PER_PIXEL) % 4) % 4;
	unsigned char v;

	for (int y = 0; y < image->height; y++)
	{
		for (int x = 0; x < image->width; x++)
		{
			v = (unsigned char)(image->rgb_data[B][y][x]);
			if (fwrite(&v, 1, sizeof(unsigned char), fp) != sizeof(unsigned char))
			{
				fprintf(stderr, "image_write: fwrite() x=%d y=%d B\n", x, y);
				(void)fclose(fp);
				return EXIT_FAILURE;
			}

			v = (unsigned char)(image->rgb_data[G][y][x]);
			if (fwrite(&v, 1, sizeof(unsigned char), fp) != sizeof(unsigned char))
			{
				fprintf(stderr, "image_write: fwrite() x=%d y=%d G\n", x, y);
				(void)fclose(fp);
				return EXIT_FAILURE;
			}

			v = (unsigned char)(image->rgb_data[R][y][x]);
			if (fwrite(&v, 1, sizeof(unsigned char), fp) != sizeof(unsigned char))
			{
				fprintf(stderr, "image_write: fwrite() x=%d y=%d R\n", x, y);
				(void)fclose(fp);
				return EXIT_FAILURE;
			}
		}

		if (fwrite(bmp_pad, 1, padding, fp) != padding)
		{
			fprintf(stderr, "image_write: fwrite() y=%d padding\n", y);
			(void)fclose(fp);
			return EXIT_FAILURE;
		}
	}

	if (fclose(fp) != EXIT_SUCCESS)
	{
		fprintf(stderr, "image_write: fclose()\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void image_free(image_t *image)
{
	_dealloc_channel_up_to_i(image, R, image->height);
	_dealloc_channel_up_to_i(image, G, image->height);
	_dealloc_channel_up_to_i(image, B, image->height);
	_dealloc_channels(image);
}