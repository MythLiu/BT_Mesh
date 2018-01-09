#include "i2c.h"
#include "em_gpio.h"
#include <math.h>
#include "timer0.h"

void i2c_init(void){
	//i2c is on PC11 and PC 10 on location 16
	I2C_Init_TypeDef i2c_init = I2C_INIT_DEFAULT;
	CMU_ClockEnable(cmuClock_HFPER,true);
	CMU_ClockEnable(cmuClock_I2C0, true);

	//set gpio pin modes
	GPIO_PinModeSet(gpioPortC, 10, gpioModeWiredAnd, 1);//pull up SCL
	GPIO_PinModeSet(gpioPortC, 11, gpioModeWiredAnd, 1);//SDA

	i2c_init.clhr = i2cClockHLRStandard;
	i2c_init.master = true;
	i2c_init.refFreq = 0;
	i2c_init.freq = 5000; //check the datasheet
	i2c_init.enable = false;//

	//route the pins to respective location
	I2C0 -> ROUTELOC0 = I2C_ROUTELOC0_SDALOC_LOC16 |I2C_ROUTELOC0_SCLLOC_LOC14;
	I2C0 -> ROUTEPEN = I2C_ROUTEPEN_SCLPEN | I2C_ROUTEPEN_SDAPEN;

	//init I2C
	I2C_Init(I2C0, &i2c_init);

	//clear buffers in I2C slave
	for (int i=0; i<9; i++){
		GPIO_PinOutClear(gpioPortC, 10);
		GPIO_PinOutSet(gpioPortC, 10);
	}

	//reset the I2C bus
	if(I2C0->STATE & I2C_STATE_BUSY){
		I2C0->CMD = I2C_CMD_ABORT;
	}


//	I2C_IntClear(I2C0, I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_SSTOP);//clear all the pending interrupt
//	I2C_IntEnable(I2C0, I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_SSTOP);//enable all the interrupts
//	NVIC_EnableIRQ(I2C0_IRQn);
}

void i2c_enable(void){
	GPIO_PinOutSet(gpioPortD, 9);
	for(int i = 0; i <100000; i++);
	i2c_init();
	I2C_Enable(I2C0, true);
	//unblockSleepMode(EM1);
}

void i2c_disable(void){
	if(I2C0->STATE & I2C_STATE_BUSY){
		I2C0->CMD = I2C_CMD_ABORT;
	}
	GPIO_PinOutClear(gpioPortD, 9);
	I2C_Enable(I2C0, false);
	//blockSleepMode(EM3);
}


uint8_t i2c_readByte(uint8_t slave_addr,uint8_t mem_addr){
	I2C0->CMD |= I2C_IFC_MSTOP;
	I2C0->CMD |= I2C_IFC_SSTOP;
	I2C0->CMD |= I2C_IFC_TXC;

	uint8_t read_data;
	uint16_t i;
	//signifying write of address (0x40) to the slave
	I2C0->TXDATA = (slave_addr << 1) | 0x00 ;//start with write

	//send the START bit
	I2C0->CMD = I2C_CMD_START;
	//I2C0 -> IFC = I2C_IFC_START;

	//wait for the salve to respond
	while ((I2C0->IF & I2C_IF_ACK) == 0);

	//after ACK has been received, it must be cleared from the IF
	I2C0->IFC = I2C_IFC_ACK;

	// Set register to 0xd0 for ID
	I2C0->TXDATA = mem_addr;

	//I2C0->CMD = I2C_CMD_START;

	//wait for the salve to respond
	while ((I2C0->IF & I2C_IF_ACK) == 0);

	//after ACK has been received, it must be cleared from the IF
	I2C0->IFC = I2C_IFC_ACK;



	I2C0->CMD = I2C_CMD_START;
	//signifying read of address to the slave
	I2C0->TXDATA = (slave_addr << 1) | 0x01 ;//read


	//wait for the salve to respond
	while ((I2C0->IF & I2C_IF_ACK) == 0);

	//after ACK has been received, it must be cleared from the IF
	I2C0->IFC = I2C_IFC_ACK;

	while((I2C0->IF & I2C_IF_RXDATAV) ==0);

	read_data = I2C0->RXDATA;    //receive MSB
    I2C0->IFC = I2C_IFC_ACK;
    I2C0->CMD |= I2C_CMD_ACK;
	I2C0->CMD |= I2C_CMD_STOP;//stop I2c

	for(i=0;i<1000;i++);

	//I2C0->CMD |= I2C_CMD_ABORT;
	//I2C0->CMD |= I2C_CMD_CLEARPC;
	return read_data;
}

