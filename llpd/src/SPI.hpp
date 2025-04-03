#include "LLPD.hpp"

static void setup_spi_registers (SPI_TypeDef* spiPtr, const SPI_BAUD_RATE& baudRate, const SPI_CLK_POL& pol,
				const SPI_CLK_PHASE& phase, const SPI_DUPLEX& duplex, const SPI_FRAME_FORMAT& frameFormat,
				const SPI_DATA_SIZE& dataSize)
{
	if ( spiPtr )
	{
		// enable spi peripheral clock
		if ( spiPtr == SPI1 )
		{
			RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
		}
		else if ( spiPtr == SPI2 )
		{
			RCC->APB1LENR |= RCC_APB1LENR_SPI2EN;
		}
		else if ( spiPtr == SPI3 )
		{
			RCC->APB1LENR |= RCC_APB1LENR_SPI3EN;
		}
		else if ( spiPtr == SPI4 )
		{
			RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
		}
		else if ( spiPtr == SPI5 )
		{
			RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
		}
		else if ( spiPtr == SPI6 )
		{
			RCC->APB2ENR |= RCC_APB4ENR_SPI6EN;
		}

		// ensure spi is off
		spiPtr->CR1 &= ~(SPI_CR1_SPE);

		// reset registers
		if ( spiPtr == SPI1 )
		{
			RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI1RST);
		}
		else if ( spiPtr == SPI2 )
		{
			RCC->APB1LRSTR |= RCC_APB1LRSTR_SPI2RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_SPI2RST);
		}
		else if ( spiPtr == SPI3 )
		{
			RCC->APB1LRSTR |= RCC_APB1LRSTR_SPI3RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_SPI3RST);
		}
		else if ( spiPtr == SPI4 )
		{
			RCC->APB2RSTR |= RCC_APB2RSTR_SPI4RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI4RST);
		}
		else if ( spiPtr == SPI5 )
		{
			RCC->APB2RSTR |= RCC_APB2RSTR_SPI5RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI5RST);
		}
		else if ( spiPtr == SPI6 )
		{
			RCC->APB4RSTR |= RCC_APB4RSTR_SPI6RST;
			RCC->APB4RSTR &= ~(RCC_APB4RSTR_SPI6RST);
		}

		// set baud rate
		if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_2 )
		{
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_4 )
		{
			spiPtr->CFG1 |= SPI_CFG1_MBR_0;
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_8 )
		{
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
			spiPtr->CFG1 |= SPI_CFG1_MBR_1;
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_16 )
		{
			spiPtr->CFG1 |= SPI_CFG1_MBR_0;
			spiPtr->CFG1 |= SPI_CFG1_MBR_1;
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_32 )
		{
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
			spiPtr->CFG1 |= SPI_CFG1_MBR_2;
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_64 )
		{
			spiPtr->CFG1 |= SPI_CFG1_MBR_0;
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
			spiPtr->CFG1 |= SPI_CFG1_MBR_2;
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_128 )
		{
			spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
			spiPtr->CFG1 |= SPI_CFG1_MBR_1;
			spiPtr->CFG1 |= SPI_CFG1_MBR_2;
		}
		else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_256 )
		{
			spiPtr->CFG1 |= SPI_CFG1_MBR_0;
			spiPtr->CFG1 |= SPI_CFG1_MBR_1;
			spiPtr->CFG1 |= SPI_CFG1_MBR_2;
		}

		// set clock polarity and phase
		if ( pol == SPI_CLK_POL::LOW_IDLE )
		{
			spiPtr->CFG2 &= ~(SPI_CFG2_CPOL);
		}
		else if ( pol == SPI_CLK_POL::HIGH_IDLE )
		{
			spiPtr->CFG2 |= SPI_CFG2_CPOL;
		}

		if ( phase == SPI_CLK_PHASE::FIRST )
		{
			spiPtr->CFG2 &= ~(SPI_CFG2_CPHA);
		}
		else if ( phase == SPI_CLK_PHASE::SECOND )
		{
			spiPtr->CFG2 |= SPI_CFG2_CPHA;
		}

		// set duplex
		if ( duplex == SPI_DUPLEX::FULL )
		{
			spiPtr->CFG2 &= ~(SPI_CFG2_COMM);
		}
		else if ( duplex == SPI_DUPLEX::HALF )
		{
			spiPtr->CFG2 |= SPI_CFG2_COMM;
		}

		// set the frame format
		if ( frameFormat == SPI_FRAME_FORMAT::MSB_FIRST )
		{
			spiPtr->CFG2 &= ~(SPI_CFG2_LSBFRST);
		}
		else if ( frameFormat == SPI_FRAME_FORMAT::LSB_FIRST )
		{
			spiPtr->CFG2 |= SPI_CFG2_LSBFRST;
		}

		// ensure CRC is disabled
		spiPtr->CFG1 &= ~(SPI_CFG1_CRCEN);

		// set data size
		uint32_t dataSizeBits = 8;
		switch ( dataSize )
		{
			case SPI_DATA_SIZE::BITS_4:
				dataSizeBits = 4;

				break;
			case SPI_DATA_SIZE::BITS_5:
				dataSizeBits = 5;

				break;
			case SPI_DATA_SIZE::BITS_6:
				dataSizeBits = 6;

				break;
			case SPI_DATA_SIZE::BITS_7:
				dataSizeBits = 7;

				break;
			case SPI_DATA_SIZE::BITS_8:
				dataSizeBits = 8;

				break;
			case SPI_DATA_SIZE::BITS_9:
				dataSizeBits = 9;

				break;
			case SPI_DATA_SIZE::BITS_10:
				dataSizeBits = 10;

				break;
			case SPI_DATA_SIZE::BITS_11:
				dataSizeBits = 11;

				break;
			case SPI_DATA_SIZE::BITS_12:
				dataSizeBits = 12;

				break;
			case SPI_DATA_SIZE::BITS_13:
				dataSizeBits = 13;

				break;
			case SPI_DATA_SIZE::BITS_14:
				dataSizeBits = 14;

				break;
			case SPI_DATA_SIZE::BITS_15:
				dataSizeBits = 15;

				break;
			case SPI_DATA_SIZE::BITS_16:
				dataSizeBits = 16;

				break;
		}

		dataSizeBits -= 1; // we need to subtract one for the register value to be correct
		spiPtr->CFG1 &= ~(SPI_CFG1_DSIZE);
		spiPtr->CFG1 |= (dataSizeBits << SPI_CFG1_DSIZE_Pos);

		// set ss pin to software management
		spiPtr->CFG2 |= SPI_CFG2_SSM;
		spiPtr->CR1  |= SPI_CR1_SSI;

		// set as master
		spiPtr->CFG2 |= SPI_CFG2_MASTER;

		// lastly, enable SPI peripheral
		spiPtr->CR1 |= SPI_CR1_SPE;
	}
}

