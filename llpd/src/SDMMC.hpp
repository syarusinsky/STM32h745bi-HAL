#include "LLPD.hpp"

// commands
#define SDMMC_CMD_GO_IDLE_STATE 0
#define SDMMC_CMD_SEND_OP_COND 1
#define SDMMC_CMD_SEND_IF_COND 8
#define SDMMC_CMD_APP_CMD 55
#define SDMMC_ACMD_SD_SEND_OP_COND 41

// wait response values
#define SDMMC_CMD_SHORT_RESPONSE 0b01

// acmd41 queries
#define SDMMC_ACMD41_HIGH_CAPACITY 0x40000000

// r1 response masks
#define SDMMC_R1_IDLE_STATE 0b00000001

static GPIO_PORT cDPort = GPIO_PORT::A;
static GPIO_PIN cDPin = GPIO_PIN::PIN_4;

static void sendCommand (const uint32_t arg, const uint32_t cmdIndex, const uint32_t response = 0, const bool waitForInterrupt = false,
				const bool waitPend = false, const bool cpsmEnable = true, const bool cmdSuspend = false)
{
	SDMMC1->ARG = arg;

	const uint32_t cmdRegVal = (SDMMC1->CMD & ~(SDMMC_CMD_CMDINDEX | SDMMC_CMD_WAITRESP | SDMMC_CMD_WAITINT | SDMMC_CMD_WAITPEND
							| SDMMC_CMD_CPSMEN | SDMMC_CMD_CMDSUSPEND))
			| ( cmdIndex << SDMMC_CMD_CMDINDEX_Pos )
			| ( response << SDMMC_CMD_WAITRESP_Pos )
			| ( static_cast<uint32_t>(waitForInterrupt) << SDMMC_CMD_WAITINT_Pos )
			| ( static_cast<uint32_t>(waitPend) << SDMMC_CMD_WAITPEND_Pos )
			| ( static_cast<uint32_t>(cpsmEnable) << SDMMC_CMD_CPSMEN_Pos )
			| ( static_cast<uint32_t>(cmdSuspend) << SDMMC_CMD_CMDSUSPEND_Pos );

	SDMMC1->CMD = cmdRegVal;
}

static void clearFlags()
{
	SDMMC1->ICR &= ~(SDMMC_ICR_CCRCFAILC | SDMMC_ICR_CTIMEOUTC | SDMMC_ICR_CMDRENDC | SDMMC_ICR_CMDSENTC | SDMMC_ICR_BUSYD0ENDC);
}

static void waitForCmdSentAndClearFlags()
{
	while ( ! (SDMMC1->STA & SDMMC_STA_CMDSENT) ) {}

	SDMMC1->ICR &= ~(SDMMC_ICR_CCRCFAILC | SDMMC_ICR_CTIMEOUTC | SDMMC_ICR_CMDRENDC | SDMMC_ICR_CMDSENTC | SDMMC_ICR_BUSYD0ENDC);
}

static void waitForResponseAndClearFlags()
{
	while ( (SDMMC1->STA & (SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT | SDMMC_STA_BUSYD0END)) == 0
		|| (SDMMC1->STA & (SDMMC_STA_CPSMACT)) != 0 ) {}

	SDMMC1->ICR &= ~(SDMMC_ICR_CCRCFAILC | SDMMC_ICR_CTIMEOUTC | SDMMC_ICR_CMDRENDC | SDMMC_ICR_CMDSENTC | SDMMC_ICR_BUSYD0ENDC);
}