void i2c_writeByte(uint8_t slave_addr,uint8_t mem_addr,uint8_t data){
	I2C0->TXDATA = slave_addr << 1;
	//send the START bit
	I2C0->CMD = I2C_CMD_START;
	//wait for the salve to respond
	while ((I2C0->IF & I2C_IF_ACK) == 0);
	//after ACK has been received, it must be cleared from the IF
	I2C0->IFC = I2C_IFC_ACK;

	I2C0->TXDATA =mem_addr;
	//send the START bit
	//I2C0->CMD = I2C_CMD_START;
	//wait for the salve to respond
	while ((I2C0->IF & I2C_IF_ACK) == 0);
	//after ACK has been received, it must be cleared from the IF
	I2C0->IFC = I2C_IFC_ACK;

	I2C0->TXDATA =data;
	//send the START bit
	//I2C0->CMD = I2C_CMD_START;
	//wait for the salve to respond
	while ((I2C0->IF & I2C_IF_ACK) == 0);
	//after ACK has been received, it must be cleared from the IF
	I2C0->IFC = I2C_IFC_ACK;

	//I2C0->CMD = I2C_CMD_NACK;

	I2C0->CMD = I2C_CMD_STOP;//stop I2C
}

void lum_enable()
{
	//I2C_RegisterWrite((TSL2561_Command_register | TSL2561_Register_Control), TSL2561_Control_Power_Up, 1);
	//i2c_writeByte(SLAVE_ADDRESS_TSL2561, TSL2561_Command_register | TSL2561_Register_Control,TSL2561_Control_Power_Up);
	i2c_writeByte(SLAVE_ADDRESS_TSL2561, TSL2561_Command_register | TSL2561_Register_Control,0x02); //setting gain =0 and time=2 ie 402ms
	i2c_writeByte(SLAVE_ADDRESS_TSL2561, TSL2561_Command_register | TSL2561_Register_Control,TSL2561_Control_Power_Up);//power up
	for(int i=0;i<1000000;i++);
//	TIMER0_startwithCount(1900);
//	while(!TIMER_INT_SERVED);
//	TIMER_INT_SERVED = false;
}

double getLuminosityValue()
{
	uint8_t DataLow0, DataLow1, DataHigh0, DataHigh1;
	uint32_t Ch0, Ch1;
	double ratio, lux = 0;
	//lum_enable();

	//channel 0
	DataLow0 = i2c_readByte(SLAVE_ADDRESS_TSL2561,TSL2561_Command_register | TSL2561_Word_register | TSL2561_CH0_Low);
	DataHigh0 = i2c_readByte(SLAVE_ADDRESS_TSL2561,TSL2561_Command_register | TSL2561_Word_register | TSL2561_CH0_High);
	Ch0 = 256 * DataHigh0 + DataLow0;

	//channel 1
	DataLow1 = i2c_readByte(SLAVE_ADDRESS_TSL2561,TSL2561_Command_register | TSL2561_Word_register | TSL2561_CH1_Low);
	DataHigh1 = i2c_readByte(SLAVE_ADDRESS_TSL2561,TSL2561_Command_register | TSL2561_Word_register | TSL2561_CH1_High);
	Ch1 = 256 * DataHigh1 + DataLow1;

	ratio = (double)((double)Ch1)/Ch0;
	Ch0 = Ch0*16;
	Ch1 = Ch1*16;

	//Calculate LUX
	if (ratio > 0 && ratio <= 0.50)
	{
		lux = 0.0304*Ch0 - 0.062*Ch0*(pow(ratio, 1.4));
	}
	else if (ratio > 0.50 && ratio <= 0.61)
	{
		lux = 0.0224*Ch0 - 0.031*Ch1;
	}
	else if (ratio > 0.61 && ratio <= 0.80)
	{
		lux = 0.0128*Ch0 - 0.0153*Ch1;
	}
	else if (ratio > 0.80 && ratio <= 1.30)
	{
		lux = 0.00146*Ch0 - 0.00112*Ch1;
	}
	else if (ratio > 1.30)
	{
		lux = 0;
	}

	return lux;
}
