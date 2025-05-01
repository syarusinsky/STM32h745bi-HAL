#include "LLPD.hpp"

void LLPD::fmc_sdram_init (const FMC_SDRAM_BANK& bank, const FMC_SDRAM_COL_ADDR_BITS& colBits, const FMC_SDRAM_ROW_ADDR_BITS& rowBits,
				const FMC_SDRAM_DATA_ADDR_BITS& dataBits, const FMC_SDRAM_NUM_BANKS& numBanks,
				const FMC_SDRAM_CLOCK_CONFIG& clkConfig, const FMC_SDRAM_CAS_LATENCY& casLatency, const bool burstRead,
				const FMC_SDRAM_RPIPE_DELAY& rPipeDelay, const bool writeProtection, const uint8_t tMRD, const uint8_t tXSR,
				const uint8_t tRAS, const uint8_t tRC, const uint8_t tWR, const uint8_t tRP, const uint8_t tRCD)
{
	const uint8_t bankNum = static_cast<uint8_t>( bank );

	// setup SDCR
	uint32_t sdcrVal = 0;

	sdcrVal |= static_cast<uint8_t>( colBits ) << FMC_SDCRx_NC_Pos;

	sdcrVal |= static_cast<uint8_t>( rowBits ) << FMC_SDCRx_NR_Pos;

	sdcrVal |= static_cast<uint8_t>( dataBits ) << FMC_SDCRx_MWID_Pos;

	sdcrVal |= static_cast<uint8_t>( numBanks ) << FMC_SDCRx_NB_Pos;

	sdcrVal |= static_cast<uint8_t>( casLatency ) << FMC_SDCRx_CAS_Pos;

	if ( writeProtection )
	{
		sdcrVal |= FMC_SDCRx_WP;
	}

	sdcrVal |= static_cast<uint8_t>( clkConfig ) << FMC_SDCRx_SDCLK_Pos;

	// burst read and read pipe delay must be written to SDCR1 register
	if ( bankNum == 0 && burstRead )
	{
		sdcrVal |= FMC_SDCRx_RBURST;
	}

	if ( bankNum == 0 )
	{
		sdcrVal |= static_cast<uint8_t>( rPipeDelay ) << FMC_SDCRx_RPIPE_Pos;
	}

	// program device timing
	uint32_t sdtrVal = 0;

	sdtrVal |= tMRD << FMC_SDTRx_TMRD_Pos;

	sdtrVal |= tXSR << FMC_SDTRx_TXSR_Pos;

	sdtrVal |= tRAS << FMC_SDTRx_TRAS_Pos;

	sdtrVal |= tRC << FMC_SDTRx_TRC_Pos;

	sdtrVal |= tWR << FMC_SDTRx_TWR_Pos;

	sdtrVal |= tRP << FMC_SDTRx_TRP_Pos;

	sdtrVal |= tRCD << FMC_SDTRx_TRCD_Pos;

	// apply values
	FMC_Bank5_6_R->SDCR[bankNum] = sdcrVal;
	FMC_Bank5_6_R->SDTR[bankNum] = sdtrVal;

	// enable fmc
	FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;
}

void LLPD::fmc_sdram_start (const bool startBank1, const bool startBank2, const uint8_t numAutoRefresh, const unsigned int tREFInMilliseconds,
				const unsigned int numRows, const unsigned int clkRateInMHz, const uint16_t modeRegisterVal)
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
	uint32_t modeReg = 0;
	modeReg |= bankSelection | ( 0b001 << FMC_SDCMR_MODE_Pos );
	FMC_Bank5_6_R->SDCMR = modeReg;

	// delay for at least 100us (we just delay for 150us to be safe)
	LLPD::tim6_delay( 150 );

	// send precharge all command
	modeReg = 0;
	modeReg |= bankSelection | ( 0b010 << FMC_SDCMR_MODE_Pos );
	FMC_Bank5_6_R->SDCMR = modeReg;

	// send autorefresh commands
	modeReg = 0;
	modeReg |= bankSelection | ( 0b011 << FMC_SDCMR_MODE_Pos ) | ( (numAutoRefresh - 1) << FMC_SDCMR_NRFS_Pos );
	FMC_Bank5_6_R->SDCMR = modeReg;

	// set mode register
	modeReg = 0;
	modeReg |= bankSelection | ( 0b100 << FMC_SDCMR_MODE_Pos ) | ( modeRegisterVal << FMC_SDCMR_MRD_Pos );
	FMC_Bank5_6_R->SDCMR = modeReg;

	// set refresh rate counter
	unsigned int refreshRate = ( (static_cast<float>(tREFInMilliseconds * 1000) / static_cast<float>(numRows)) * clkRateInMHz ) - 20.0f;
	FMC_Bank5_6_R->SDRTR = refreshRate << FMC_SDRTR_COUNT_Pos;
}
