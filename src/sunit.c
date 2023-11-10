#include "sunit.h"
#include <math.h>

const double S_UNIT_STEP_DBM = 6;

const double S1_DBM_HF = -121;
const double S9_DBM_HF = -73;

const double S1_DBM_VHF = -141;
const double S9_DBM_VHF = -93;

void dBm_to_s_unit(double dbm, s_unit_t *s_unit, double s9, double s1)
{
	if (dbm >= s9)
	{
		s_unit->full_units = 9;
		s_unit->dB_over = dbm - s9;
	}
	else if (dbm < s1)
	{
		s_unit->full_units = 1;
		s_unit->dB_over = dbm - s1;
	}
	else
	{
		s_unit->full_units = 1 + (int)floor((dbm - s1) / S_UNIT_STEP_DBM);
		s_unit->dB_over = dbm - (s1 + (s_unit->full_units - 1) * S_UNIT_STEP_DBM);
	}
}

double s_unit_to_dBm(s_unit_t *s_unit, double s1)
{
	return s1 + (s_unit->full_units - 1) * S_UNIT_STEP_DBM + s_unit->dB_over;
}

void dBm_to_s_unit_hf(double dbm, s_unit_t *s_unit)
{
	dBm_to_s_unit(dbm, s_unit, S9_DBM_HF, S1_DBM_HF);
}

double s_unit_to_dBm_hf(s_unit_t *s_unit)
{
	return s_unit_to_dBm(s_unit, S1_DBM_HF);
}

void dBm_to_s_unit_vhf(double dbm, s_unit_t *s_unit)
{
	dBm_to_s_unit(dbm, s_unit, S9_DBM_VHF, S1_DBM_VHF);
}

double s_unit_to_dBm_vhf(s_unit_t *s_unit)
{
	return s_unit_to_dBm(s_unit, S1_DBM_VHF);
}