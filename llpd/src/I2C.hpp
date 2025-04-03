#include "LLPD.hpp"

#include <cstdarg>

static void setI2CRegisters (I2C_TypeDef* i2cPtr, uint32_t timingRegVal, uint32_t resetBits)
{
	// disable i2c peripheral
	i2cPtr->CR1 &= ~(I2C_CR1_PE);

	// set the i2c timing register
	i2cPtr->TIMINGR = (uint32_t)timingRegVal;

	// enable i2c peripheral
	i2cPtr->CR1 |= I2C_CR1_PE;
}

void LLPD::i2c_master_setup (const I2C_NUM& i2cNum, uint32_t timingRegVal)
{
	I2C_TypeDef* i2cPtr = nullptr;
	uint32_t rccI2CEnableBits = 0;
	uint32_t rccI2CResetBits = 0;

	GPIO_TypeDef* gpioPtrScl = nullptr;
	GPIO_TypeDef* gpioPtrSda = nullptr;
	GPIO_PORT gpioPortScl;
	GPIO_PORT gpioPortSda;
	GPIO_PIN  sclPin;
	GPIO_PIN  sdaPin;

	int afValScl = 0;
	int afValSda = 0;
	int afPinScl = 0;
	int afPinSda = 0;

	if ( i2cNum == I2C_NUM::I2C_1 )
	{
		rccI2CEnableBits = RCC_APB1LENR_I2C1EN;
		rccI2CResetBits = RCC_APB1LRSTR_I2C1RST;

		i2cPtr = I2C1;

		gpioPtrScl = GPIOB;
		gpioPtrSda = GPIOB;
		gpioPortScl = GPIO_PORT::B;
		gpioPortSda = GPIO_PORT::B;
		sclPin = GPIO_PIN::PIN_6;
		sdaPin = GPIO_PIN::PIN_7;

		afValScl = 4;
		afValSda = 4;
		afPinScl = 6;
		afPinSda = 7;
	}
	else if ( i2cNum == I2C_NUM::I2C_2 )
	{
		rccI2CEnableBits = RCC_APB1LENR_I2C2EN;
		rccI2CResetBits = RCC_APB1LRSTR_I2C2RST;

		i2cPtr = I2C2;

		gpioPtrScl = GPIOB;
		gpioPtrSda = GPIOB;
		gpioPortScl = GPIO_PORT::B;
		gpioPortSda = GPIO_PORT::B;
		sclPin = GPIO_PIN::PIN_10;
		sdaPin = GPIO_PIN::PIN_11;

		afValScl = 4;
		afValSda = 4;
		afPinScl = 10;
		afPinSda = 11;
	}
	else if ( i2cNum == I2C_NUM::I2C_3 )
	{
		rccI2CEnableBits = RCC_APB1LENR_I2C3EN;
		rccI2CResetBits = RCC_APB1LRSTR_I2C3RST;

		i2cPtr = I2C3;

		gpioPtrScl = GPIOA;
		gpioPtrSda = GPIOC;
		gpioPortScl = GPIO_PORT::A;
		gpioPortSda = GPIO_PORT::C;
		sclPin = GPIO_PIN::PIN_8;
		sdaPin = GPIO_PIN::PIN_9;

		afValScl = 4;
		afValSda = 4;
		afPinScl = 8;
		afPinSda = 9;
	}
	else if ( i2cNum == I2C_NUM::I2C_4 )
	{
		rccI2CEnableBits = RCC_APB4ENR_I2C4EN;
		rccI2CResetBits = RCC_APB4RSTR_I2C4RST;

		i2cPtr = I2C4;

		gpioPtrScl = GPIOF;
		gpioPtrSda = GPIOF;
		gpioPortScl = GPIO_PORT::F;
		gpioPortSda = GPIO_PORT::F;
		sclPin = GPIO_PIN::PIN_14;
		sdaPin = GPIO_PIN::PIN_15;

		afValScl = 4;
		afValSda = 4;
		afPinScl = 14;
		afPinSda = 15;
	}

	// set alternate function registers for gpio
	LLPD::setup_alt_func_pin( gpioPtrScl, afPinScl, afValScl );
	LLPD::setup_alt_func_pin( gpioPtrSda, afPinSda, afValSda );

	// i2c scl
	LLPD::gpio_output_setup( gpioPortScl, sclPin, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::OPEN_DRAIN,
					GPIO_OUTPUT_SPEED::HIGH, true );

	// i2c sda
	LLPD::gpio_output_setup( gpioPortSda, sdaPin, GPIO_PUPD::NONE, GPIO_OUTPUT_TYPE::OPEN_DRAIN,
					GPIO_OUTPUT_SPEED::HIGH, true );

	// enable i2c peripheral clock
	if ( i2cNum == I2C_NUM::I2C_4 )
	{
		RCC->APB4ENR |= rccI2CEnableBits;
	}
	else
	{
		RCC->APB1LENR |= rccI2CEnableBits;
	}

	setI2CRegisters( i2cPtr, timingRegVal, rccI2CResetBits );
}

