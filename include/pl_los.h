#ifndef PL_LOS_H
#define PL_LOS_H

typedef struct
{
    double d;
    double hts;
    double hrs;
    double f;
    double p;
    double b0;
    double dlt;
    double dlr;
} pl_los_input_t;

typedef struct
{
    double Lbfs;
    double Lb0p;
    double Lb0b;
} pl_los_output_t;

void pl_los(pl_los_input_t *input, pl_los_output_t *output);

#endif