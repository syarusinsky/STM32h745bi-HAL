#ifndef LLPD_H
#define LLPD_H

#include <stdint.h>

#include "stm32h745xx.h"

enum class GPIO_PORT
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H
};

enum class GPIO_PIN
{
	PIN_0  = 0,
	PIN_1  = 1,
	PIN_2  = 2,
	PIN_3  = 3,
	PIN_4  = 4,
	PIN_5  = 5,
	PIN_6  = 6,
	PIN_7  = 7,
	PIN_8  = 8,
	PIN_9  = 9,
	PIN_10 = 10,
	PIN_11 = 11,
	PIN_12 = 12,
	PIN_13 = 13,
	PIN_14 = 14,
	PIN_15 = 15,
};

enum class GPIO_OUTPUT_TYPE
{
	PUSH_PULL,
	OPEN_DRAIN
};

enum class GPIO_OUTPUT_SPEED
{
	LOW,
	MEDIUM,
	HIGH,
	VERY_HIGH
};

enum class GPIO_PUPD
{
	NONE,
	PULL_UP,
	PULL_DOWN
};

enum class ADC_NUM
{
	ADC_1_2,
	ADC_3
};

enum class ADC_CYCLES_PER_SAMPLE
{
	CPS_1p5, 	// 1.5
	CPS_2p5, 	// 2.5
	CPS_8p5, 	// 8.5
	CPS_16p5, 	// 16.5
	CPS_32p5, 	// 32.5
	CPS_64p5, 	// 64.5
	CPS_387p5, 	// 387.5
	CPS_810p5, 	// 810.5
};

enum class ADC_CHANNEL
{
	CHAN_0,
	CHAN_1,
	CHAN_2,
	CHAN_3,
	CHAN_4,
	CHAN_5,
	CHAN_6,
	CHAN_7,
	CHAN_8,
	CHAN_9,
	CHAN_10,
	CHAN_11,
	CHAN_12,
	CHAN_13,
	CHAN_14,
	CHAN_15,
	CHAN_16,
	CHAN_17,
	CHAN_18,
	CHAN_19,
	CHAN_INVALID
};

enum class SPI_NUM
{
	SPI_1,
	SPI_2,
	SPI_3,
	SPI_4,
	SPI_5,
	SPI_6
};

enum class SPI_BAUD_RATE
{
	SYSCLK_DIV_BY_2,
	SYSCLK_DIV_BY_4,
	SYSCLK_DIV_BY_8,
	SYSCLK_DIV_BY_16,
	SYSCLK_DIV_BY_32,
	SYSCLK_DIV_BY_64,
	SYSCLK_DIV_BY_128,
	SYSCLK_DIV_BY_256
};

enum class SPI_CLK_POL
{
	LOW_IDLE,
	HIGH_IDLE
};

enum class SPI_CLK_PHASE
{
	FIRST,
	SECOND
};

enum class SPI_DUPLEX
{
	FULL,
	HALF
};

enum class SPI_FRAME_FORMAT
{
	MSB_FIRST,
	LSB_FIRST
};

enum class SPI_DATA_SIZE
{
	BITS_4,
	BITS_5,
	BITS_6,
	BITS_7,
	BITS_8,
	BITS_9,
	BITS_10,
	BITS_11,
	BITS_12,
	BITS_13,
	BITS_14,
	BITS_15,
	BITS_16
};

enum class I2C_NUM
{
	I2C_1,
	I2C_2,
	I2C_3,
	I2C_4
};

enum class I2C_ADDR_MODE
{
	BITS_7,
	BITS_10
};

enum class USART_NUM
{
	USART_1,
	USART_2,
	USART_3,
	USART_6
};

enum class USART_WORD_LENGTH
{
	BITS_7,
	BITS_8,
	BITS_9
};

enum class USART_PARITY
{
	ODD,
	EVEN,
	NONE
};

