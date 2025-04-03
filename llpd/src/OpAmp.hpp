#include "LLPD.hpp"

void LLPD::opamp_init (const OPAMP_NUM& opAmpNum)
{
	// enable clock to op amp
	RCC->APB1HENR |= RCC_APB1HENR_OPAMPEN;

	OPAMP_TypeDef* opAmp = nullptr;
	if ( opAmpNum == OPAMP_NUM::OPAMP_1 )
	{
		opAmp = OPAMP1;
	}
	else // OPAMP_NUM::OPAMP_2
	{
		opAmp = OPAMP2;
	}

	// disable op amp
	opAmp->CSR &= ~(OPAMP_CSR_OPAMPxEN);

	// select inputs
	opAmp->CSR &= ~(OPAMP_CSR_VMSEL); // INM0 connected to OPAMP INM input
	opAmp->CSR &= ~(OPAMP_CSR_VPSEL); // GPIO connected to OPAMP INP input

	// enable op amp
	opAmp->CSR |= OPAMP_CSR_OPAMPxEN;
}
