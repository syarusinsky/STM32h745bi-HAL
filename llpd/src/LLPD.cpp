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