enum class USART_CONF
{
	TX_ONLY,
	RX_ONLY,  // Recieve interrupt will be enabled
	TX_AND_RX // Recieve interrupt will be enabled
};

enum class USART_STOP_BITS
{
	BITS_1,
	BITS_2
};

enum class OPAMP_NUM
{
	OPAMP_1,
	OPAMP_2
};

constexpr unsigned int D3_SRAM_TIM6_OFFSET_IN_BYTES = sizeof(float) * 3 + sizeof(uint32_t);
constexpr unsigned int D3_SRAM_ADC_OFFSET_IN_BYTES = D3_SRAM_TIM6_OFFSET_IN_BYTES + ( sizeof(uint32_t) * 32 ) + ( sizeof(ADC_CHANNEL) * 32 );
constexpr unsigned int D3_SRAM_UNUSED_OFFSET_IN_BYTES = D3_SRAM_TIM6_OFFSET_IN_BYTES + D3_SRAM_ADC_OFFSET_IN_BYTES;

class LLPD
{
	public:
		// RCC
		static void rcc_clock_start_max_cpu1 (const unsigned int pll1qPresc = 8); 	// starts M7 core at 480 MHz using PLL and LDO
												// (needs to be used with below function)
		static void rcc_clock_start_max_cpu2(); // starts M4 core at 240 MHx using PLL and LDO (needs to be used with above function)
		static void rcc_start_pll2 (const unsigned int pll2qPresc = 20); // starts the second PLL

		// GPIO
		static void gpio_enable_clock (const GPIO_PORT& port);
		static void gpio_digital_input_setup (const GPIO_PORT& port, const GPIO_PIN& pin, const GPIO_PUPD& pupd,
						bool alternateFunc = false);
		static void gpio_analog_setup (const GPIO_PORT& port, const GPIO_PIN& pin);
		static void gpio_output_setup (const GPIO_PORT& port, const GPIO_PIN& pin, const GPIO_PUPD& pupd,
						const GPIO_OUTPUT_TYPE& type, const GPIO_OUTPUT_SPEED& speed,
						bool alternateFunc = false);
		static bool gpio_input_get (const GPIO_PORT& port, const GPIO_PIN& pin);
		static void gpio_output_set (const GPIO_PORT& port, const GPIO_PIN& pin, bool set);
		static void gpio_test();

		// ADC
		// initialization needs to take place after counter is started for tim6, since it uses delay function
		// adc12 uses adc1 channels
		// adc values and order are stored on D3 sram, so if you plan on using that account for the sizeof(uint32_t) * 32
		// plus sizeof(ADC_CHANNEL) * 32 bytes it takes up (offset value found above)
		// TODO we really need to specify cyclesPerSample separately for fast and slow channels...
		static void adc_init (const ADC_NUM& adcNum, const ADC_CYCLES_PER_SAMPLE& cyclesPerSample);
		static void adc_set_channel_order (const ADC_NUM& adcNum, uint8_t numChannels, const ADC_CHANNEL& channel...);
		static void adc_perform_conversion_sequence (const ADC_NUM& adcNum);
		static uint16_t adc_get_channel_value (const ADC_NUM& adcNum, const ADC_CHANNEL& adcChannel);

		// DAC dac1( vout1 = a4, vout2 = a5 )
		static void dac_init (bool useVoltageBuffer); // not using dma
		static void dac_init_use_dma (bool useVoltageBuffer, uint32_t* buffer1, uint32_t* buffer2, unsigned int numSamplesPerBuf); // can't use DTCM memory for buffers
		static void dac_send (uint16_t ch1Data, uint16_t ch2Data); // only for use if not using DMA
		static bool dac_dma_using_buffer1();
		static void dac_dma_stop();

