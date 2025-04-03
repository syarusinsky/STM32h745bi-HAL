#include "LLPD.hpp"

#include <stdlib.h>

static uint16_t usart1WordLenMask = 0b0000000011111111;
static uint16_t usart2WordLenMask = 0b0000000011111111;
static uint16_t usart3WordLenMask = 0b0000000011111111;
static uint16_t usart6WordLenMask = 0b0000000011111111;

void LLPD::usart_init (const USART_NUM& usartNum, const USART_WORD_LENGTH& wordLen, const USART_PARITY& parity,
			const USART_CONF& conf, const USART_STOP_BITS& stopBits, const unsigned int periphClockFreq,
			const unsigned int baudRate)
{
	USART_TypeDef* usart = nullptr;
	uint16_t* usartWordLenMask = nullptr;

	if ( usartNum == USART_NUM::USART_1 )
	{
		// enable usart peripheral clock
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

		usart = USART1;
		usartWordLenMask = &usart1WordLenMask;

		// set alternate function registers to af7 for b6 and b7
		setup_alt_func_pin( GPIOB, 6, 7 );
		setup_alt_func_pin( GPIOB, 7, 7 );

		// tx
		gpio_output_setup( GPIO_PORT::B, GPIO_PIN::PIN_6, GPIO_PUPD::PULL_DOWN, GPIO_OUTPUT_TYPE::PUSH_PULL,
					GPIO_OUTPUT_SPEED::HIGH, true );
		gpio_output_set( GPIO_PORT::B, GPIO_PIN::PIN_6, false );

		// rx
		gpio_digital_input_setup( GPIO_PORT::B, GPIO_PIN::PIN_7, GPIO_PUPD::NONE, true );
		gpio_output_set( GPIO_PORT::B, GPIO_PIN::PIN_7, false );
	}
	else if ( usartNum == USART_NUM::USART_2 )
	{
		// enable usart peripheral clock
		RCC->APB1LENR |= RCC_APB1LENR_USART2EN;

		usart = USART2;
		usartWordLenMask = &usart2WordLenMask;

		// set alternate function registers to af7 for d5 and d6
		setup_alt_func_pin( GPIOD, 5, 7 );
		setup_alt_func_pin( GPIOD, 6, 7 );

		// tx
		gpio_output_setup( GPIO_PORT::D, GPIO_PIN::PIN_5, GPIO_PUPD::PULL_DOWN, GPIO_OUTPUT_TYPE::PUSH_PULL,
					GPIO_OUTPUT_SPEED::HIGH, true );
		gpio_output_set( GPIO_PORT::D, GPIO_PIN::PIN_5, false );

		// rx
		gpio_digital_input_setup( GPIO_PORT::D, GPIO_PIN::PIN_6, GPIO_PUPD::NONE, true );
		gpio_output_set( GPIO_PORT::D, GPIO_PIN::PIN_6, false );
	}
	else if ( usartNum == USART_NUM::USART_3 )
	{
		// enable usart peripheral clock
		RCC->APB1LENR |= RCC_APB1LENR_USART3EN;

		usart = USART3;
		usartWordLenMask = &usart3WordLenMask;

		// set alternate function registers to af7 for c10 and c11
		setup_alt_func_pin( GPIOC, 10, 7 );
		setup_alt_func_pin( GPIOC, 11, 7 );

		// tx
		gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_10, GPIO_PUPD::PULL_DOWN, GPIO_OUTPUT_TYPE::PUSH_PULL,
					GPIO_OUTPUT_SPEED::HIGH, true );
		gpio_output_set( GPIO_PORT::C, GPIO_PIN::PIN_10, false );

		// rx
		gpio_digital_input_setup( GPIO_PORT::C, GPIO_PIN::PIN_11, GPIO_PUPD::NONE, true );
		gpio_output_set( GPIO_PORT::C, GPIO_PIN::PIN_11, false );
	}
	else if ( usartNum == USART_NUM::USART_6 )
	{
		// enable usart peripheral clock
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;

		usart = USART6;
		usartWordLenMask = &usart6WordLenMask;

		// set alternate function registers to af7 for c6 and c7
		setup_alt_func_pin( GPIOC, 6, 7 );
		setup_alt_func_pin( GPIOC, 7, 7 );

		// tx
		gpio_output_setup( GPIO_PORT::C, GPIO_PIN::PIN_6, GPIO_PUPD::PULL_DOWN, GPIO_OUTPUT_TYPE::PUSH_PULL,
					GPIO_OUTPUT_SPEED::HIGH, true );
		gpio_output_set( GPIO_PORT::C, GPIO_PIN::PIN_6, false );

		// rx
		gpio_digital_input_setup( GPIO_PORT::C, GPIO_PIN::PIN_7, GPIO_PUPD::NONE, true );
		gpio_output_set( GPIO_PORT::C, GPIO_PIN::PIN_7, false );
	}

	if ( usart && usartWordLenMask )
	{
		// ensure usart is off
		usart->CR1 &= ~(USART_CR1_UE);

		// disable FIFO mode
		usart->CR1 &= ~(USART_CR1_FIFOEN);

		// reset registers
		if ( usartNum == USART_NUM::USART_1 )
		{
			RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_USART1RST);
		}
		else if ( usartNum == USART_NUM::USART_2 )
		{
			RCC->APB1LRSTR |= RCC_APB1LRSTR_USART2RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_USART2RST);
		}
		else if ( usartNum == USART_NUM::USART_3 )
		{
			RCC->APB1LRSTR |= RCC_APB1LRSTR_USART3RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_USART3RST);
		}
		else if ( usartNum == USART_NUM::USART_6 )
		{
			RCC->APB2RSTR |= RCC_APB2RSTR_USART6RST;
			RCC->APB2RSTR &= ~(RCC_APB2RSTR_USART6RST);
		}

		// set word length
		if ( wordLen == USART_WORD_LENGTH::BITS_7 )
		{
			usart->CR1 &= ~(USART_CR1_M0);
			usart->CR1 |= USART_CR1_M1;
			*usartWordLenMask = 0b0000000001111111;
		}
		else if ( wordLen == USART_WORD_LENGTH::BITS_8 )
		{
			usart->CR1 &= ~(USART_CR1_M0);
			usart->CR1 &= ~(USART_CR1_M1);
			*usartWordLenMask = 0b0000000011111111;
		}
		else if ( wordLen == USART_WORD_LENGTH::BITS_9 )
		{
			usart->CR1 |= USART_CR1_M0;
			usart->CR1 &= ~(USART_CR1_M1);
			*usartWordLenMask = 0b0000000111111111;
		}

		// set parity
		if ( parity == USART_PARITY::NONE )
		{
			usart->CR1 &= ~(USART_CR1_PCE);
		}
		else if ( parity == USART_PARITY::ODD )
		{
			usart->CR1 |= USART_CR1_PCE;
			usart->CR1 |= USART_CR1_PS;
		}
		else if ( parity == USART_PARITY::EVEN )
		{
			usart->CR1 |= USART_CR1_PCE;
			usart->CR1 &= ~(USART_CR1_PS);
		}

		// enable transmitter and/or reciever
		bool usingReceiver = false;
		if ( conf == USART_CONF::TX_ONLY )
		{
			usart->CR1 |= USART_CR1_TE;
			usart->CR1 &= ~(USART_CR1_RE);
			usingReceiver = false;
		}
		else if ( conf == USART_CONF::RX_ONLY )
		{
			usart->CR1 &= ~(USART_CR1_TE);
			usart->CR1 |= USART_CR1_RE;
			usingReceiver = true;
		}
		else if ( conf == USART_CONF::TX_AND_RX )
		{
			usart->CR1 |= USART_CR1_TE;
			usart->CR1 |= USART_CR1_RE;
			usingReceiver = true;
		}

		// set oversampling to by 16
		usart->CR1 &= ~(USART_CR1_OVER8);

		// set number of stop bits
		if ( stopBits == USART_STOP_BITS::BITS_1 )
		{
			usart->CR2 &= ~(USART_CR2_STOP);
		}
		else if ( stopBits == USART_STOP_BITS::BITS_2 )
		{
			usart->CR2 &= ~(USART_CR2_STOP_0);
			usart->CR2 |= USART_CR2_STOP_1;
		}

		// set baud rate
		uint16_t usartDiv = periphClockFreq / baudRate;
		usart->BRR = usartDiv;

		if ( usingReceiver )
		{
			// enable reciever interrupt
			usart->CR1 |= USART_CR1_RXNEIE;

			// enable interrupt service routine with priority behind audio timer
			if ( usartNum == USART_NUM::USART_1 )
			{
				NVIC_SetPriority( USART1_IRQn, 0x01 );
				NVIC_EnableIRQ( USART1_IRQn );
			}
			else if ( usartNum == USART_NUM::USART_2 )
			{
				NVIC_SetPriority( USART2_IRQn, 0x01 );
				NVIC_EnableIRQ( USART2_IRQn );
			}
			else if ( usartNum == USART_NUM::USART_3 )
			{
				NVIC_SetPriority( USART3_IRQn, 0x01 );
				NVIC_EnableIRQ( USART3_IRQn );
			}
			else if ( usartNum == USART_NUM::USART_6 )
			{
				NVIC_SetPriority( USART6_IRQn, 0x01 );
				NVIC_EnableIRQ( USART6_IRQn );
			}
		}
		else // not using reciever
		{
			// disable reciever interrupt
			usart->CR1 &= ~(USART_CR1_RXNEIE);
		}

		// clear any set flags
		usart->ICR = 0b11111111;

		// enable usart
		usart->CR1 |= USART_CR1_UE;

		// clear any set flags
		usart->ICR = 0b11111111;
	}
}

