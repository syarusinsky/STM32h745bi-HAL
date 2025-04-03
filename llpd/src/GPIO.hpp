#include "LLPD.hpp"

static inline GPIO_TypeDef* PortToPortPtr (const GPIO_PORT& port)
{
	switch ( port )
	{
		case GPIO_PORT::A:
			return GPIOA;

		case GPIO_PORT::B:
			return GPIOB;

		case GPIO_PORT::C:
			return GPIOC;

		case GPIO_PORT::D:
			return GPIOD;

		case GPIO_PORT::E:
			return GPIOE;

		case GPIO_PORT::F:
			return GPIOF;

		case GPIO_PORT::G:
			return GPIOG;

		case GPIO_PORT::H:
			return GPIOH;
	}

	return nullptr;
}

static inline void PinToModeBits (const GPIO_PIN& pin, uint32_t& modeBit0, uint32_t& modeBit1)
{
	switch ( pin )
	{
		case GPIO_PIN::PIN_0:
			modeBit0 = GPIO_MODER_MODE0_0;
			modeBit1 = GPIO_MODER_MODE0_1;

			break;
		case GPIO_PIN::PIN_1:
			modeBit0 = GPIO_MODER_MODE1_0;
			modeBit1 = GPIO_MODER_MODE1_1;

			break;
		case GPIO_PIN::PIN_2:
			modeBit0 = GPIO_MODER_MODE2_0;
			modeBit1 = GPIO_MODER_MODE2_1;

			break;
		case GPIO_PIN::PIN_3:
			modeBit0 = GPIO_MODER_MODE3_0;
			modeBit1 = GPIO_MODER_MODE3_1;

			break;
		case GPIO_PIN::PIN_4:
			modeBit0 = GPIO_MODER_MODE4_0;
			modeBit1 = GPIO_MODER_MODE4_1;

			break;
		case GPIO_PIN::PIN_5:
			modeBit0 = GPIO_MODER_MODE5_0;
			modeBit1 = GPIO_MODER_MODE5_1;

			break;
		case GPIO_PIN::PIN_6:
			modeBit0 = GPIO_MODER_MODE6_0;
			modeBit1 = GPIO_MODER_MODE6_1;

			break;
		case GPIO_PIN::PIN_7:
			modeBit0 = GPIO_MODER_MODE7_0;
			modeBit1 = GPIO_MODER_MODE7_1;

			break;
		case GPIO_PIN::PIN_8:
			modeBit0 = GPIO_MODER_MODE8_0;
			modeBit1 = GPIO_MODER_MODE8_1;

			break;
		case GPIO_PIN::PIN_9:
			modeBit0 = GPIO_MODER_MODE9_0;
			modeBit1 = GPIO_MODER_MODE9_1;

			break;
		case GPIO_PIN::PIN_10:
			modeBit0 = GPIO_MODER_MODE10_0;
			modeBit1 = GPIO_MODER_MODE10_1;

			break;
		case GPIO_PIN::PIN_11:
			modeBit0 = GPIO_MODER_MODE11_0;
			modeBit1 = GPIO_MODER_MODE11_1;

			break;
		case GPIO_PIN::PIN_12:
			modeBit0 = GPIO_MODER_MODE12_0;
			modeBit1 = GPIO_MODER_MODE12_1;

			break;
		case GPIO_PIN::PIN_13:
			modeBit0 = GPIO_MODER_MODE13_0;
			modeBit1 = GPIO_MODER_MODE13_1;

			break;
		case GPIO_PIN::PIN_14:
			modeBit0 = GPIO_MODER_MODE14_0;
			modeBit1 = GPIO_MODER_MODE14_1;

			break;
		case GPIO_PIN::PIN_15:
			modeBit0 = GPIO_MODER_MODE15_0;
			modeBit1 = GPIO_MODER_MODE15_1;

			break;
		default:
			return;
	}
}

