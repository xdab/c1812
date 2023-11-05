#ifndef DL_SE_FT_INNER_H
#define DL_SE_FT_INNER_H

typedef struct {
    double epsr;
    double sigma;
    double d;
    double hte;
    double hre;
    double adft;
    double f;
} dl_se_ft_inner_input_t;

typedef struct {
    double Ldft[2];
} dl_se_ft_inner_output_t;

void dl_se_ft_inner(dl_se_ft_inner_input_t *input, dl_se_ft_inner_output_t *output);

#endif