void LLPD::spi_master_init (const SPI_NUM& spiNum, const SPI_BAUD_RATE& baudRate, const SPI_CLK_POL& pol,
				const SPI_CLK_PHASE& phase, const SPI_DUPLEX& duplex, const SPI_FRAME_FORMAT& frameFormat,
				const SPI_DATA_SIZE& dataSize)
{
	SPI_TypeDef* spiPtr = nullptr;

	GPIO_TypeDef* gpioPtr = nullptr;

	GPIO_PORT gpioPort;
	GPIO_PIN  sckPin;
	GPIO_PIN  misoPin;
	GPIO_PIN  mosiPin;

	int afValue = 0;
	int afPinSk = 0; // sck pin
	int afPinMi = 0; // miso pin
	int afPinMo = 0; // mosi pin

	// set up appropriate gpio
	if ( spiNum == SPI_NUM::SPI_1 )
	{
		afValue = 5;
		afPinSk = 5;
		afPinMi = 6;
		afPinMo = 7;

		gpioPtr = GPIOA;

		gpioPort = GPIO_PORT::A;
		sckPin = GPIO_PIN::PIN_5;
		misoPin = GPIO_PIN::PIN_6;
		mosiPin = GPIO_PIN::PIN_7;

		spiPtr = SPI1;
	}
	else if ( spiNum == SPI_NUM::SPI_2 )
	{
		afValue = 5;
		afPinSk = 13;
		afPinMi = 14;
		afPinMo = 15;

		gpioPtr = GPIOB;

		gpioPort = GPIO_PORT::B;
		sckPin = GPIO_PIN::PIN_13;
		misoPin = GPIO_PIN::PIN_14;
		mosiPin = GPIO_PIN::PIN_15;

		spiPtr = SPI2;
	}
	else if ( spiNum == SPI_NUM::SPI_3 )
	{
		afValue = 6;
		afPinSk = 10;
		afPinMi = 11;
		afPinMo = 12;

		gpioPtr = GPIOC;

		gpioPort = GPIO_PORT::C;
		sckPin = GPIO_PIN::PIN_10;
		misoPin = GPIO_PIN::PIN_11;
		mosiPin = GPIO_PIN::PIN_12;

		spiPtr = SPI3;
	}
	else if ( spiNum == SPI_NUM::SPI_4 )
	{
		afValue = 5;
		afPinSk = 12;
		afPinMi = 13;
		afPinMo = 14;

		gpioPtr = GPIOE;

		gpioPort = GPIO_PORT::E;
		sckPin = GPIO_PIN::PIN_12;
		misoPin = GPIO_PIN::PIN_13;
		mosiPin = GPIO_PIN::PIN_14;

		spiPtr = SPI4;
	}
	else if ( spiNum == SPI_NUM::SPI_5 )
	{
		afValue = 5;
		afPinSk = 7;
		afPinMi = 8;
		afPinMo = 9;

		gpioPtr = GPIOF;

		gpioPort = GPIO_PORT::F;
		sckPin = GPIO_PIN::PIN_7;
		misoPin = GPIO_PIN::PIN_8;
		mosiPin = GPIO_PIN::PIN_9;

		spiPtr = SPI5;
	}
	else if ( spiNum == SPI_NUM::SPI_6 )
	{
		afValue = 5;
		afPinSk = 13;
		afPinMi = 12;
		afPinMo = 14;

		gpioPtr = GPIOG;

		gpioPort = GPIO_PORT::G;
		sckPin = GPIO_PIN::PIN_13;
		misoPin = GPIO_PIN::PIN_12;
		mosiPin = GPIO_PIN::PIN_14;

		spiPtr = SPI6;
	}

	// set alternate function registers for gpio
	LLPD::setup_alt_func_pin( gpioPtr, afPinSk, afValue );
	LLPD::setup_alt_func_pin( gpioPtr, afPinMi, afValue );
	LLPD::setup_alt_func_pin( gpioPtr, afPinMo, afValue );

	// spi sck
	LLPD::gpio_output_setup( gpioPort, sckPin, GPIO_PUPD::PULL_UP, GPIO_OUTPUT_TYPE::PUSH_PULL,
					GPIO_OUTPUT_SPEED::VERY_HIGH, true );

	// spi miso
	LLPD::gpio_output_setup( gpioPort, misoPin, GPIO_PUPD::PULL_UP, GPIO_OUTPUT_TYPE::OPEN_DRAIN,
					GPIO_OUTPUT_SPEED::VERY_HIGH, true );

	// spi1 mosi
	LLPD::gpio_output_setup( gpioPort, mosiPin, GPIO_PUPD::PULL_UP, GPIO_OUTPUT_TYPE::PUSH_PULL,
					GPIO_OUTPUT_SPEED::VERY_HIGH, true );

	// set sck low
	LLPD::gpio_output_set( gpioPort, sckPin, false );

	setup_spi_registers( spiPtr, baudRate, pol, phase, duplex, frameFormat, dataSize );
}

