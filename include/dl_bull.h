#ifndef DL_BULL_H
#define DL_BULL_H

typedef struct
{
    int n;
    double *d;
    double *h;
    double *Ct;
    double hts;
    double hrs;
    double ap;
    double f;
    double lambda;
    double dtot;
} dl_bull_input_t;

typedef struct
{
    double Lbull;
} dl_bull_output_t;

void dl_bull(dl_bull_input_t *input, dl_bull_output_t *output);

#endif