#ifndef I2C
#define I2C

#include "em_i2c.h"
#include "em_cmu.h"

void i2c_init(void);
void i2c_enable(void);
float Caculate_Celsius(uint16_t i2c_read_data);
void i2c_disable(void);
uint8_t i2c_readByte(uint8_t slave_addr,uint8_t mem_addr);
void i2c_writeByte(uint8_t slave_addr,uint8_t mem_addr,uint8_t data);
double getLuminosityValue();
void lum_enable();

#define SLAVE_ADDRESS_temp            0x40
#define SLAVE_ADDRESS_TSL2561         0x39

#define TSL2561_Command_register      0x80
#define TSL2561_Word_register         0x20
#define TSL2561_Control_Power_Up      0x03
#define TSL2561_Control_Power_Down    0x00
#define TSL2561_Register_Control      0x00

#define TSL2561_CH0_Low               0x0c
#define TSL2561_CH0_High              0x0d
#define TSL2561_CH1_Low               0x0e
#define TSL2561_CH1_High              0x0f



#endif
