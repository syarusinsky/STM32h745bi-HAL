#include "LLPD.hpp"

void LLPD::fmc_sdram_init (const FMC_SDRAM_BANK& bank, const FMC_SDRAM_COL_ADDR_BITS& colBits, const FMC_SDRAM_ROW_ADDR_BITS& rowBits,
				const FMC_SDRAM_DATA_ADDR_BITS& dataBits, const FMC_SDRAM_NUM_BANKS& numBanks, const FMC_SDRAM_CLOCK_CONFIG& clkConfig,
				const FMC_SDRAM_CAS_LATENCY& casLatency, const bool burstRead, const FMC_SDRAM_RPIPE_DELAY& rPipeDelay, const bool writeProtection,
				const uint8_t tMRD, const uint8_t tXSR, const uint8_t tRAS, const uint8_t tRC, const uint8_t tWR, const uint8_t tRP,
				const uint8_t tRCD)
{
	const uint8_t bankNum = static_cast<uint8_t>( bank );

	// setup SDCR
	FMC_Bank5_6_R->SDCR[bankNum] &= FMC_SDCRx_NC;
	FMC_Bank5_6_R->SDCR[bankNum] |= static_cast<uint8_t>( colBits ) << FMC_SDCRx_NC_Pos;

	FMC_Bank5_6_R->SDCR[bankNum] &= FMC_SDCRx_NR;
	FMC_Bank5_6_R->SDCR[bankNum] |= static_cast<uint8_t>( rowBits ) << FMC_SDCRx_NR_Pos;

	FMC_Bank5_6_R->SDCR[bankNum] &= FMC_SDCRx_MWID;
	FMC_Bank5_6_R->SDCR[bankNum] |= static_cast<uint8_t>( dataBits ) << FMC_SDCRx_MWID_Pos;

	FMC_Bank5_6_R->SDCR[bankNum] &= FMC_SDCRx_NB;
	FMC_Bank5_6_R->SDCR[bankNum] |= static_cast<uint8_t>( numBanks ) << FMC_SDCRx_NB_Pos;

	FMC_Bank5_6_R->SDCR[bankNum] &= FMC_SDCRx_CAS;
	FMC_Bank5_6_R->SDCR[bankNum] |= static_cast<uint8_t>( casLatency ) << FMC_SDCRx_CAS_Pos;

	if ( writeProtection )
	{
		FMC_Bank5_6_R->SDCR[bankNum] |= FMC_SDCRx_WP;
	}
	else
	{
		FMC_Bank5_6_R->SDCR[bankNum] &= ~(FMC_SDCRx_WP);
	}

	FMC_Bank5_6_R->SDCR[bankNum] &= FMC_SDCRx_SDCLK;
	FMC_Bank5_6_R->SDCR[bankNum] |= static_cast<uint8_t>( clkConfig ) << FMC_SDCRx_SDCLK_Pos;

	// burst read and read pipe delay must be written to SDCR1 register
	if ( burstRead )
	{
		FMC_Bank5_6_R->SDCR[0] |= FMC_SDCRx_RBURST;
	}
	else
	{
		FMC_Bank5_6_R->SDCR[0] &= ~(FMC_SDCRx_RBURST);
	}

	FMC_Bank5_6_R->SDCR[0] &= FMC_SDCRx_RPIPE;
	FMC_Bank5_6_R->SDCR[0] |= static_cast<uint8_t>( rPipeDelay ) << FMC_SDCRx_RPIPE_Pos;

	// program device timing
	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TMRD;
	FMC_Bank5_6_R->SDTR[bankNum] |= tMRD << FMC_SDTRx_TMRD_Pos;

	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TXSR;
	FMC_Bank5_6_R->SDTR[bankNum] |= tXSR << FMC_SDTRx_TXSR_Pos;

	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TRAS;
	FMC_Bank5_6_R->SDTR[bankNum] |= tRAS << FMC_SDTRx_TRAS_Pos;

	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TRC;
	FMC_Bank5_6_R->SDTR[bankNum] |= tRC << FMC_SDTRx_TRC_Pos;

	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TWR;
	FMC_Bank5_6_R->SDTR[bankNum] |= tWR << FMC_SDTRx_TWR_Pos;

	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TRP;
	FMC_Bank5_6_R->SDTR[bankNum] |= tRP << FMC_SDTRx_TRP_Pos;

	FMC_Bank5_6_R->SDTR[bankNum] &= FMC_SDTRx_TRCD;
	FMC_Bank5_6_R->SDTR[bankNum] |= tRCD << FMC_SDTRx_TRCD_Pos;
}

void LLPD::fmc_sdram_start (const bool startBank1, const bool startBank2, const uint8_t numAutoRefresh, const unsigned int tREFInMilliseconds, const unsigned int numRows,
				const unsigned int clkRateInMHz, const uint16_t modeRegisterVal)
{
	uint32_t bankSelection = 0;
	if ( startBank1 )
	{
		bankSelection |= FMC_SDCMR_CTB1;
	}
	if ( startBank2 )
	{
		bankSelection |= FMC_SDCMR_CTB2;
	}

	// enable clock
	uint32_t modeReg = FMC_Bank5_6_R->SDCMR;
	modeReg &= ~(FMC_SDCMR_MODE) & ~(FMC_SDCMR_CTB1) & ~(FMC_SDCMR_CTB2);
	modeReg |= bankSelection | FMC_SDCMR_MODE_0;
	FMC_Bank5_6_R->SDCMR = modeReg;

	// delay for at least 100us (we just delay for 1ms to be safe)
	LLPD::tim6_delay( 1000 );

	// send precharge all command
	modeReg = FMC_Bank5_6_R->SDCMR;
	modeReg &= ~(FMC_SDCMR_MODE) & ~(FMC_SDCMR_CTB1) & ~(FMC_SDCMR_CTB2);
	modeReg |= bankSelection | FMC_SDCMR_MODE_1;
	FMC_Bank5_6_R->SDCMR = modeReg;

	// send autorefresh commands
	modeReg = FMC_Bank5_6_R->SDCMR;
	modeReg &= ~(FMC_SDCMR_MODE) & ~(FMC_SDCMR_CTB1) & ~(FMC_SDCMR_CTB2) & ~(FMC_SDCMR_NRFS);
	modeReg |= bankSelection | FMC_SDCMR_MODE_1 |FMC_SDCMR_MODE_0 | ( (numAutoRefresh - 1) << FMC_SDCMR_NRFS_Pos );
	FMC_Bank5_6_R->SDCMR = modeReg;

	// set mode register
	modeReg = FMC_Bank5_6_R->SDCMR;
	modeReg &= ~(FMC_SDCMR_MODE) & ~(FMC_SDCMR_CTB1) & ~(FMC_SDCMR_CTB2) & ~(FMC_SDCMR_MRD);
	modeReg |= bankSelection | FMC_SDCMR_MODE_2 | ( modeRegisterVal << FMC_SDCMR_MRD_Pos );
	FMC_Bank5_6_R->SDCMR = modeReg;

	// set refresh rate counter
	unsigned int refreshRate = ( (static_cast<float>(tREFInMilliseconds * 1000) / static_cast<float>(numRows)) * clkRateInMHz ) - 20.0f;
	FMC_Bank5_6_R->SDRTR &= FMC_SDRTR_COUNT;
	FMC_Bank5_6_R->SDRTR |= refreshRate << FMC_SDRTR_COUNT_Pos;
}
