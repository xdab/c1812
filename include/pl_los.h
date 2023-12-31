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
    double Lbfs; // Basic transmission loss due to free-space propagation
    double Lb0p; // Basic transmission loss not exceeded for time percentage, p%, due to LoS propagation
    double Lb0b; // Basic transmission loss not exceedd for time percentage, b0%, due to LoS propagation
} pl_los_output_t;

void pl_los(pl_los_input_t *input, pl_los_output_t *output);

#endif