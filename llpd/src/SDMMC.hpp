#include "LLPD.hpp"

#include <cmath>

// commands
static constexpr uint32_t SDMMC_CMD_GO_IDLE_STATE = 0;
static constexpr uint32_t SDMMC_CMD_SEND_OP_COND = 1;
static constexpr uint32_t SDMMC_CMD_ALL_SEND_CID = 2;
static constexpr uint32_t SDMMC_CMD_SEND_RELATIVE_ADDR = 3;
static constexpr uint32_t SDMMC_ACMD_SET_BUS_WIDTH = 6;
static constexpr uint32_t SDMMC_CMD_SELECT_DESELECT_CARD = 7;
static constexpr uint32_t SDMMC_CMD_SEND_IF_COND = 8;
static constexpr uint32_t SDMMC_CMD_SEND_CSD = 9;
static constexpr uint32_t SDMMC_CMD_SEND_STATUS = 13;
static constexpr uint32_t SDMMC_CMD_SET_BLOCKLEN = 16;
static constexpr uint32_t SDMMC_ACMD_SD_SEND_OP_COND = 41;
static constexpr uint32_t SDMMC_ACMD_SEND_SCR = 51;
static constexpr uint32_t SDMMC_CMD_APP_CMD = 55;

// wait response values
static constexpr uint32_t SDMMC_CMD_NO_RESPONSE = 0b00;
static constexpr uint32_t SDMMC_CMD_SHORT_RESPONSE = 0b01;
static constexpr uint32_t SDMMC_CMD_LONG_RESPONSE = 0b11;

// acmd41 queries
static constexpr uint32_t SDMMC_ACMD41_BUSY = 0x80000000;
static constexpr uint32_t SDMMC_ACMD41_VOLTAGE_WINDOW = 0x100000; // 3.2V - 3.3V
static constexpr uint32_t SDMMC_ACMD41_HIGH_CAPACITY = 0x40000000;

// r1 response masks
static constexpr uint32_t SDMMC_R1_IDLE_STATE = 0b00000001;

// block sizes
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_1BYTES = 0b0000;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_2BYTES = 0b0001;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_4BYTES = 0b0010;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_8BYTES = 0b0011;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_16BYTES = 0b0100;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_32BYTES = 0b0101;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_64BYTES = 0b0110;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_128BYTES = 0b0111;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_256BYTES = 0b1000;
static constexpr uint32_t SDMMC_DATABLOCK_SIZE_512BYTES = 0b1001;

// scr values
static constexpr uint32_t SDMMC_4BIT_BUS_SUPPORT = 0x00040000;

// data timeout (dtimer reg val)
static constexpr uint32_t SDMMC_TIMEOUT = 0xFFFFFFFF;

// card detection port and pin
static constexpr GPIO_PORT cDPort = GPIO_PORT::A;
static constexpr GPIO_PIN cDPin = GPIO_PIN::PIN_4;

// user defined timeout
static constexpr uint32_t TIMEOUT_MAX = 100000;

static uint32_t cid[4] = { 0, 0, 0, 0 };
static uint32_t rca = 0; // the real rca is only the last 16 bits, but since it's used in the argument for some commands, leave as-is
static uint32_t csd[4] = { 0, 0, 0, 0 };
static uint32_t scr[2] = { 0, 0 };
static bool isVersion2Card = true;

static void sendCommand (const uint32_t arg, const uint32_t cmdIndex, const uint32_t response = SDMMC_CMD_NO_RESPONSE,
				const bool waitForInterrupt = false, const bool waitPend = false, const bool cpsmEnable = true,
				const bool cmdSuspend = false)
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

static void clearCmdFlags()
{
	SDMMC1->ICR |= SDMMC_ICR_CCRCFAILC | SDMMC_ICR_CTIMEOUTC | SDMMC_ICR_CMDRENDC | SDMMC_ICR_CMDSENTC | SDMMC_ICR_BUSYD0ENDC;
}