static inline void PinToPUPDBits (const GPIO_PIN& pin, uint32_t& pupdBit0, uint32_t& pupdBit1)
{
	switch ( pin )
	{
		case GPIO_PIN::PIN_0:
			pupdBit0 = GPIO_PUPDR_PUPD0_0;
			pupdBit1 = GPIO_PUPDR_PUPD0_1;

			break;
		case GPIO_PIN::PIN_1:
			pupdBit0 = GPIO_PUPDR_PUPD1_0;
			pupdBit1 = GPIO_PUPDR_PUPD1_1;

			break;
		case GPIO_PIN::PIN_2:
			pupdBit0 = GPIO_PUPDR_PUPD2_0;
			pupdBit1 = GPIO_PUPDR_PUPD2_1;

			break;
		case GPIO_PIN::PIN_3:
			pupdBit0 = GPIO_PUPDR_PUPD3_0;
			pupdBit1 = GPIO_PUPDR_PUPD3_1;

			break;
		case GPIO_PIN::PIN_4:
			pupdBit0 = GPIO_PUPDR_PUPD4_0;
			pupdBit1 = GPIO_PUPDR_PUPD4_1;

			break;
		case GPIO_PIN::PIN_5:
			pupdBit0 = GPIO_PUPDR_PUPD5_0;
			pupdBit1 = GPIO_PUPDR_PUPD5_1;

			break;
		case GPIO_PIN::PIN_6:
			pupdBit0 = GPIO_PUPDR_PUPD6_0;
			pupdBit1 = GPIO_PUPDR_PUPD6_1;

			break;
		case GPIO_PIN::PIN_7:
			pupdBit0 = GPIO_PUPDR_PUPD7_0;
			pupdBit1 = GPIO_PUPDR_PUPD7_1;

			break;
		case GPIO_PIN::PIN_8:
			pupdBit0 = GPIO_PUPDR_PUPD8_0;
			pupdBit1 = GPIO_PUPDR_PUPD8_1;

			break;
		case GPIO_PIN::PIN_9:
			pupdBit0 = GPIO_PUPDR_PUPD9_0;
			pupdBit1 = GPIO_PUPDR_PUPD9_1;

			break;
		case GPIO_PIN::PIN_10:
			pupdBit0 = GPIO_PUPDR_PUPD10_0;
			pupdBit1 = GPIO_PUPDR_PUPD10_1;

			break;
		case GPIO_PIN::PIN_11:
			pupdBit0 = GPIO_PUPDR_PUPD11_0;
			pupdBit1 = GPIO_PUPDR_PUPD11_1;

			break;
		case GPIO_PIN::PIN_12:
			pupdBit0 = GPIO_PUPDR_PUPD12_0;
			pupdBit1 = GPIO_PUPDR_PUPD12_1;

			break;
		case GPIO_PIN::PIN_13:
			pupdBit0 = GPIO_PUPDR_PUPD13_0;
			pupdBit1 = GPIO_PUPDR_PUPD13_1;

			break;
		case GPIO_PIN::PIN_14:
			pupdBit0 = GPIO_PUPDR_PUPD14_0;
			pupdBit1 = GPIO_PUPDR_PUPD14_1;

			break;
		case GPIO_PIN::PIN_15:
			pupdBit0 = GPIO_PUPDR_PUPD15_0;
			pupdBit1 = GPIO_PUPDR_PUPD15_1;

			break;
		default:
			return;
	}
}