uint16_t LLPD::spi_master_send_and_recieve (const SPI_NUM& spiNum, uint8_t data)
{
	SPI_TypeDef* spiPtr = nullptr;

	switch ( spiNum )
	{
		case SPI_NUM::SPI_1:
			spiPtr = SPI1;

			break;
		case SPI_NUM::SPI_2:
			spiPtr = SPI2;

			break;
		case SPI_NUM::SPI_3:
			spiPtr = SPI3;

			break;
		case SPI_NUM::SPI_4:
			spiPtr = SPI4;

			break;
		case SPI_NUM::SPI_5:
			spiPtr = SPI5;

			break;
		case SPI_NUM::SPI_6:
			spiPtr = SPI6;

			break;
	}

	// start the transfer
	spiPtr->CR1 |= SPI_CR1_CSTART;

	// spinlock until ready to send
	while ( !(spiPtr->SR & SPI_SR_TXP) ) {}

	*(uint8_t*)&(spiPtr->TXDR) = data;

	// spinlock until there's something to read
	while ( !(spiPtr->SR & SPI_SR_RXP) ) {}

	return spiPtr->RXDR;
}

void LLPD::spi_master_change_baud_rate (const SPI_NUM& spiNum, const SPI_BAUD_RATE& baudRate)
{
	SPI_TypeDef* spiPtr = nullptr;

	if ( spiNum == SPI_NUM::SPI_1 )
	{
		spiPtr = SPI1;
	}
	else if ( spiNum == SPI_NUM::SPI_2 )
	{
		spiPtr = SPI2;
	}
	else if ( spiNum == SPI_NUM::SPI_3 )
	{
		spiPtr = SPI3;
	}
	else if ( spiNum == SPI_NUM::SPI_4 )
	{
		spiPtr = SPI4;
	}
	else if ( spiNum == SPI_NUM::SPI_5 )
	{
		spiPtr = SPI5;
	}
	else if ( spiNum == SPI_NUM::SPI_6 )
	{
		spiPtr = SPI6;
	}

	// ensure spi is off
	spiPtr->CR1 &= ~(SPI_CR1_SPE);

	// set baud rate
	if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_2 )
	{
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_4 )
	{
		spiPtr->CFG1 |= SPI_CFG1_MBR_0;
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_8 )
	{
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
		spiPtr->CFG1 |= SPI_CFG1_MBR_1;
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_16 )
	{
		spiPtr->CFG1 |= SPI_CFG1_MBR_0;
		spiPtr->CFG1 |= SPI_CFG1_MBR_1;
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_2);
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_32 )
	{
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
		spiPtr->CFG1 |= SPI_CFG1_MBR_2;
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_64 )
	{
		spiPtr->CFG1 |= SPI_CFG1_MBR_0;
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_1);
		spiPtr->CFG1 |= SPI_CFG1_MBR_2;
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_128 )
	{
		spiPtr->CFG1 &= ~(SPI_CFG1_MBR_0);
		spiPtr->CFG1 |= SPI_CFG1_MBR_1;
		spiPtr->CFG1 |= SPI_CFG1_MBR_2;
	}
	else if ( baudRate == SPI_BAUD_RATE::SYSCLK_DIV_BY_256 )
	{
		spiPtr->CFG1 |= SPI_CFG1_MBR_0;
		spiPtr->CFG1 |= SPI_CFG1_MBR_1;
		spiPtr->CFG1 |= SPI_CFG1_MBR_2;
	}

	// lastly, enable SPI peripheral
	spiPtr->CR1 |= SPI_CR1_SPE;
}