static bool waitForCmdSentAndClearFlags()
{
	uint32_t timeout = 0;
	while ( (! (SDMMC1->STA & SDMMC_STA_CMDSENT)) && timeout < TIMEOUT_MAX )
	{
		timeout++;
	}

	const uint32_t tempSTA = SDMMC1->STA;
	clearCmdFlags();

	if ( (tempSTA & SDMMC_ICR_CTIMEOUTC) != 0 )
	{
		return false;
	}
	else if ( timeout >= TIMEOUT_MAX )
	{
		return false;
	}

	return true;
}

static bool waitForResponseAndClearFlags()
{
	uint32_t timeout = 0;
	while ( ((SDMMC1->STA & (SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT | SDMMC_STA_BUSYD0END)) == 0
		|| (SDMMC1->STA & (SDMMC_STA_CPSMACT)) != 0) && timeout < TIMEOUT_MAX )
	{
		timeout++;
	}

	const uint32_t tempSTA = SDMMC1->STA;
	clearCmdFlags();

	if ( (tempSTA & SDMMC_ICR_CTIMEOUTC) != 0 )
	{
		return false;
	}
	else if ( timeout >= TIMEOUT_MAX )
	{
		return false;
	}

	return true;
}

static bool waitForSCRDataResponseAndClearFlags (uint32_t* dataPtr, unsigned int dataSize)
{
	uint32_t timeout = 0;
	uint32_t numDataReceived = 0;
	uint32_t tempSTA = SDMMC1->STA;
	while ( ((tempSTA & (SDMMC_STA_RXOVERR | SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_DBCKEND | SDMMC_STA_DATAEND) == 0)
			&& timeout < TIMEOUT_MAX)
			|| (numDataReceived < dataSize) )
	{
		if ( ((tempSTA & SDMMC_STA_RXFIFOE) == 0) && numDataReceived < dataSize )
		{
			*dataPtr = SDMMC1->FIFO;
			numDataReceived++;
			dataPtr++;
			*dataPtr = SDMMC1->FIFO;
			numDataReceived++;
		}

		timeout++;
		tempSTA = SDMMC1->STA;
	}

	tempSTA = SDMMC1->STA;
	SDMMC1->ICR |= SDMMC_ICR_RXOVERRC | SDMMC_ICR_DCRCFAILC | SDMMC_ICR_DTIMEOUTC | SDMMC_ICR_DBCKENDC | SDMMC_ICR_DATAENDC;

	if ( (tempSTA & (SDMMC_ICR_RXOVERRC | SDMMC_ICR_DCRCFAILC | SDMMC_ICR_DTIMEOUTC)) != 0 )
	{
		return false;
	}
	else if ( timeout >= TIMEOUT_MAX )
	{
		return false;
	}

	return true;
}