		// TIM6
		// tim6 stores 3 internal variables for delay functions across cores in D3 sram, so if you plan on using that account
		// for the sizeof(float) * 3 bytes it takes up (offset value found above)
		static void tim6_counter_setup (uint32_t prescalerDivisor, uint32_t cyclesPerInterrupt, uint32_t interruptRate);
		static void tim6_counter_enable_interrupts();
		static void tim6_counter_disable_interrupts();
		static void tim6_counter_start();
		static void tim6_counter_stop();
		static void tim6_counter_clear_interrupt_flag();
		static void tim6_delay (uint32_t microseconds); // delay function may not be 100% accurate
		static bool tim6_isr_handle_delay(); 	// To use delay functions, this function needs to be in the tim6 isr.
							// It will return true if a delay is not finished, or false if it is.

		// SPI spi1( sck =  a5, miso =  a6, mosi =  a7 )
		//     spi2( sck = b13, miso = b14, mosi = b15 )
		//     spi3( sck = c10, miso = c11, mosi = c12 )
		//     spi4( sck = e12, miso = e13, mosi = e14 )
		//     spi5( sck =  f7, miso =  f8, mosi =  f9 )
		//     spi6( sck = g13, miso = g12, mosi = g14 )
		// each spi uses its reset clock source
		static void spi_master_init (const SPI_NUM& spiNum, const SPI_BAUD_RATE& baudRate, const SPI_CLK_POL& pol,
						const SPI_CLK_PHASE& phase, const SPI_DUPLEX& duplex,
						const SPI_FRAME_FORMAT& frameFormat, const SPI_DATA_SIZE& dataSize);
		static void spi_master_change_baud_rate (const SPI_NUM& spiNum, const SPI_BAUD_RATE& baudRate);
		static uint16_t spi_master_send_and_recieve (const SPI_NUM& spiNum, uint8_t data);

		// I2C i2c1( sda = b7,  scl = b6  )
		//     i2c2( sda = b11, scl = b10 )
		//     i2c3( sda = c9,  scl = a8  )
		//     i2c4( sda = f15, scl = f14 )
		// each i2c uses its reset clock source
		static void i2c_master_setup (const I2C_NUM& i2cNum, uint32_t timingRegVal);
		static void i2c_master_set_slave_address (const I2C_NUM& i2cNum, const I2C_ADDR_MODE& addrMode, uint16_t address);
		static void i2c_master_write (const I2C_NUM& i2cNum, bool setStopCondition, uint8_t numBytes, uint8_t bytes...);
		static void i2c_master_read (const I2C_NUM& i2cNum, bool setStopCondition, uint8_t numBytes, uint8_t* bytes...);
		static void i2c_master_read_into_array (const I2C_NUM& i2cNum, bool setStopCondition, uint8_t numBytes, uint8_t* arr);

		// USART usart1( tx = b6,  rx = b7  )
		//       usart2( tx = d5,  rx = d6  )
		//       usart3( tx = c10, rx = c11 )
		//       usart6( tx = g14, rx = g9  )
		static void usart_init (const USART_NUM& usartNum, const USART_WORD_LENGTH& wordLen, const USART_PARITY& parity,
					const USART_CONF& conf, const USART_STOP_BITS& stopBits, const unsigned int periphClockFreq,
					const unsigned int baudRate);
		static void usart_transmit (const USART_NUM& usartNum, uint16_t data);
		static uint16_t usart_receive (const USART_NUM& usartNum);
		static void usart_log (const USART_NUM& usartNum, const char* cStr); // needs to be proper c string with terminator
		static void usart_log_int (const USART_NUM& usartNum, const char* cStr, int val);
		static void usart_log_float (const USART_NUM& usartNum, const char* cStr, float val);

		// Op Amp opamp1( v+ = b0, v- = c5, vout = c4 )
		//        opamp2( v+ = e9, v- = e8, vout = e7 )
		static void opamp_init (const OPAMP_NUM& opAmpNum);

		// HSEM
		static bool hsem_try_take (unsigned int semNum);
		static void hsem_release (unsigned int semNum);

	private:
		static inline void setup_alt_func_pin (GPIO_TypeDef* gpioPtr, int pinNum, const int afValue);
};

#endif // LLPD_H