void LLPD::sdmmc_init (const GPIO_PORT& cardDetectPort, const GPIO_PIN& cardDetectPin, const unsigned int pll2rClkRate, const unsigned int targetSDMMCClkRate,
			const SDMMC_DIRPOL& dirPol, const SDMMC_BUS_WIDTH& busWidth, const bool hardwareFlowCtrl, const bool powerSave,
			const SDMMC_CLK_EDGE& clkEdge)
{
	// start sdmmc1 clock
	RCC->AHB3ENR |= RCC_AHB3ENR_SDMMC1EN;

	// start gpio clocks
	LLPD::gpio_enable_clock( GPIO_PORT::C );
	LLPD::gpio_enable_clock( GPIO_PORT::D );
	LLPD::gpio_enable_clock( cardDetectPort );

	// initialize gpio pins
	LLPD::gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_8, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH, true, 12 );
	LLPD::gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_9, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH, true, 12 );
	LLPD::gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_10, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH, true, 12 );
	LLPD::gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_11, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH, true, 12 );
	LLPD::gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_12, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH, true, 12 );
	LLPD::gpio_output_setup( GPIO_PORT::D, GPIO_PIN::PIN_2, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH, true, 12 );
	LLPD::gpio_digital_input_setup( cardDetectPort, cardDetectPin, GPIO_PUPD::PULL_UP );

	// reset sdmmc1 registers
	RCC->AHB3RSTR |= RCC_AHB3RSTR_SDMMC1RST;
	RCC->AHB3RSTR &= ~(RCC_AHB3RSTR_SDMMC1RST);

	// nvic configuration for sdmmc1 interrupts
	NVIC_SetPriority( SDMMC1_IRQn, 5 );
	NVIC_EnableIRQ( SDMMC1_IRQn );

	// set signal polarity
	SDMMC1->POWER |= SDMMC_POWER_DIRPOL;

	// ensure clk rate is <=400kHz for initialization
	const uint32_t initClkDiv = pll2rClkRate / ( 2 * 400000 );

	// set clock control register (using init values until initialization process is complete)
	uint32_t ccRegVal = 0;
	ccRegVal |= initClkDiv << SDMMC_CLKCR_CLKDIV_Pos; // edge = rising, power save is disabled, bus width is 1 bit, and hardware flow control is disabled
	SDMMC1->CLKCR = ccRegVal;

	// set power state to on
	SDMMC1->POWER |= SDMMC_POWER_PWRCTRL;

	// wait 74 sdmmc clock cycles
	const unsigned int initSDMMCClkRate = pll2rClkRate / ( 2 * initClkDiv );
	LLPD::tim6_delay( 1 + ((74 * 1000) / initSDMMCClkRate) );

	// send cmd0: go idle state command
	sendCommand( 0, SDMMC_CMD_GO_IDLE_STATE );
	waitForCmdSentAndClearFlags();

	// send cmd8: send if cond (only available on version 2 cards, so checking if this card is a version 2 card)
	bool isVersion2Card = true;
	uint32_t cmd8Arg = 0;
	cmd8Arg |= 0x1 << 8; // supply voltage (2.7-3.6 V)
	cmd8Arg |= 0xAA; // recommended check pattern
	// cmd8Arg should be 426, cmdIndex should be 8, response should be 256, cpsm should be 4096
	// ARG ends up being 426
	// tmpreg ends up being 4360
	// CMD ends up being 264
	// RESPCMD ends up being 8
	// RESP1 ends up being 426
	// STA ends up being 64
	sendCommand( cmd8Arg, SDMMC_CMD_SEND_IF_COND, SDMMC_CMD_SHORT_RESPONSE );

	// wait for response or failure and then clear flags
	while ( (SDMMC1->STA & (SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT)) == 0
			|| (SDMMC1->STA & (SDMMC_STA_CPSMACT)) != 0 ) {}
	if ( SDMMC1->STA & SDMMC_STA_CTIMEOUT )
	{
		// no response, so card is not version 2
		isVersion2Card = false;
	}
	if ( SDMMC1->STA & SDMMC_STA_CCRCFAIL )
	{
		// ccrc failure, so card is not version 2
		isVersion2Card = false;
	}
	clearFlags();

	bool isHighCapacityCard = false;

	// if we don't have a version 2 card, send cmd0: go idle state command again
	// TODO version 1 card support may need work
	if ( ! isVersion2Card )
	{
		sendCommand( 0, SDMMC_CMD_GO_IDLE_STATE );
		waitForCmdSentAndClearFlags();

		// send cmd1: bring out of idle state
		sendCommand( 0, SDMMC_CMD_SEND_OP_COND );

		// wait for response or failure and then clear flags
		while ( (SDMMC1->STA & (SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT | SDMMC_STA_BUSYD0END)) == 0
				|| (SDMMC1->STA & (SDMMC_STA_CPSMACT)) != 0 ) {}
		if ( SDMMC1->STA & SDMMC_STA_CTIMEOUT )
		{
			while ( true ) {}
		}
		if ( SDMMC1->STA & SDMMC_STA_CCRCFAIL )
		{
			while ( true ) {}
		}
		clearFlags();
	}
	else // card is version 2 card
	{
		uint32_t response = 0;
		while ( response & SDMMC_R1_IDLE_STATE )
		{
			// send cmd55: app command with rca as 0
			sendCommand( 0, SDMMC_CMD_APP_CMD, SDMMC_CMD_SHORT_RESPONSE );
			waitForResponseAndClearFlags();

			// check to ensure command response matches
			if ( SDMMC1->RESPCMD != SDMMC_CMD_APP_CMD )
			{
				// this card is unsupported
				while ( true ) {}
			}

			// send acmd41: exit idle state and check if high capacity card
			sendCommand( SDMMC_ACMD41_HIGH_CAPACITY, SDMMC_ACMD_SD_SEND_OP_COND, SDMMC_CMD_SHORT_RESPONSE );
			waitForResponseAndClearFlags();

			response = SDMMC1->RESP1;
		}

		// TODO need to work on this
		if ( (response & SDMMC_ACMD41_HIGH_CAPACITY) == SDMMC_ACMD41_HIGH_CAPACITY )
		{
			isHighCapacityCard = true;
		}
	}

	// target sdmmc clk div will be ceil( pllclk / (targetSDMMCClkRate probably times 2???) )
}