bool LLPD::sdmmc_init (const GPIO_PORT& cardDetectPort, const GPIO_PIN& cardDetectPin, const unsigned int pll2rClkRate, const unsigned int targetSDMMCClkRate,
			const SDMMC_BUS_WIDTH& busWidth, const bool hardwareFlowCtrl, const bool powerSave, const SDMMC_CLK_EDGE& clkEdge)
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
	if ( ! waitForCmdSentAndClearFlags() )
	{
		return false;
	}

	// send cmd8: send if cond (only responds if version 2 card, so checking if this card is a version 2 card)
	unsigned int version2Trials = 0;
	constexpr uint8_t MAX_VERSION2_TRIALS = 10;
	for ( version2Trials = 0; version2Trials < MAX_VERSION2_TRIALS; version2Trials++ )
	{
		uint32_t cmd8Arg = 0;
		cmd8Arg |= 0x1 << 8; // supply voltage (2.7-3.6 V)
		cmd8Arg |= 0xAA; // recommended check pattern

		sendCommand( cmd8Arg, SDMMC_CMD_SEND_IF_COND, SDMMC_CMD_SHORT_RESPONSE );

		// wait for response or failure and then clear flags
		uint32_t timeout = 0;
		while ( (SDMMC1->STA & (SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT)) == 0
				|| (SDMMC1->STA & (SDMMC_STA_CPSMACT)) != 0 || timeout >= TIMEOUT_MAX )
		{
			timeout++;
		}

		bool shouldBreak = timeout < TIMEOUT_MAX;

		if ( SDMMC1->STA & SDMMC_STA_CTIMEOUT )
		{
			// no response, so card is not version 2
			isVersion2Card = false;
			shouldBreak = true;
		}
		if ( SDMMC1->STA & SDMMC_STA_CCRCFAIL )
		{
			// ccrc failure, so card is not version 2
			isVersion2Card = false;
			shouldBreak = true;
		}
		clearCmdFlags();

		if ( shouldBreak )
		{
			break;
		}
	}

	// if we didn't get a response at all, we probably don't have an sd card connected so we fail
	if ( version2Trials >= MAX_VERSION2_TRIALS )
	{
		return false;
	}

	uint32_t response = 0;
	uint32_t timeout = 0;
	while ( ((response & SDMMC_ACMD41_BUSY) == 0) && (timeout < TIMEOUT_MAX) )
	{
		// send cmd55: app command with rca as 0
		sendCommand( 0, SDMMC_CMD_APP_CMD, SDMMC_CMD_SHORT_RESPONSE );
		if ( ! waitForResponseAndClearFlags() )
		{
			return false;
		}

		// check to ensure command response matches
		if ( SDMMC1->RESPCMD != SDMMC_CMD_APP_CMD )
		{
			// this card is unsupported
			return false;
		}

		// send acmd41: exit idle state and check if high capacity card
		sendCommand( SDMMC_ACMD41_BUSY | SDMMC_ACMD41_HIGH_CAPACITY | SDMMC_ACMD41_VOLTAGE_WINDOW,
				SDMMC_ACMD_SD_SEND_OP_COND, SDMMC_CMD_SHORT_RESPONSE );
		if ( ! waitForResponseAndClearFlags() )
		{
			return false;
		}

		response = SDMMC1->RESP1;

		timeout++;
	}

	// unable to complete voltage trials
	if ( timeout >= TIMEOUT_MAX )
	{
		return false;
	}

	if ( response & SDMMC_ACMD41_HIGH_CAPACITY )
	{
		// not currently supporting high capacity cards
		return false;
	}

	// send cmd2: request card to send cid
	sendCommand( 0, SDMMC_CMD_ALL_SEND_CID, SDMMC_CMD_LONG_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// get cid
	cid[0] = SDMMC1->RESP1;
	cid[1] = SDMMC1->RESP2;
	cid[2] = SDMMC1->RESP3;
	cid[3] = SDMMC1->RESP4;

	// send cmd3: request card to send rca
	sendCommand( 0, SDMMC_CMD_SEND_RELATIVE_ADDR, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// get rca
	rca = SDMMC1->RESP1;

	// send cmd9: request card to send csd
	sendCommand( rca, SDMMC_CMD_SEND_CSD, SDMMC_CMD_LONG_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// get csd
	csd[0] = SDMMC1->RESP1;
	csd[1] = SDMMC1->RESP2;
	csd[2] = SDMMC1->RESP3;
	csd[3] = SDMMC1->RESP4;

	// TODO we could get all sorts of useful information here with the csd, but for now we don't really care

	// send cmd7: select the card
	sendCommand( rca, SDMMC_CMD_SELECT_DESELECT_CARD, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// TODO here we should get the card status with acmd13 to figure out speed grade and stuff, but
	// since we already know the type of card fuck it

	// send cmd16: set block length (in preparation to get scr)
	constexpr uint32_t scrBlockLen = 8;
	sendCommand( scrBlockLen, SDMMC_CMD_SET_BLOCKLEN, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}
	// send cmd55: app command with rca (in preparation to get scr)
	sendCommand( rca, SDMMC_CMD_APP_CMD, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// set the data timeout value
	SDMMC1->DTIMER = SDMMC_TIMEOUT;

	// set the data length
	SDMMC1->DLEN = scrBlockLen;

	// set the dctrl values
	uint32_t tmpRegVal = SDMMC1->DCTRL;
	tmpRegVal &= ~(SDMMC_DCTRL_DTEN | SDMMC_DCTRL_DTDIR | SDMMC_DCTRL_DTMODE | SDMMC_DCTRL_DBLOCKSIZE);
	tmpRegVal |= SDMMC_DATABLOCK_SIZE_8BYTES << SDMMC_DCTRL_DBLOCKSIZE_Pos;
	tmpRegVal |= SDMMC_DCTRL_DTDIR; // direction to host
	tmpRegVal |= SDMMC_DCTRL_DTEN;
	SDMMC1->DCTRL = tmpRegVal;

	// send acmd51: request card to send scr
	sendCommand( 0, SDMMC_ACMD_SEND_SCR, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// get scr data
	if ( ! waitForSCRDataResponseAndClearFlags(scr, 2) )
	{
		return false;
	}

	uint32_t tempSCR0 = scr[0];
	uint32_t tempSCR1 = scr[1];

	// we need to change the byte ordering of the scr values
	scr[0] = ((tempSCR1 & 0xFF) << 24) | ((tempSCR1 & 0xFF00) << 8) | ((tempSCR1 & 0xFF0000) >> 8) | ((tempSCR1 & 0xFF000000) >> 24);
	scr[1] = ((tempSCR0 & 0xFF) << 24) | ((tempSCR0 & 0xFF00) << 8) | ((tempSCR0 & 0xFF0000) >> 8) | ((tempSCR0 & 0xFF000000) >> 24);

	// ensure we support 4-bit bus
	if ( (scr[1] & SDMMC_4BIT_BUS_SUPPORT) == 0 )
	{
		return false;
	}

	// send cmd55: app command with rca (in preparation to set bus width)
	sendCommand( rca, SDMMC_CMD_APP_CMD, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// send acmd6: set bus width
	sendCommand( 2, SDMMC_ACMD_SET_BUS_WIDTH, SDMMC_CMD_SHORT_RESPONSE ); // argument 2 sets to 4-bit wide bus
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// configure sdmmc peripheral to desired parameters
	uint32_t targetSDMMCClkDiv = std::ceil( static_cast<float>(pll2rClkRate) / (2.0f * static_cast<float>(targetSDMMCClkRate)) );
	tmpRegVal = SDMMC1->CLKCR;
	tmpRegVal &= ~(SDMMC_CLKCR_CLKDIV | SDMMC_CLKCR_PWRSAV | SDMMC_CLKCR_WIDBUS | SDMMC_CLKCR_NEGEDGE | SDMMC_CLKCR_HWFC_EN
			| SDMMC_CLKCR_DDR | SDMMC_CLKCR_BUSSPEED | SDMMC_CLKCR_SELCLKRX);
	tmpRegVal |= static_cast<uint32_t>(clkEdge) << SDMMC_CLKCR_NEGEDGE_Pos;
	tmpRegVal |= ((powerSave) ? 1 : 0) << SDMMC_CLKCR_PWRSAV_Pos;
	tmpRegVal |= static_cast<uint32_t>(busWidth) << SDMMC_CLKCR_WIDBUS_Pos;
	tmpRegVal |= ((hardwareFlowCtrl) ? 1 : 0) << SDMMC_CLKCR_HWFC_EN_Pos;
	tmpRegVal |= targetSDMMCClkDiv << SDMMC_CLKCR_CLKDIV_Pos;

	SDMMC1->CLKCR = tmpRegVal;

	// send cmd16: set block length to 512
	sendCommand( 512, SDMMC_CMD_SET_BLOCKLEN, SDMMC_CMD_SHORT_RESPONSE );
	if ( ! waitForResponseAndClearFlags() )
	{
		return false;
	}

	// ensure the card is in the transfer state, meaning it's ready to send/receive
	uint8_t cardState = 0;
	timeout = 0;
	while ( (cardState != 0x04) & (timeout < TIMEOUT_MAX) )
	{
		// send cmd13: request card status
		sendCommand( rca, SDMMC_CMD_SEND_STATUS, SDMMC_CMD_SHORT_RESPONSE );
		if ( ! waitForResponseAndClearFlags() )
		{
			return false;
		}

		cardState = SDMMC1->RESP1 >> 9;
		timeout++;
	}

	if ( timeout > TIMEOUT_MAX )
	{
		return false;
	}

	return true;
}