static inline void PinToTypeBit (const GPIO_PIN& pin, uint32_t& typeBit)
{
	switch ( pin )
	{
		case GPIO_PIN::PIN_0:
			typeBit = GPIO_OTYPER_OT0;

			break;
		case GPIO_PIN::PIN_1:
			typeBit = GPIO_OTYPER_OT1;

			break;
		case GPIO_PIN::PIN_2:
			typeBit = GPIO_OTYPER_OT2;

			break;
		case GPIO_PIN::PIN_3:
			typeBit = GPIO_OTYPER_OT3;

			break;
		case GPIO_PIN::PIN_4:
			typeBit = GPIO_OTYPER_OT4;

			break;
		case GPIO_PIN::PIN_5:
			typeBit = GPIO_OTYPER_OT5;

			break;
		case GPIO_PIN::PIN_6:
			typeBit = GPIO_OTYPER_OT6;

			break;
		case GPIO_PIN::PIN_7:
			typeBit = GPIO_OTYPER_OT7;

			break;
		case GPIO_PIN::PIN_8:
			typeBit = GPIO_OTYPER_OT8;

			break;
		case GPIO_PIN::PIN_9:
			typeBit = GPIO_OTYPER_OT9;

			break;
		case GPIO_PIN::PIN_10:
			typeBit = GPIO_OTYPER_OT10;

			break;
		case GPIO_PIN::PIN_11:
			typeBit = GPIO_OTYPER_OT11;

			break;
		case GPIO_PIN::PIN_12:
			typeBit = GPIO_OTYPER_OT12;

			break;
		case GPIO_PIN::PIN_13:
			typeBit = GPIO_OTYPER_OT13;

			break;
		case GPIO_PIN::PIN_14:
			typeBit = GPIO_OTYPER_OT14;

			break;
		case GPIO_PIN::PIN_15:
			typeBit = GPIO_OTYPER_OT15;

			break;
		default:
			return;
	}
}

static inline void PinToSpeedBits (const GPIO_PIN& pin, uint32_t& speedBit0, uint32_t& speedBit1)
{
	switch ( pin )
	{
		case GPIO_PIN::PIN_0:
			speedBit0 = GPIO_OSPEEDR_OSPEED0_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED0_1;

			break;
		case GPIO_PIN::PIN_1:
			speedBit0 = GPIO_OSPEEDR_OSPEED1_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED1_1;

			break;
		case GPIO_PIN::PIN_2:
			speedBit0 = GPIO_OSPEEDR_OSPEED2_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED2_1;

			break;
		case GPIO_PIN::PIN_3:
			speedBit0 = GPIO_OSPEEDR_OSPEED3_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED3_1;

			break;
		case GPIO_PIN::PIN_4:
			speedBit0 = GPIO_OSPEEDR_OSPEED4_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED4_1;

			break;
		case GPIO_PIN::PIN_5:
			speedBit0 = GPIO_OSPEEDR_OSPEED5_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED5_1;

			break;
		case GPIO_PIN::PIN_6:
			speedBit0 = GPIO_OSPEEDR_OSPEED6_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED6_1;

			break;
		case GPIO_PIN::PIN_7:
			speedBit0 = GPIO_OSPEEDR_OSPEED7_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED7_1;

			break;
		case GPIO_PIN::PIN_8:
			speedBit0 = GPIO_OSPEEDR_OSPEED8_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED8_1;

			break;
		case GPIO_PIN::PIN_9:
			speedBit0 = GPIO_OSPEEDR_OSPEED9_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED9_1;

			break;
		case GPIO_PIN::PIN_10:
			speedBit0 = GPIO_OSPEEDR_OSPEED10_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED10_1;

			break;
		case GPIO_PIN::PIN_11:
			speedBit0 = GPIO_OSPEEDR_OSPEED11_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED11_1;

			break;
		case GPIO_PIN::PIN_12:
			speedBit0 = GPIO_OSPEEDR_OSPEED12_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED12_1;

			break;
		case GPIO_PIN::PIN_13:
			speedBit0 = GPIO_OSPEEDR_OSPEED13_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED13_1;

			break;
		case GPIO_PIN::PIN_14:
			speedBit0 = GPIO_OSPEEDR_OSPEED14_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED14_1;

			break;
		case GPIO_PIN::PIN_15:
			speedBit0 = GPIO_OSPEEDR_OSPEED15_0;
			speedBit1 = GPIO_OSPEEDR_OSPEED15_1;

			break;
		default:
			return;
	}
}

