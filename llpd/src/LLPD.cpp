#include "LLPD.hpp"

void LLPD::setup_alt_func_pin (GPIO_TypeDef* gpioPtr, int pinNum, const int afValue)
{
	if ( pinNum <= 7 )
	{
		// clear alternate function register for pin
		gpioPtr->AFR[0] &= ~(0b1111 << (pinNum * 4)); // 4 is the width of the alternate function pin value

		// set to alternate function num
		gpioPtr->AFR[0] |= (afValue << (pinNum * 4));
	}
	else
	{
		// we have to subtract 8 since we're modifying the high register
		pinNum -= 8;

		// clear alternate function register for pin
		gpioPtr->AFR[1] &= ~(0b1111 << (pinNum * 4));

		// set to alternate function num
		gpioPtr->AFR[1] |= (afValue << (pinNum * 4));
	}
}

#include "GPIO.hpp"
#include "RCC.hpp"
#include "DAC.hpp"
#include "ADC.hpp"
#include "Timers.hpp"
#include "SPI.hpp"
#include "I2C.hpp"
#include "USART.hpp"
#include "OpAmp.hpp"
#include "FMC.hpp"
#include "SDMMC.hpp"
#include "LTDC.hpp"
#include "HSEM.hpp"

// this function is called in system_stm32h7xx.c and can be used to ensure certain things are done on reset
extern "C" void Custom_Reset_Handler(void)
{
	// dma may still be running from the last reset
	LLPD::dac_dma_stop();
}

// sdmmc1 dma handling
extern "C" void SDMMC1_IRQHandler (void)
{
	if ( SDMMC1->STA & SDMMC_STA_DATAEND )
	{
		// clear data flags
		sdmmcClearDataFlags();

		// disable interrupts
		SDMMC1->MASK &= ~( SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE | SDMMC_MASK_TXUNDERRIE | SDMMC_MASK_DATAENDIE
					| SDMMC_MASK_RXOVERRIE );

		// disable data transfer command
		SDMMC1->CMD &= ~(SDMMC_CMD_CMDTRANS);

		// reset registers
		SDMMC1->DLEN = 0;
		SDMMC1->DCTRL = 0;
		SDMMC1->IDMACTRL = 0;

		if ( sdmmcMultiBlockTransfer )
		{
			// enable command stop
			SDMMC1->CMD |= SDMMC_CMD_CMDSTOP;

			// send cmd12: stop transmission
			sendCommand( 0, SDMMC_CMD_STOP_TRANSMISSION, SDMMC_CMD_SHORT_RESPONSE );
			if ( ! waitForResponseAndClearFlags() )
			{
				sdmmcTransferError = true;
			}

			// disable command stop
			SDMMC1->CMD &= ~(SDMMC_CMD_CMDSTOP);
		}

		sdmmcTransferCompleted = true;
		sdmmcMultiBlockTransfer = false;
	}
	else if ( SDMMC1->STA & (SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_TXUNDERR | SDMMC_STA_RXOVERR) )
	{
		sdmmcTransferError = true;
		sdmmcTransferErrorStaRegVal = SDMMC1->STA;

		sdmmcClearDataFlags();

		// disable interrupts
		SDMMC1->MASK &= ~( SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE | SDMMC_MASK_TXUNDERRIE | SDMMC_MASK_DATAENDIE
					| SDMMC_MASK_RXOVERRIE );

		sdmmcTransferCompleted = true;
		sdmmcMultiBlockTransfer = false;
	}
}
