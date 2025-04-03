#include "LLPD.hpp"

#include <cstdarg>
#include <string.h>

// these arrays are used to hold the values of each channel after a conversion sequence
static uint32_t* adc12ChannelValues = (uint32_t*) ( ((uint8_t*) D3_SRAM_BASE) + D3_SRAM_TIM6_OFFSET_IN_BYTES );
static uint32_t* adc3ChannelValues = adc12ChannelValues + 16;
static uint8_t  adc12NumChansInSeq = 0;
static uint8_t  adc3NumChansInSeq = 0;

// these arrays are used to hold the mapping of the channel order to channel number
static ADC_CHANNEL* adc12ChannelOrder = (ADC_CHANNEL*) ( adc3ChannelValues + 16 );
static ADC_CHANNEL* adc3ChannelOrder = adc12ChannelOrder + 16;

static uint8_t adcChannelToNum (const ADC_CHANNEL& channel)
{
	uint8_t channelNum = 0;

	switch ( channel )
	{
		case ADC_CHANNEL::CHAN_0:
			channelNum = 0;
			break;
		case ADC_CHANNEL::CHAN_1:
			channelNum = 1;
			break;
		case ADC_CHANNEL::CHAN_2:
			channelNum = 2;
			break;
		case ADC_CHANNEL::CHAN_3:
			channelNum = 3;
			break;
		case ADC_CHANNEL::CHAN_4:
			channelNum = 4;
			break;
		case ADC_CHANNEL::CHAN_5:
			channelNum = 5;
			break;
		case ADC_CHANNEL::CHAN_6:
			channelNum = 6;
			break;
		case ADC_CHANNEL::CHAN_7:
			channelNum = 7;
			break;
		case ADC_CHANNEL::CHAN_8:
			channelNum = 8;
			break;
		case ADC_CHANNEL::CHAN_9:
			channelNum = 9;
			break;
		case ADC_CHANNEL::CHAN_10:
			channelNum = 10;
			break;
		case ADC_CHANNEL::CHAN_11:
			channelNum = 11;
			break;
		case ADC_CHANNEL::CHAN_12:
			channelNum = 12;
			break;
		case ADC_CHANNEL::CHAN_13:
			channelNum = 13;
			break;
		case ADC_CHANNEL::CHAN_14:
			channelNum = 14;
			break;
		case ADC_CHANNEL::CHAN_15:
			channelNum = 15;
			break;
		case ADC_CHANNEL::CHAN_16:
			channelNum = 16;
			break;
		case ADC_CHANNEL::CHAN_17:
			channelNum = 17;
			break;
		case ADC_CHANNEL::CHAN_18:
			channelNum = 18;
			break;
		case ADC_CHANNEL::CHAN_19:
			channelNum = 19;
			break;
		default:
			break;
	}

	return channelNum;
}

static uint8_t orderNumToPos (uint8_t orderNum)
{
	uint8_t position = ADC_SQR1_SQ1_Pos;

	if ( orderNum >= 1 && orderNum <= 16 )
	{
		switch ( orderNum )
		{
			case 1:
				position = ADC_SQR1_SQ1_Pos;
				break;
			case 2:
				position = ADC_SQR1_SQ2_Pos;
				break;
			case 3:
				position = ADC_SQR1_SQ3_Pos;
				break;
			case 4:
				position = ADC_SQR1_SQ4_Pos;
				break;
			case 5:
				position = ADC_SQR2_SQ5_Pos;
				break;
			case 6:
				position = ADC_SQR2_SQ6_Pos;
				break;
			case 7:
				position = ADC_SQR2_SQ7_Pos;
				break;
			case 8:
				position = ADC_SQR2_SQ8_Pos;
				break;
			case 9:
				position = ADC_SQR2_SQ9_Pos;
				break;
			case 10:
				position = ADC_SQR3_SQ10_Pos;
				break;
			case 11:
				position = ADC_SQR3_SQ11_Pos;
				break;
			case 12:
				position = ADC_SQR3_SQ12_Pos;
				break;
			case 13:
				position = ADC_SQR3_SQ13_Pos;
				break;
			case 14:
				position = ADC_SQR3_SQ14_Pos;
				break;
			case 15:
				position = ADC_SQR4_SQ15_Pos;
				break;
			case 16:
				position = ADC_SQR4_SQ16_Pos;
				break;
			default:
				break;
		}
	}

	return position;
}