void LLPD::gpio_enable_clock (const GPIO_PORT& port)
{
	uint32_t portClockVal;

	switch ( port )
	{
		case GPIO_PORT::A:
			portClockVal = RCC_AHB4ENR_GPIOAEN;

			break;
		case GPIO_PORT::B:
			portClockVal = RCC_AHB4ENR_GPIOBEN;

			break;
		case GPIO_PORT::C:
			portClockVal = RCC_AHB4ENR_GPIOCEN;

			break;
		case GPIO_PORT::D:
			portClockVal = RCC_AHB4ENR_GPIODEN;

			break;
		case GPIO_PORT::E:
			portClockVal = RCC_AHB4ENR_GPIOEEN;

			break;
		case GPIO_PORT::F:
			portClockVal = RCC_AHB4ENR_GPIOFEN;

			break;
		case GPIO_PORT::G:
			portClockVal = RCC_AHB4ENR_GPIOGEN;

			break;
		case GPIO_PORT::H:
			portClockVal = RCC_AHB4ENR_GPIOHEN;

			break;
		default:
			return;
	}

	RCC->AHB4ENR |= portClockVal;
}

void LLPD::gpio_digital_input_setup (const GPIO_PORT& port, const GPIO_PIN& pin, const GPIO_PUPD& pupd, bool alternateFunc)
{
	GPIO_TypeDef* portPtr = PortToPortPtr( port );

	uint32_t modeBit0;
	uint32_t modeBit1;
	uint32_t pupdBit0;
	uint32_t pupdBit1;

	PinToModeBits( pin, modeBit0, modeBit1 );
	PinToPUPDBits( pin, pupdBit0, pupdBit1 );

	// set mode to input or alternate function if desired
	if ( alternateFunc )
	{
		portPtr->MODER &= ~(modeBit0);
		portPtr->MODER |= modeBit1;
	}
	else
	{
		portPtr->MODER &= ~(modeBit0);
		portPtr->MODER &= ~(modeBit1);
	}

	// set pull-up/pull-down resistor settings
	if ( pupd == GPIO_PUPD::NONE )
	{
		portPtr->PUPDR &= ~(pupdBit0);
		portPtr->PUPDR &= ~(pupdBit1);
	}
	else if ( pupd == GPIO_PUPD::PULL_UP )
	{
		portPtr->PUPDR |= pupdBit0;
		portPtr->PUPDR &= ~(pupdBit1);
	}
	else if ( pupd == GPIO_PUPD::PULL_DOWN )
	{
		portPtr->PUPDR &= ~(pupdBit0);
		portPtr->PUPDR |= pupdBit1;
	}
}

void LLPD::gpio_analog_setup (const GPIO_PORT& port, const GPIO_PIN& pin)
{
	GPIO_TypeDef* portPtr = PortToPortPtr( port );

	uint32_t modeBit0;
	uint32_t modeBit1;

	PinToModeBits( pin, modeBit0, modeBit1 );

	// set mode to analog
	portPtr->MODER |= modeBit0;
	portPtr->MODER |= modeBit1;
}

