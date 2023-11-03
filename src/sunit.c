#include "sunit.h"

const double S1_DBM = -121;
const double S9_DBM = -73;
const double S_UNIT_STEP_DBM = 6;

void dBm_to_s_unit(double dbm, s_unit_t *s_unit)
{
	if (dbm >= S9_DBM)
	{
		s_unit->full_units = 9;
		s_unit->dB_over = dbm - S9_DBM;
		return;
	}
	s_unit->full_units = 1 + (int)((dbm - S1_DBM) / S_UNIT_STEP_DBM);
	s_unit->dB_over = dbm - (S1_DBM + (s_unit->full_units - 1) * S_UNIT_STEP_DBM);
}

double s_unit_to_dBm(s_unit_t *s_unit)
{
	return S1_DBM + (s_unit->full_units - 1) * S_UNIT_STEP_DBM + s_unit->dB_over;
}