static uint32_t orderNumToMask (uint8_t orderNum)
{
	uint32_t mask = ADC_SQR1_SQ1;

	if ( orderNum >= 1 && orderNum <= 16 )
	{
		switch ( orderNum )
		{
			case 1:
				mask = ADC_SQR1_SQ1;
				break;
			case 2:
				mask = ADC_SQR1_SQ2;
				break;
			case 3:
				mask = ADC_SQR1_SQ3;
				break;
			case 4:
				mask = ADC_SQR1_SQ4;
				break;
			case 5:
				mask = ADC_SQR2_SQ5;
				break;
			case 6:
				mask = ADC_SQR2_SQ6;
				break;
			case 7:
				mask = ADC_SQR2_SQ7;
				break;
			case 8:
				mask = ADC_SQR2_SQ8;
				break;
			case 9:
				mask = ADC_SQR2_SQ9;
				break;
			case 10:
				mask = ADC_SQR3_SQ10;
				break;
			case 11:
				mask = ADC_SQR3_SQ11;
				break;
			case 12:
				mask = ADC_SQR3_SQ12;
				break;
			case 13:
				mask = ADC_SQR3_SQ13;
				break;
			case 14:
				mask = ADC_SQR3_SQ14;
				break;
			case 15:
				mask = ADC_SQR4_SQ15;
				break;
			case 16:
				mask = ADC_SQR4_SQ16;
				break;
			default:
				break;
		}
	}

	return mask;
}

