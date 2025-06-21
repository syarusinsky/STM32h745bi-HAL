#include "LLPD.hpp"

static void ensureCorrectFlashLatency (unsigned int freq)
{
	uint32_t intendedLatency = 0;
	uint32_t intendedSgDelay = 0;
	if ( freq <= 70000000 )
	{
		intendedLatency = FLASH_ACR_LATENCY_0WS;
		intendedSgDelay = 0;
	}
	else if ( freq <= 140000000 )
	{
		intendedLatency = FLASH_ACR_LATENCY_1WS;
		intendedSgDelay = FLASH_ACR_WRHIGHFREQ_0;
	}
	else if ( freq <= 185000000 )
	{
		intendedLatency = FLASH_ACR_LATENCY_2WS;
		intendedSgDelay = FLASH_ACR_WRHIGHFREQ_0;
	}
	else if ( freq <= 210000000 )
	{
		intendedLatency = FLASH_ACR_LATENCY_2WS;
		intendedSgDelay = FLASH_ACR_WRHIGHFREQ_1;
	}
	else if ( freq <= 225000000 )
	{
		intendedLatency = FLASH_ACR_LATENCY_3WS;
		intendedSgDelay = FLASH_ACR_WRHIGHFREQ_1;
	}
	else if ( freq <= 240000000 )
	{
		intendedLatency = FLASH_ACR_LATENCY_4WS;
		intendedSgDelay = FLASH_ACR_WRHIGHFREQ_1;
	}
	else
	{
		intendedLatency = FLASH_ACR_LATENCY_15WS;
		intendedSgDelay = FLASH_ACR_WRHIGHFREQ_1;
	}

	FLASH->ACR &= ~(FLASH_ACR_LATENCY);
	FLASH->ACR |= intendedLatency;
	FLASH->ACR &= ~(FLASH_ACR_WRHIGHFREQ);
	FLASH->ACR |= intendedSgDelay;
}