void LLPD::gpio_output_setup (const GPIO_PORT& port, const GPIO_PIN& pin, const GPIO_PUPD& pupd,
				const GPIO_OUTPUT_TYPE& type, const GPIO_OUTPUT_SPEED& speed, bool alternateFunc)
{
	GPIO_TypeDef* portPtr = PortToPortPtr( port );

	uint32_t modeBit0;
	uint32_t modeBit1;
	uint32_t pupdBit0;
	uint32_t pupdBit1;
	uint32_t typeBit;
	uint32_t speedBit0;
	uint32_t speedBit1;

	PinToModeBits ( pin, modeBit0,  modeBit1  );
	PinToPUPDBits ( pin, pupdBit0,  pupdBit1  );
	PinToTypeBit  ( pin, typeBit              );
	PinToSpeedBits( pin, speedBit0, speedBit1 );

	// set mode to output or alternate function if desired
	if ( alternateFunc )
	{
		portPtr->MODER &= ~(modeBit0);
		portPtr->MODER |= modeBit1;
	}
	else
	{
		portPtr->MODER |= modeBit0;
		portPtr->MODER &= ~(modeBit1);
	}

	// set pull-up/pull-down resistor settings
	if ( pupd == GPIO_PUPD::NONE )
	{
		portPtr->PUPDR &= ~(pupdBit0);
		portPtr->PUPDR &= ~(pupdBit1);
	}
	else if ( pupd == GPIO_PUPD::PULL_UP )
	{
		portPtr->PUPDR |= pupdBit0;
		portPtr->PUPDR &= ~(pupdBit1);
	}
	else if ( pupd == GPIO_PUPD::PULL_DOWN )
	{
		portPtr->PUPDR &= ~(pupdBit0);
		portPtr->PUPDR |= pupdBit1;
	}

	// set output type settings
	if ( type == GPIO_OUTPUT_TYPE::PUSH_PULL )
	{
		portPtr->OTYPER &= ~(typeBit);
	}
	else if ( type == GPIO_OUTPUT_TYPE::OPEN_DRAIN )
	{
		portPtr->OTYPER |= typeBit;
	}

	// set output speed settings
	if ( speed == GPIO_OUTPUT_SPEED::LOW )
	{
		portPtr->OSPEEDR &= ~(speedBit0);
		portPtr->OSPEEDR &= ~(speedBit1);
	}
	else if ( speed == GPIO_OUTPUT_SPEED::MEDIUM )
	{
		portPtr->OSPEEDR |= speedBit0;
		portPtr->OSPEEDR &= ~(speedBit1);
	}
	else if ( speed == GPIO_OUTPUT_SPEED::HIGH )
	{
		portPtr->OSPEEDR &= ~(speedBit0);
		portPtr->OSPEEDR |= speedBit1;
	}
	else if ( speed == GPIO_OUTPUT_SPEED::VERY_HIGH )
	{
		portPtr->OSPEEDR |= speedBit0;
		portPtr->OSPEEDR |= speedBit1;
	}
}

bool LLPD::gpio_input_get (const GPIO_PORT& port, const GPIO_PIN& pin)
{
	GPIO_TypeDef* portPtr = PortToPortPtr( port );

	if ( pin == GPIO_PIN::PIN_0 )
	{
		return portPtr->IDR & GPIO_IDR_ID0;
	}
	else if ( pin == GPIO_PIN::PIN_1 )
	{
		return portPtr->IDR & GPIO_IDR_ID1;
	}
	else if ( pin == GPIO_PIN::PIN_2 )
	{
		return portPtr->IDR & GPIO_IDR_ID2;
	}
	else if ( pin == GPIO_PIN::PIN_3 )
	{
		return portPtr->IDR & GPIO_IDR_ID3;
	}
	else if ( pin == GPIO_PIN::PIN_4 )
	{
		return portPtr->IDR & GPIO_IDR_ID4;
	}
	else if ( pin == GPIO_PIN::PIN_5 )
	{
		return portPtr->IDR & GPIO_IDR_ID5;
	}
	else if ( pin == GPIO_PIN::PIN_6 )
	{
		return portPtr->IDR & GPIO_IDR_ID6;
	}
	else if ( pin == GPIO_PIN::PIN_7 )
	{
		return portPtr->IDR & GPIO_IDR_ID7;
	}
	else if ( pin == GPIO_PIN::PIN_8 )
	{
		return portPtr->IDR & GPIO_IDR_ID8;
	}
	else if ( pin == GPIO_PIN::PIN_9 )
	{
		return portPtr->IDR & GPIO_IDR_ID9;
	}
	else if ( pin == GPIO_PIN::PIN_10 )
	{
		return portPtr->IDR & GPIO_IDR_ID10;
	}
	else if ( pin == GPIO_PIN::PIN_11 )
	{
		return portPtr->IDR & GPIO_IDR_ID11;
	}
	else if ( pin == GPIO_PIN::PIN_12 )
	{
		return portPtr->IDR & GPIO_IDR_ID12;
	}
	else if ( pin == GPIO_PIN::PIN_13 )
	{
		return portPtr->IDR & GPIO_IDR_ID13;
	}
	else if ( pin == GPIO_PIN::PIN_14 )
	{
		return portPtr->IDR & GPIO_IDR_ID14;
	}
	else if ( pin == GPIO_PIN::PIN_15 )
	{
		return portPtr->IDR & GPIO_IDR_ID15;
	}
	else
	{
		return false;
	}
}

