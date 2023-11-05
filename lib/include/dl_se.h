#ifndef DL_SE_H
#define DL_SE_H

typedef struct {
    double d;
    double hte;
    double hre;
    double ap;
    double f;
    double lambda;
    double omega;
} dl_se_input_t;

typedef struct {
    double Ldsph[2];
} dl_se_output_t;

void dl_se(dl_se_input_t *input, dl_se_output_t *output);

#endif