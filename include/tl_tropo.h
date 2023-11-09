#ifndef TL_TROPO_H
#define TL_TROPO_H

typedef struct
{
	double dtot;
	double theta;
	double f;
	double p;
	double N0;
} tl_tropo_input_t;

typedef struct
{
	double Lbs;
} tl_tropo_output_t;

void tl_tropo(tl_tropo_input_t *input, tl_tropo_output_t *output);

#endif