void LLPD::rcc_clock_start_max_cpu1 (const unsigned int pll1qPresc)
{
// initial stuff
	// wait for d2ckrdy == reset in rcc to wait for cpu 2 to enter stop mode
	while ( RCC->CR & RCC_CR_D2CKRDY ) {}

	// enable syscfg clock
	RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;

// setup power stuff
	// enable LDO
	uint32_t supplyConfig = PWR->CR3;
	supplyConfig |= PWR_CR3_LDOEN;
	supplyConfig &= ~(PWR_CR3_SMPSEN);
	PWR->CR3 = supplyConfig;

	// wait for voltage level to stabilize
	while ( ! (PWR->CSR1 & PWR_CSR1_ACTVOSRDY) ) {}

	// set voltage scaling to scale 1 first
	PWR->D3CR |= PWR_D3CR_VOS;

	// set LDO overdrive to enabled
	SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN;

	// wait until ready
	while ( ! (PWR->D3CR & PWR_D3CR_VOSRDY) ) {}
// clock stuff
	// enable hse
	RCC->CR |= RCC_CR_HSEON;

	// wait until hse ready
	while ( ! (RCC->CR & RCC_CR_HSERDY) ) {}

	// disable pll
	RCC->CR &= ~(RCC_CR_PLL1ON);

	// wait until pll is disabled
	while ( RCC->CR & RCC_CR_PLL1RDY ) {}

	// set prescaler for pll1
	RCC->PLLCKSELR &= ~(RCC_PLLCKSELR_DIVM1);
	RCC->PLLCKSELR |= RCC_PLLCKSELR_DIVM1_0;

	// select source for pll clock
	RCC->PLLCKSELR &= ~(RCC_PLLCKSELR_PLLSRC);
	RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE;

	// enable pll divp1 output
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;

	// enable pll divq1 output
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;

	// set pll input frequency range
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1RGE;

	// set pll output frequency range
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL1VCOSEL);

	// set pll 1 divp prescaler value
	RCC->PLL1DIVR &= ~(RCC_PLL1DIVR_P1);
	RCC->PLL1DIVR |= (1 << RCC_PLL1DIVR_P1_Pos);

	// set pll 1 divq prescaler value
	RCC->PLL1DIVR &= ~(RCC_PLL1DIVR_Q1);
	RCC->PLL1DIVR |= ( (pll1qPresc - 1) << RCC_PLL1DIVR_Q1_Pos );

	// set pll multiply
	RCC->PLL1DIVR &= ~(RCC_PLL1DIVR_N1);
	RCC->PLL1DIVR |= ( (60 - 1) << RCC_PLL1DIVR_N1_Pos );

	// disable pll fract
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL1FRACEN);

	// enable main pll
	RCC->CR |= RCC_CR_PLL1ON;

	// wait until pll is ready
	while ( ! (RCC_CR_PLL1RDY) ) {}

	// set clock dividers
	RCC->D1CFGR &= ~(RCC_D1CFGR_HPRE);
	RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV2;
	RCC->D1CFGR &= ~(RCC_D1CFGR_D1PPRE);
	RCC->D1CFGR |= RCC_D1CFGR_D1PPRE_DIV2;
	RCC->D2CFGR &= ~(RCC_D2CFGR_D2PPRE1);
	RCC->D2CFGR |= RCC_D2CFGR_D2PPRE1_DIV2;
	RCC->D2CFGR &= ~(RCC_D2CFGR_D2PPRE2);
	RCC->D2CFGR |= RCC_D2CFGR_D2PPRE2_DIV2;
	RCC->D3CFGR &= ~(RCC_D3CFGR_D3PPRE);
	RCC->D3CFGR |= RCC_D3CFGR_D3PPRE_DIV2;

	// increase flash latency if necessary
	ensureCorrectFlashLatency( 240000000 );

	// set system clock as pll
	RCC->CFGR &= ~(RCC_CFGR_SW);
	RCC->CFGR |= RCC_CFGR_SW_PLL1;

	// decrease flash latency if necessary // TODO just leaving this as a reminder to do so if I write more generic code
	ensureCorrectFlashLatency( 240000000 );

	// enable clock security system
	RCC->CR |= RCC_CR_CSSHSEON;

// core synchronization stuff
	// enable hardware semaphore clock
	RCC->AHB4ENR |= RCC_AHB4ENR_HSEMEN;

	// take the semaphore (spinlock to catch if not taken)
	while ( HSEM->RLR[0] != (HSEM_CR_COREID_CURRENT | HSEM_RLR_LOCK) ) {}

	// set the setupCompleteFlag to false
	bool* volatile setupCompleteFlag = reinterpret_cast<bool*>( D2_AHBSRAM_BASE );
	*setupCompleteFlag = false;

	// release the semaphore to notifiy to CPU2 that everything is cool
	HSEM->R[0] = ( 0 /*process id*/ | HSEM_CR_COREID_CURRENT );

	// wait for d2ckrdy flag to == set in rcc to wait for cpu to enter run mode
	while ( ! (RCC->CR & RCC_CR_D2CKRDY) ) {}
}

void LLPD::rcc_clock_start_max_cpu2()
{
// core syncronization stuff
	// enable hardware semaphore clock
	RCC->AHB4ENR |= RCC_AHB4ENR_HSEMEN;

	// activate hardware semaphore notification for cortex m4
	if ( HSEM_CR_COREID_CURRENT == HSEM_CR_COREID_CPU1 )
	{
		HSEM->C1IER |= 0b1;
	}
	else if ( HSEM_CR_COREID_CURRENT == HSEM_CR_COREID_CPU2 )
	{
		HSEM->C2IER |= 0b1;
	}

	// clear pending event (must be done before cpu goes into csleep or cstop?)
	__SEV();
	__WFE();

	// set voltage regulator to main mode when SVOS3 is selected for stop mode
	PWR->CR1 &= ~(PWR_CR1_LPDS);

	// keep DSTOP mode when d2 goes into deep sleep
	PWR->CPU2CR &= ~(PWR_CPU2CR_PDDS_D2);

	// set deep sleep bit in the system control register
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	// ensure all instructions are done before entering deep sleep
	__DSB();
	__ISB();

	// wait for event
	__WFE();

	// clear deep sleep bit in the system control register
	SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);

	// clear the hardware semaphore flag
	HSEM->C2ICR |= 0b1;
}

