#ifndef S_UNIT_H
#define S_UNIT_H

typedef struct
{
	int full_units;
	double dB_over;
} s_unit_t;

void dBm_to_s_unit_hf(double dbm, s_unit_t *s_unit);
double s_unit_to_dBm_hf(s_unit_t *s_unit);
void dBm_to_s_unit_vhf(double dbm, s_unit_t *s_unit);
double s_unit_to_dBm_vhf(s_unit_t *s_unit);

#endif