// TODO add setting cyclesPerSample for fast and slow channels separately
void LLPD::adc_init (const ADC_NUM& adcNum, const ADC_CYCLES_PER_SAMPLE& cyclesPerSample)
{
	// reset adc registers
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		RCC->AHB1RSTR |= RCC_AHB1RSTR_ADC12RST;
		RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_ADC12RST);
	}
	else // ADC_NUM::ADC_3
	{
		RCC->AHB4RSTR |= RCC_AHB4RSTR_ADC3RST;
		RCC->AHB4RSTR &= ~(RCC_AHB4RSTR_ADC3RST);
	}

	// enable clock to adc
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;

		// get out of deep-power-down state
		ADC1->CR &= ~(ADC_CR_DEEPPWD);
	}
	else // ADC_NUM::ADC_3
	{
		RCC->AHB4ENR |= RCC_AHB4ENR_ADC3EN;

		// get out of deep-power-down state
		ADC3->CR &= ~(ADC_CR_DEEPPWD);
	}

	// use per_ck (hsi) as kernel clock for adc
	RCC->D3CCIPR &= ~(RCC_D3CCIPR_ADCSEL);
	RCC->D3CCIPR |= RCC_D3CCIPR_ADCSEL_1;

	// set clock to use synchronous mode with sys_ck / 2 (since HPRE) / 2 / 4, for a clock speed of 30MHz
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC12_COMMON->CCR |= ADC_CCR_CKMODE;

		// setting boost for 30MHz clock
		ADC1->CR |= ADC_CR_BOOST;
	}
	else // ADC_NUM::ADC_3
	{
		ADC3_COMMON->CCR |= ADC_CCR_CKMODE;

		// setting boost for 30MHz clock
		ADC3->CR |= ADC_CR_BOOST;
	}

	// set adcs to combined simultaneous regular and injected mode
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC12_COMMON->CCR &= ~(ADC_CCR_DUAL);
		ADC12_COMMON->CCR |= ADC_CCR_DUAL_0;
	}
	else // ADC_NUM::ADC_3
	{
		ADC3_COMMON->CCR &= ~(ADC_CCR_DUAL);
		ADC3_COMMON->CCR |= ADC_CCR_DUAL_0;
	}

	// setup voltage regulator
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC1->CR &= ~(ADC_CR_ADVREGEN);
		ADC1->CR |= ADC_CR_ADVREGEN;
	}
	else // ADC_NUM::ADC_3
	{
		ADC3->CR &= ~(ADC_CR_ADVREGEN);
		ADC3->CR |= ADC_CR_ADVREGEN;
	}

	// actual startup time for the regulator is 10us, but might as well give it 100 -_(o-o)_-
	LLPD::tim6_delay( 100 );

	// ensure adc is disabled, deep-power-down is disabled, adc voltage regulator is enabled and ready
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		while ( ADC1->CR & ADC_CR_ADEN ) {}
		while ( ADC1->CR & ADC_CR_DEEPPWD ) {}
		while ( ! (ADC1->CR & ADC_CR_ADVREGEN) ) {}
		while ( ! (ADC1->ISR & (1 << 12)) ) {} // for whatever reason ldordy bit isn't in the struct?
	}
	else // ADC_NUM::ADC_3
	{
		while ( ADC3->CR & ADC_CR_ADEN ) {}
		while ( ADC3->CR & ADC_CR_DEEPPWD ) {}
		while ( ! (ADC3->CR & ADC_CR_ADVREGEN) ) {}
		while ( ! (ADC3->ISR & (1 << 12)) ) {} // for whatever reason ldordy bit isn't in the struct?
	}

	// start adc calibration for single-ended mode with linearity calibration
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC1->CR |= ADC_CR_ADCALLIN;
		ADC1->CR &= ~(ADC_CR_ADCALDIF);
		ADC1->CR |= ADC_CR_ADCAL;
	}
	else // ADC_NUM::ADC_3
	{
		ADC3->CR |= ADC_CR_ADCALLIN;
		ADC3->CR &= ~(ADC_CR_ADCALDIF);
		ADC3->CR |= ADC_CR_ADCAL;
	}

	// wait for adc calibration to complete
	while ( (ADC1->CR & ADC_CR_ADCAL) || (ADC3->CR & ADC_CR_ADCAL) ) {}

	// enable adc
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC1->CR |= ADC_CR_ADEN;
	}
	else // ADC_NUM::ADC_3
	{
		ADC3->CR |= ADC_CR_ADEN;
	}

	// wait until adc is ready
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		while ( ! (ADC1->ISR & ADC_ISR_ADRDY) ) {}
	}
	else // ADC_NUM::ADC_3
	{
		while ( ! (ADC3->ISR & ADC_ISR_ADRDY) ) {}
	}

	// set cycles per adc sample
	uint8_t clkRegVal = 0;

	switch ( cyclesPerSample )
	{
		case ADC_CYCLES_PER_SAMPLE::CPS_1p5:
			clkRegVal = 0b000;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_2p5:
			clkRegVal = 0b001;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_8p5:
			clkRegVal = 0b010;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_16p5:
			clkRegVal = 0b011;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_32p5:
			clkRegVal = 0b100;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_64p5:
			clkRegVal = 0b101;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_387p5:
			clkRegVal = 0b110;
			break;
		case ADC_CYCLES_PER_SAMPLE::CPS_810p5:
			clkRegVal = 0b111;
			break;
	}

	// set each timing clock in sample time registers
	uint8_t spacing = 3; // 3 bits per entry
	uint8_t offset = 10; // SMPR2 begins at channel 10
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC1->SMPR1 = 	( clkRegVal << (spacing * 0) ) |
				( clkRegVal << (spacing * 1) ) |
				( clkRegVal << (spacing * 2) ) |
				( clkRegVal << (spacing * 3) ) |
				( clkRegVal << (spacing * 4) ) |
				( clkRegVal << (spacing * 5) ) |
				( clkRegVal << (spacing * 6) ) |
				( clkRegVal << (spacing * 7) ) |
				( clkRegVal << (spacing * 8) ) |
				( clkRegVal << (spacing * 9) );
		ADC1->SMPR2 = 	( clkRegVal << (spacing * (10 - offset)) ) |
				( clkRegVal << (spacing * (11 - offset)) ) |
				( clkRegVal << (spacing * (12 - offset)) ) |
				( clkRegVal << (spacing * (13 - offset)) ) |
				( clkRegVal << (spacing * (14 - offset)) ) |
				( clkRegVal << (spacing * (15 - offset)) ) |
				( clkRegVal << (spacing * (16 - offset)) ) |
				( clkRegVal << (spacing * (17 - offset)) ) |
				( clkRegVal << (spacing * (18 - offset)) ) |
				( clkRegVal << (spacing * (19 - offset)) );
	}
	else // ADC_NUM::ADC_3
	{
		ADC3->SMPR1 = 	( clkRegVal << (spacing * 0) ) |
				( clkRegVal << (spacing * 1) ) |
				( clkRegVal << (spacing * 2) ) |
				( clkRegVal << (spacing * 3) ) |
				( clkRegVal << (spacing * 4) ) |
				( clkRegVal << (spacing * 5) ) |
				( clkRegVal << (spacing * 6) ) |
				( clkRegVal << (spacing * 7) ) |
				( clkRegVal << (spacing * 8) ) |
				( clkRegVal << (spacing * 9) );
		ADC3->SMPR2 = 	( clkRegVal << (spacing * (10 - offset)) ) |
				( clkRegVal << (spacing * (11 - offset)) ) |
				( clkRegVal << (spacing * (12 - offset)) ) |
				( clkRegVal << (spacing * (13 - offset)) ) |
				( clkRegVal << (spacing * (14 - offset)) ) |
				( clkRegVal << (spacing * (15 - offset)) ) |
				( clkRegVal << (spacing * (16 - offset)) ) |
				( clkRegVal << (spacing * (17 - offset)) ) |
				( clkRegVal << (spacing * (18 - offset)) ) |
				( clkRegVal << (spacing * (19 - offset)) );
	}

	// set to 12-bits
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC1->CFGR &= ~(ADC_CFGR_RES);
		ADC1->CFGR |= ( ADC_CFGR_RES_2 | ADC_CFGR_RES_1 );
	}
	else // ADC_NUM::ADC_3
	{
		ADC3->CFGR &= ~(ADC_CFGR_RES);
		ADC3->CFGR |= ( ADC_CFGR_RES_2 | ADC_CFGR_RES_1 );
	}
}