void LLPD::gpio_output_set (const GPIO_PORT& port, const GPIO_PIN& pin, bool set)
{
	GPIO_TypeDef* portPtr = PortToPortPtr( port );

	uint32_t setBit;
	uint32_t resetBit;

	switch ( pin )
	{
		case GPIO_PIN::PIN_0:
			setBit   = GPIO_BSRR_BS0;
			resetBit = GPIO_BSRR_BR0;

			break;
		case GPIO_PIN::PIN_1:
			setBit   = GPIO_BSRR_BS1;
			resetBit = GPIO_BSRR_BR1;

			break;
		case GPIO_PIN::PIN_2:
			setBit   = GPIO_BSRR_BS2;
			resetBit = GPIO_BSRR_BR2;

			break;
		case GPIO_PIN::PIN_3:
			setBit   = GPIO_BSRR_BS3;
			resetBit = GPIO_BSRR_BR3;

			break;
		case GPIO_PIN::PIN_4:
			setBit   = GPIO_BSRR_BS4;
			resetBit = GPIO_BSRR_BR4;

			break;
		case GPIO_PIN::PIN_5:
			setBit   = GPIO_BSRR_BS5;
			resetBit = GPIO_BSRR_BR5;

			break;
		case GPIO_PIN::PIN_6:
			setBit   = GPIO_BSRR_BS6;
			resetBit = GPIO_BSRR_BR6;

			break;
		case GPIO_PIN::PIN_7:
			setBit   = GPIO_BSRR_BS7;
			resetBit = GPIO_BSRR_BR7;

			break;
		case GPIO_PIN::PIN_8:
			setBit   = GPIO_BSRR_BS8;
			resetBit = GPIO_BSRR_BR8;

			break;
		case GPIO_PIN::PIN_9:
			setBit   = GPIO_BSRR_BS9;
			resetBit = GPIO_BSRR_BR9;

			break;
		case GPIO_PIN::PIN_10:
			setBit   = GPIO_BSRR_BS10;
			resetBit = GPIO_BSRR_BR10;

			break;
		case GPIO_PIN::PIN_11:
			setBit   = GPIO_BSRR_BS11;
			resetBit = GPIO_BSRR_BR11;

			break;
		case GPIO_PIN::PIN_12:
			setBit   = GPIO_BSRR_BS12;
			resetBit = GPIO_BSRR_BR12;

			break;
		case GPIO_PIN::PIN_13:
			setBit   = GPIO_BSRR_BS13;
			resetBit = GPIO_BSRR_BR13;

			break;
		case GPIO_PIN::PIN_14:
			setBit   = GPIO_BSRR_BS14;
			resetBit = GPIO_BSRR_BR14;

			break;
		case GPIO_PIN::PIN_15:
			setBit   = GPIO_BSRR_BS15;
			resetBit = GPIO_BSRR_BR15;

			break;
		default:
			return;
	}

	if ( set )
	{
		portPtr->BSRR |= setBit;
	}
	else
	{
		portPtr->BSRR |= resetBit;
	}
}

void LLPD::gpio_test()
{
	GPIOA->ODR |= GPIO_ODR_OD0;
}