void LLPD::usart_transmit (const USART_NUM& usartNum, uint16_t data)
{
	USART_TypeDef* usart = nullptr;
	uint16_t* usartWordLenMask = nullptr;

	if ( usartNum == USART_NUM::USART_1 )
	{
		usart = USART1;
		usartWordLenMask = &usart1WordLenMask;
	}
	else if ( usartNum == USART_NUM::USART_2 )
	{
		usart = USART2;
		usartWordLenMask = &usart2WordLenMask;
	}
	else if ( usartNum == USART_NUM::USART_3 )
	{
		usart = USART3;
		usartWordLenMask = &usart3WordLenMask;
	}
	else if ( usartNum == USART_NUM::USART_6 )
	{
		usart = USART6;
		usartWordLenMask = &usart6WordLenMask;
	}

	if ( usart && usartWordLenMask )
	{
		// mask the data so it correctly fits in buffer
		uint16_t dataMasked = data & *usartWordLenMask;

		// wait for transmit data fifo to have space
		while ( ! (usart->ISR & USART_ISR_TXE_TXFNF) ) {}

		// put data in the transmit data register
		usart->TDR = dataMasked;
	}
}

uint16_t LLPD::usart_receive (const USART_NUM& usartNum)
{
	USART_TypeDef* usart = nullptr;

	if ( usartNum == USART_NUM::USART_1 )
	{
		usart = USART1;
	}
	else if ( usartNum == USART_NUM::USART_2 )
	{
		usart = USART2;
	}
	else if ( usartNum == USART_NUM::USART_3 )
	{
		usart = USART3;
	}
	else if ( usartNum == USART_NUM::USART_6 )
	{
		usart = USART6;
	}

	if ( usart )
	{
		// if there's been an overrun, clear the overrun error flag
		if ( usart->ISR & USART_ISR_ORE )
		{
			usart->ICR |= USART_ICR_ORECF;
		}

		// wait until there is data to read in the read data register
		while ( ! (usart->ISR & USART_ISR_RXNE_RXFNE) ) {}

		// read data from register
		uint16_t data = usart->RDR;

		return data;
	}

	return 0;
}