void LLPD::rcc_start_pll2 (const unsigned int pllMultiply)
{
	// disable pll
	RCC->CR &= ~(RCC_CR_PLL2ON);

	// wait until pll is disabled
	while ( RCC->CR & RCC_CR_PLL2RDY ) {}

	// set prescaler for pll2 (16 MHz / 16 = 1 MHz)
	RCC->PLLCKSELR &= ~(RCC_PLLCKSELR_DIVM2);
	RCC->PLLCKSELR |= 16 << RCC_PLLCKSELR_DIVM2_Pos;

	// set pll input frequency range
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL2RGE);

	// enable pll divr2 output
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR2EN;

	// set pll output frequency range
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL2VCOSEL;

	// set pll fract
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL2FRACEN);

	// set pll2 divr prescaler value
	RCC->PLL2DIVR &= ~(RCC_PLL2DIVR_R2);
	RCC->PLL2DIVR |= 0 << RCC_PLL2DIVR_R2_Pos;

	// // set pll multiply
	RCC->PLL2DIVR &= ~(RCC_PLL2DIVR_N2);
	RCC->PLL2DIVR |= ( (pllMultiply - 1) << RCC_PLL2DIVR_N2_Pos );

	// enable pll
	RCC->CR |= RCC_CR_PLL2ON;

	// wait until pll is ready
	while ( ! (RCC_CR_PLL2RDY) ) {}

	// TODO maybe move this somewhere else
	// ensure fmc use pll2 divr
	RCC->D1CCIPR &= ~(RCC_D1CCIPR_FMCSEL);
	RCC->D1CCIPR |= RCC_D1CCIPR_FMCSEL_1;
	// ensure sdmmc1 use pll2 divr
	RCC->D1CCIPR &= ~(RCC_D1CCIPR_SDMMCSEL);
	RCC->D1CCIPR |= RCC_D1CCIPR_SDMMCSEL;
	// enable fmc peripheral clock
	RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
}

void LLPD::rcc_start_pll3 (const unsigned int pllMultiply)
{
	// disable pll
	RCC->CR &= ~(RCC_CR_PLL3ON);

	// wait until pll is disabled
	while ( RCC->CR & RCC_CR_PLL3RDY ) {}

	// set prescaler for pll3 (16 MHz / 16 = 1 MHz)
	RCC->PLLCKSELR &= ~(RCC_PLLCKSELR_DIVM3);
	RCC->PLLCKSELR |= 16 << RCC_PLLCKSELR_DIVM3_Pos;

	// set pll input frequency range
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL3RGE);

	// enable pll divr3 output
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR3EN;

	// set pll output frequency range
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL3VCOSEL;

	// set pll fract
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLL3FRACEN);

	// set pll3 divr prescaler value
	RCC->PLL3DIVR &= ~(RCC_PLL3DIVR_R3);
	RCC->PLL3DIVR |= (5 - 1) << RCC_PLL3DIVR_R3_Pos;

	// // set pll multiply
	RCC->PLL3DIVR &= ~(RCC_PLL3DIVR_N3);
	RCC->PLL3DIVR |= ( (pllMultiply - 1) << RCC_PLL3DIVR_N3_Pos );

	// enable pll
	RCC->CR |= RCC_CR_PLL3ON;

	// wait until pll is ready
	while ( ! (RCC_CR_PLL3RDY) ) {}

	// TODO maybe move this somewhere else
	// enable ltdc peripheral clock
	RCC->APB3ENR |= RCC_APB3ENR_LTDCEN;
}
