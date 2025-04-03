#include "LLPD.hpp"

bool LLPD::hsem_try_take (unsigned int semNum)
{
	if ( HSEM->RLR[semNum] != (HSEM_CR_COREID_CURRENT | HSEM_RLR_LOCK) )
	{
		return false;
	}

	return true;
}

void LLPD::hsem_release (unsigned int semNum)
{
	HSEM->R[semNum] = HSEM_CR_COREID_CURRENT;
}
