#include "LLPD.hpp"

void LLPD::dac_init (bool useVoltageBuffer)
{
	// enable clock to dac
	RCC->APB1LENR |= RCC_APB1LENR_DAC12EN;

	// reset dac
	RCC->APB1LRSTR |= RCC_APB1LRSTR_DAC12RST;
	RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_DAC12RST);

	// set pins a4 and a5 to analog mode
	LLPD::gpio_analog_setup( GPIO_PORT::A, GPIO_PIN::PIN_4 );
	LLPD::gpio_analog_setup( GPIO_PORT::A, GPIO_PIN::PIN_5 );

	// disable both dac channels
	DAC1->CR &= ~( DAC_CR_EN1 | DAC_CR_EN2 );

	// enable voltage buffer
	DAC1->MCR &= ~( DAC_MCR_MODE1 | DAC_MCR_MODE2 );

	// disable voltage buffer for both channels if requested
	if ( ! useVoltageBuffer )
	{
		DAC1->MCR |= DAC_MCR_MODE1_1 | DAC_MCR_MODE2_1;
	}

	// enable both dac channels
	DAC1->CR |= DAC_CR_EN1 | DAC_CR_EN2;
}

void LLPD::dac_init_use_dma (bool useVoltageBuffer, uint32_t* buffer1, uint32_t* buffer2, unsigned int numSamplesPerBuf)
{
	// enable clock to dac
	RCC->APB1LENR |= RCC_APB1LENR_DAC12EN;

	// reset dac
	RCC->APB1LRSTR |= RCC_APB1LRSTR_DAC12RST;
	RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_DAC12RST);

	// set pins a4 and a5 to analog mode
	LLPD::gpio_analog_setup( GPIO_PORT::A, GPIO_PIN::PIN_4 );
	LLPD::gpio_analog_setup( GPIO_PORT::A, GPIO_PIN::PIN_5 );

	// disable both dac channels
	DAC1->CR &= ~( DAC_CR_EN1 | DAC_CR_EN2 );

	// set trigger as tim6 for transfers
	DAC1->CR |= ( 5 << DAC_CR_TSEL1_Pos ); // only selecting trigger for channel one since only one dma request is needed

	// enable syscfg clock for DMA
	RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;

	// enable dma1 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	// ensure dma stream is disabled and control register is reset
	DMA1_Stream1->CR = 0;
	while ( DMA1_Stream1->CR & DMA_SxCR_EN ) {}

	// set peripheral address for stream
	DMA1_Stream1->PAR = (uint64_t) &(DAC1->DHR12RD);

	// set the stream to handle bufferable transfers
	DMA1_Stream1->CR |= DMA_SxCR_TRBUFF;

	// set double buffer mode and circular mode
	DMA1_Stream1->CR |= DMA_SxCR_DBM;
	DMA1_Stream1->CR |= DMA_SxCR_CIRC;

	// set data size to 32 bits (since we're sending 16 bit right and left values packed)
	DMA1_Stream1->CR &= ~(DMA_SxCR_PSIZE);
	DMA1_Stream1->CR |= DMA_SxCR_PSIZE_1;
	DMA1_Stream1->CR &= ~(DMA_SxCR_MSIZE);
	DMA1_Stream1->CR |= DMA_SxCR_MSIZE_1;

	// set memory increment mode for memory region
	DMA1_Stream1->CR |= DMA_SxCR_MINC;

	// set the memory addresses for where the dac data will be coming from
	DMA1_Stream1->M0AR = (uint64_t) buffer1;
	DMA1_Stream1->M1AR = (uint64_t) buffer2;

	// configure the number of data to be transferred
	DMA1_Stream1->NDTR = numSamplesPerBuf;

	// set data transfer direction from memory to peripheral
	DMA1_Stream1->CR &= ~(DMA_SxCR_DIR);
	DMA1_Stream1->CR |= DMA_SxCR_DIR_0;

	// configure stream priority to very high
	DMA1_Stream1->CR |= DMA_SxCR_PL;

	// set up dma request input
	DMAMUX1_Channel1->CCR = 67; 	// 67 is the dma request mux input for dac_ch1_dma

	// set direct mode
	DMA1_Stream1->FCR &= ~(DMA_SxFCR_DMDIS);

	// clear flags before enabling
	DMA1->LIFCR |= DMA_LIFCR_CFEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTCIF1;

	// enable stream
	DMA1_Stream1->CR |= DMA_SxCR_EN;

	// setup dma for send requests for channel 1 (only need channel 1 request)
	DAC1->CR |= DAC_CR_DMAEN1;

	// enable voltage buffer
	DAC1->MCR &= ~( DAC_MCR_MODE1 | DAC_MCR_MODE2 );

	// disable voltage buffer for both channels if requested
	if ( ! useVoltageBuffer )
	{
		DAC1->MCR |= DAC_MCR_MODE1_1 | DAC_MCR_MODE2_1;
	}

	// enable both dac channels
	DAC1->CR |= DAC_CR_EN1 | DAC_CR_EN2;

	// enable trigger for channel (only need channel 1 trigger)
	DAC1->CR |= DAC_CR_TEN1;
}

void LLPD::dac_send (uint16_t ch1Data, uint16_t ch2Data)
{
	// put data in data registers and ensure only 12 bits are used
	DAC1->DHR12RD = ( (ch1Data & 0b0000111111111111) | ((ch2Data & 0b0000111111111111) << 16) );
}

bool LLPD::dac_dma_using_buffer1()
{
	return DMA1_Stream1->CR & DMA_SxCR_CT;
}

void LLPD::dac_dma_stop()
{
	// ensure dma stream is disabled and control register is reset
	DMA1_Stream1->CR = 0;
	while ( DMA1_Stream1->CR & DMA_SxCR_EN ) {}
}