void LLPD::adc_set_channel_order (const ADC_NUM& adcNum, uint8_t numChannels, const ADC_CHANNEL& channel...)
{
	// ensure valid amount of channels
	if ( numChannels <= 0 || numChannels > 16 )
	{
		return;
	}

	// cache the number of channels so we can use this later
	ADC_CHANNEL* channelOrder = nullptr;
	volatile uint32_t* sqr1 = nullptr;
	volatile uint32_t* sqr2 = nullptr;
	volatile uint32_t* sqr3 = nullptr;
	volatile uint32_t* sqr4 = nullptr;
	volatile uint32_t* pcsel = nullptr;
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		adc12NumChansInSeq = numChannels;
		channelOrder = adc12ChannelOrder;
		sqr1 = &( ADC1->SQR1 );
		sqr2 = &( ADC1->SQR2 );
		sqr3 = &( ADC1->SQR3 );
		sqr4 = &( ADC1->SQR4 );
		pcsel = &( ADC1->PCSEL );
	}
	else // ADC_NUM::ADC_3
	{
		adc3NumChansInSeq = numChannels;
		channelOrder = adc3ChannelOrder;
		sqr1 = &( ADC3->SQR1 );
		sqr2 = &( ADC3->SQR2 );
		sqr3 = &( ADC3->SQR3 );
		sqr4 = &( ADC3->SQR4 );
		pcsel = &( ADC3->PCSEL );
	}

	// reset 'order' array
	for ( uint8_t chanNum = 0; chanNum < 16; chanNum++ )
	{
		channelOrder[chanNum] = ADC_CHANNEL::CHAN_INVALID;
	}

	// set number of channels in sequence
	uint8_t numChannelsOffset = numChannels - 1;
	*sqr1 &= ~(ADC_SQR1_L);
	*sqr1 |= ( 0x0000000F & numChannelsOffset );

	va_list channels;
	va_start( channels, channel );

	// order first channel
	ADC_CHANNEL chanFirst = channel;
	uint8_t chanFirstNum = adcChannelToNum( chanFirst );

	// add channel to preselection
	*pcsel |= (1 << chanFirstNum);

	*sqr1 &= ~(ADC_SQR1_SQ1);
	*sqr1 |= ( chanFirstNum << ADC_SQR1_SQ1_Pos );

	// add first channel to the 'order' array
	channelOrder[0] = chanFirst;

	// oder rest of channels
	for ( uint8_t orderNum = 2; orderNum <= numChannels; orderNum++ )
	{
		ADC_CHANNEL chanEnum = va_arg( channels, ADC_CHANNEL );
		uint32_t mask = orderNumToMask( orderNum );
		uint8_t position = orderNumToPos( orderNum );
		uint8_t chan = adcChannelToNum( chanEnum );

		// add channel to preselection
		*pcsel |= (1 << chan);

		// add channel to the 'order' array
		channelOrder[orderNum - 1] = chanEnum;

		// we need to use different registers depending on the order number
		if ( orderNum < 5 )
		{
			*sqr1 &= ~(mask);
			*sqr1 |= ( chan << position );
		}
		else if ( orderNum < 10 )
		{
			*sqr2 &= ~(mask);
			*sqr2 |= ( chan << position );
		}
		else if ( orderNum < 15 )
		{
			*sqr3 &= ~(mask);
			*sqr3 |= ( chan << position );
		}
		else
		{
			*sqr4 &= ~(mask);
			*sqr4 |= ( chan << position );
		}
	}

	va_end( channels );

	// enable syscfg clock
	RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;

	// set up dma (use dma1 channel 0 for adc12 and bdma channel 0 for adc3)
	ADC_TypeDef* adc = nullptr;
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		// enable dma1 clock
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

		// ensure dma stream is disabled and control register is reset
		DMA1_Stream0->CR = 0;
		while ( DMA1_Stream0->CR & DMA_SxCR_EN ) {}

		adc = ADC1;

		// set peripheral address for stream
		DMA1_Stream0->PAR = (uint64_t) &(adc->DR);

		// set the memory address for where the adc data will be stored
		DMA1_Stream0->M0AR = (uint64_t) adc12ChannelValues;

		// clear memory for adc data
		memset( adc12ChannelValues, 0, 16 * sizeof(*adc12ChannelValues) );

		// configure the number of data to be transferred
		DMA1_Stream0->NDTR = numChannels;

		// configure stream priority to low
		DMA1_Stream0->CR &= ~(DMA_SxCR_PL);

		// set data transfer direction from peripheral to memory
		DMA1_Stream0->CR &= ~(DMA_SxCR_DIR);

		// enable memory incrementing
		DMA1_Stream0->CR |= DMA_SxCR_MINC;

		// set the peripheral and memory data sizes to 32 bits
		DMA1_Stream0->CR &= ~(DMA_SxCR_PSIZE);
		DMA1_Stream0->CR |= DMA_SxCR_PSIZE_1;
		DMA1_Stream0->CR &= ~(DMA_SxCR_MSIZE);
		DMA1_Stream0->CR |= DMA_SxCR_MSIZE_1;

		// set direct mode
		DMA1_Stream0->FCR &= ~(DMA_SxFCR_DMDIS);

		// set up dma request input
		DMAMUX1_Channel0->CCR = 9; // 9 is the dma request mux input for adc1 (as per reference manual)
	}
	else // ADC_NUM::ADC_3
	{
		// enable bdma clock
		RCC->AHB4ENR |= RCC_AHB4ENR_BDMAEN;

		// ensure bdma is disabled and control register is reset
		BDMA_Channel0->CCR = 0;
		while ( BDMA_Channel0->CCR & BDMA_CCR_EN ) {}

		adc = ADC3;

		// set peripheral address for stream
		BDMA_Channel0->CPAR = (uint64_t) &(adc->DR);

		// set the memory address for where the adc data will be stored
		BDMA_Channel0->CM0AR = (uint64_t) adc3ChannelValues;

		// clear memory for adc data
		memset( adc3ChannelValues, 0, 16 * sizeof(*adc3ChannelValues) );

		// configure the number of data to be transferred
		BDMA_Channel0->CNDTR = numChannels;

		// configure channel priority to very high
		BDMA_Channel0->CCR |= BDMA_CCR_PL;

		// set data transfer direction from peripheral to memory
		BDMA_Channel0->CCR &= ~(BDMA_CCR_DIR);

		// enable memory incrementing
		BDMA_Channel0->CCR |= BDMA_CCR_MINC;

		// set the peripheral and memory data sizes to 32 bits
		BDMA_Channel0->CCR &= ~(BDMA_CCR_PSIZE);
		BDMA_Channel0->CCR |= BDMA_CCR_PSIZE_1;
		BDMA_Channel0->CCR &= ~(BDMA_CCR_MSIZE);
		BDMA_Channel0->CCR |= BDMA_CCR_MSIZE_1;

		// set up dma request input
		DMAMUX2_Channel0->CCR = 17; // 17 is the dma request mux input for adc3 (as per reference manual)
	}


	// set up adc to use dma one-shot mode
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		ADC1->CFGR &= ~(ADC_CFGR_DMNGT);
		ADC1->CFGR |= ADC_CFGR_DMNGT_0;
	}
	else // ADC_NUM::ADC_3
	{
		ADC3->CFGR &= ~(ADC_CFGR_DMNGT);
		ADC3->CFGR |= ADC_CFGR_DMNGT_0;
	}
}