void LLPD::usart_log (const USART_NUM& usartNum, const char* cStr)
{
	while ( *cStr )
	{
		LLPD::usart_transmit( usartNum, *cStr );
		cStr++;
	}

	LLPD::usart_transmit( usartNum, '\n' );
}

void LLPD::usart_log_int (const USART_NUM& usartNum, const char* cStr, int val)
{
	while ( *cStr )
	{
		LLPD::usart_transmit( usartNum, *cStr );
		cStr++;
	}

	char intStrBuf[ (sizeof(int) * 8) + 1 ];
	itoa( val, intStrBuf, 10 );

	char* character = &intStrBuf[0];
	while ( *character )
	{
		LLPD::usart_transmit( usartNum, *character );
		character++;
	}

	LLPD::usart_transmit( usartNum, '\n' );
}

void LLPD::usart_log_float (const USART_NUM& usartNum, const char* cStr, float val)
{
	while ( *cStr )
	{
		LLPD::usart_transmit( usartNum, *cStr );
		cStr++;
	}

	char floatIntStrBuf[ (sizeof(int) * 8) + 1 ];
	char floatDecStrBuf[ (sizeof(int) * 8) + 1 ];
	int floatIntPart = static_cast<int>( val );
	int floatDecPart = static_cast<int>( (val - floatIntPart) * 1000.0f ); // currently 3 point precision
	itoa( floatIntPart, floatIntStrBuf, 10 );
	itoa( floatDecPart, floatDecStrBuf, 10 );

	char* character = &floatIntStrBuf[0];
	while ( *character )
	{
		LLPD::usart_transmit( usartNum, *character );
		character++;
	}

	LLPD::usart_transmit( usartNum, '.' );

	character = &floatDecStrBuf[0];
	while ( *character )
	{
		LLPD::usart_transmit( usartNum, *character );
		character++;
	}

	LLPD::usart_transmit( usartNum, '\n' );
}