static I2C_TypeDef* getI2CPointer (const I2C_NUM& i2cNum)
{
	I2C_TypeDef* i2cPtr = nullptr;

	switch ( i2cNum )
	{
		case I2C_NUM::I2C_1:
			i2cPtr = I2C1;

			break;
		case I2C_NUM::I2C_2:
			i2cPtr = I2C2;

			break;
		case I2C_NUM::I2C_3:
			i2cPtr = I2C3;

			break;
		case I2C_NUM::I2C_4:
			i2cPtr = I2C4;

			break;
	}

	return i2cPtr;
}

void LLPD::i2c_master_set_slave_address (const I2C_NUM& i2cNum, const I2C_ADDR_MODE& addrMode, uint16_t address)
{
	I2C_TypeDef* i2cPtr = getI2CPointer( i2cNum );

	if ( addrMode == I2C_ADDR_MODE::BITS_7 )
	{
		// set address mode to 7 bits
		i2cPtr->CR2 &= ~(I2C_CR2_ADD10);

		// mask address to ensure it's 7 bits max
		address = (address & 0b0000000001111111);

		// clear slave address register and set to new address
		i2cPtr->CR2 &= ~(0b000000000000000000000000011111110);
		i2cPtr->CR2 |= (address << 1);
	}
	else if ( addrMode == I2C_ADDR_MODE::BITS_10 )
	{
		// set address mode to 10 bits
		i2cPtr->CR2 |= I2C_CR2_ADD10;

		// mask address to ensure it's 10 bits max
		address = (address & 0b0000001111111111);

		// clear slave address register and set to new address
		i2cPtr->CR2 &= ~(0b000000000000000000000001111111111);
		i2cPtr->CR2 |= address;
	}
}

void LLPD::i2c_master_write (const I2C_NUM& i2cNum, bool setStopCondition, uint8_t numBytes, uint8_t bytes...)
{
	I2C_TypeDef* i2cPtr = getI2CPointer( i2cNum );

	// spinlock while busy
	while ( i2cPtr->ISR & I2C_ISR_BUSY ) {}

	// set write flag
	i2cPtr->CR2 &= ~(I2C_CR2_RD_WRN);

	// set number of bytes to send
	i2cPtr->CR2 &= 0b11111111000000001111111111111111;
	i2cPtr->CR2 |= (numBytes << 16);

	// set start condition
	i2cPtr->CR2 |= I2C_CR2_START;

	// spinlock to ensure start condition and address bits have been sent
	while ( i2cPtr->CR2 & I2C_CR2_START ) {}

	// in case slave is busy
	while ( i2cPtr->ISR & I2C_ISR_NACKF )
	{
		// clear the stopf and nackf flags
		i2cPtr->ICR |= I2C_ICR_STOPCF;
		i2cPtr->ICR |= I2C_ICR_NACKCF;

		// restart the transfer
		i2cPtr->CR2 |= I2C_CR2_START;

		// spinlock to ensure start condition and address bits have been sent
		while ( i2cPtr->CR2 & I2C_CR2_START ) {}
	}

	// transfer bytes
	va_list args;
	va_start( args, bytes );

	// first transfer
	uint8_t data = bytes;
	i2cPtr->TXDR = data;

	// spinlock to ensure first transmission complete
	while ( !(i2cPtr->ISR & I2C_ISR_TXE) ) {}

	// subsequent transfers
	for ( uint8_t byte = 1; byte < numBytes; byte++ )
	{
		uint8_t data = static_cast<uint8_t>( va_arg(args, unsigned int) );
		i2cPtr->TXDR = data;

		// spinlock to ensure last transmission complete
		while ( !(i2cPtr->ISR & I2C_ISR_TXE) ) {}
	}

	va_end( args );

	// spinlock to ensure transfer is complete
	while ( !(i2cPtr->ISR & I2C_ISR_TC) ) {}

	if ( setStopCondition )
	{
		// set stop condition
		i2cPtr->CR2 |= I2C_CR2_STOP;

		// spinlock to ensure stop condition is detected and bus is not busy
		while ( (i2cPtr->CR2 & I2C_CR2_STOP) && (i2cPtr->ISR & I2C_ISR_BUSY) && (i2cPtr->ISR & I2C_ISR_STOPF) ) {}
	}
}