void LLPD::adc_perform_conversion_sequence (const ADC_NUM& adcNum)
{
	ADC_TypeDef* adc = nullptr;
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		adc = ADC1;

		// clear flags
		DMA1->LIFCR |= DMA_LIFCR_CTCIF0 | DMA_LIFCR_CTEIF0;

		// configure number of data to be transferred
		DMA1_Stream0->NDTR = adc12NumChansInSeq;

		// enable dma channel
		DMA1_Stream0->CR |= DMA_SxCR_EN;
	}
	else // ADC_NUM::ADC_3
	{
		adc = ADC3;

		// disable dma channel
		BDMA_Channel0->CCR &= ~(BDMA_CCR_EN);

		// clear flags
		BDMA->IFCR |= BDMA_IFCR_CTCIF0 | BDMA_IFCR_CTEIF0;

		// configure number of data to be transferred
		BDMA_Channel0->CNDTR = adc3NumChansInSeq;

		// enable dma channel
		BDMA_Channel0->CCR |= BDMA_CCR_EN;
	}

	// start conversion
	adc->CR |= ADC_CR_ADSTART;

	// wait for the end of sequence to ensure the last transfer was completed
	while ( ! (adc->ISR & ADC_ISR_EOS) ) {}

	// clear end of sequence flag
	adc->ISR |= ADC_ISR_EOS;
}

uint16_t LLPD::adc_get_channel_value (const ADC_NUM& adcNum, const ADC_CHANNEL& channel)
{
	uint32_t* channelValues = nullptr;
	ADC_CHANNEL* channelOrder = nullptr;
	if ( adcNum == ADC_NUM::ADC_1_2 )
	{
		channelValues = adc12ChannelValues;
		channelOrder = adc12ChannelOrder;
	}
	else // ADC_NUM::ADC_3
	{
		channelValues = adc3ChannelValues;
		channelOrder = adc3ChannelOrder;
	}

	for ( uint8_t index = 0; index < 16; index++ )
	{
		if ( channel == channelOrder[index] )
		{
			return channelValues[index];
		}
	}

	return 0;
}
