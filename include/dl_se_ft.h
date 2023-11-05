#ifndef DL_SE_FT_H
#define DL_SE_FT_H

typedef struct
{
    double d;
    double hte;
    double hre;
    double ap;
    double f;
    double lambda;
    double omega;
} dl_se_ft_input_t;

typedef struct
{
    double Ldft[2];
} dl_se_ft_output_t;

void dl_se_ft(dl_se_ft_input_t *input, dl_se_ft_output_t *output);

#endif