void LLPD::i2c_master_read (const I2C_NUM& i2cNum, bool setStopCondition, uint8_t numBytes, uint8_t* bytes...)
{
	I2C_TypeDef* i2cPtr = getI2CPointer( i2cNum );

	// set read flag
	i2cPtr->CR2 |= I2C_CR2_RD_WRN;

	// set number of bytes to read
	i2cPtr->CR2 &= 0b11111111000000001111111111111111;
	i2cPtr->CR2 |= (numBytes << 16);

	// set start condition
	i2cPtr->CR2 |= I2C_CR2_START;

	// spinlock to ensure start condition address bits have been sent
	while ( i2cPtr->CR2 & I2C_CR2_START ) {}

	// in case slave is busy
	while ( i2cPtr->ISR & I2C_ISR_NACKF )
	{
		// clear the stopf and nackf flags
		i2cPtr->ICR |= I2C_ICR_STOPCF;
		i2cPtr->ICR |= I2C_ICR_NACKCF;

		// restart the transfer
		i2cPtr->CR2 |= I2C_CR2_START;

		// spinlock to ensure start condition and address bits have been sent
		while ( i2cPtr->CR2 & I2C_CR2_START ) {}
	}

	// read bytes
	va_list args;
	va_start( args, bytes );

	// spinlock to ensure recieve data register has something to read
	while ( !(i2cPtr->ISR & I2C_ISR_RXNE) ) {}

	// first read
	uint8_t* data = bytes;
	*data = i2cPtr->RXDR;

	// subsequent reads
	for ( uint8_t byte = 1; byte < numBytes; byte++ )
	{
		// spinlock to ensure recieve data register has something to read
		while ( !(i2cPtr->ISR & I2C_ISR_RXNE) ) {}

		uint8_t* data = va_arg(args, uint8_t*);
		*data = i2cPtr->RXDR;
	}

	va_end( args );

	// spinlock to ensure transfer is complete
	while ( !(i2cPtr->ISR & I2C_ISR_TC) ) {}

	if ( setStopCondition )
	{
		// set stop condition
		i2cPtr->CR2 |= I2C_CR2_STOP;

		// spinlock to ensure stop condition is detected and bus is not busy
		while ( (i2cPtr->CR2 & I2C_CR2_STOP) && (i2cPtr->ISR & I2C_ISR_BUSY) && (i2cPtr->ISR & I2C_ISR_STOPF) ) {}
	}
}

void LLPD::i2c_master_read_into_array (const I2C_NUM& i2cNum, bool setStopCondition, uint8_t numBytes, uint8_t* arr)
{
	I2C_TypeDef* i2cPtr = getI2CPointer( i2cNum );

	// set read flag
	i2cPtr->CR2 |= I2C_CR2_RD_WRN;

	// set number of bytes to read
	i2cPtr->CR2 &= 0b11111111000000001111111111111111;
	i2cPtr->CR2 |= (numBytes << 16);

	// set start condition
	i2cPtr->CR2 |= I2C_CR2_START;

	// spinlock to ensure start condition address bits have been sent
	while ( i2cPtr->CR2 & I2C_CR2_START ) {}

	// in case slave is busy
	while ( i2cPtr->ISR & I2C_ISR_NACKF )
	{
		// clear the stopf and nackf flags
		i2cPtr->ICR |= I2C_ICR_STOPCF;
		i2cPtr->ICR |= I2C_ICR_NACKCF;

		// restart the transfer
		i2cPtr->CR2 |= I2C_CR2_START;

		// spinlock to ensure start condition and address bits have been sent
		while ( i2cPtr->CR2 & I2C_CR2_START ) {}
	}

	// spinlock to ensure recieve data register has something to read
	while ( !(i2cPtr->ISR & I2C_ISR_RXNE) ) {}

	// read data
	for ( uint8_t byte = 0; byte < numBytes; byte++ )
	{
		// spinlock to ensure recieve data register has something to read
		while ( !(i2cPtr->ISR & I2C_ISR_RXNE) ) {}

		arr[byte] = i2cPtr->RXDR;
	}

	// spinlock to ensure transfer is complete
	while ( !(i2cPtr->ISR & I2C_ISR_TC) ) {}

	if ( setStopCondition )
	{
		// set stop condition
		i2cPtr->CR2 |= I2C_CR2_STOP;

		// spinlock to ensure stop condition is detected and bus is not busy
		while ( (i2cPtr->CR2 & I2C_CR2_STOP) && (i2cPtr->ISR & I2C_ISR_BUSY) && (i2cPtr->ISR & I2C_ISR_STOPF) ) {}
	}
}
