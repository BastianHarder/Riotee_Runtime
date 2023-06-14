#include <stdint.h>
#include "printf.h"
#include "nrf_gpio.h"
#include "riotee_spic.h"
#include "riotee_spis.h"
#include "max2769.h"

uint8_t snapshot_buf[SNAPSHOT_SIZE_BYTES];  //Global buffer to store one snapshot
static uint8_t tx_buf[4];           //buffer to store bytes to be transmitted to max2769
static uint8_t rx_buf[1];           //dummy buffer, no data is expected to be received from max2769
static const size_t n_tx = 4;       //number of bytes to be transmitted to max2769 is fixed to 4
static const size_t n_rx = 0;       //no data is expected to be received from max2769

//global variables to track max2769 register status as max2769 registers cannot be read
static uint32_t conf1_value;    
static uint32_t conf2_value;  
static uint32_t conf3_value; 
static uint32_t pllconf_value;
static uint32_t pllidr_value; 
static uint32_t fdiv_value;   
static uint32_t strm_value;   
static uint32_t cfdr_value;   

//Function that initializes all global variables and configures all pins that are used for the max2769 eval board. 
//This functions should be executed once after system reset
void max2769_init()
{
    //set global variables which track the current max2769 register states to the default value after a max2769 reset
    conf1_value     = MAX2769_CONF1_DEF;  
    conf2_value     = MAX2769_CONF2_DEF;  
    conf3_value     = MAX2769_CONF3_DEF;  
    pllconf_value   = MAX2769_PLLCONF_DEF;
    pllidr_value    = MAX2769_PLLIDR_DEF; 
    fdiv_value      = MAX2769_FDIV_DEF;   
    strm_value      = MAX2769_STRM_DEF;   
    cfdr_value      = MAX2769_CFDR_DEF;  
    //configure pin used for max2769 power_enable, shutdown_n and idle_n and set it low to keep the device off
	nrf_gpio_cfg_output(PIN_POWER_ENABLE);
	nrf_gpio_pin_clear(PIN_POWER_ENABLE);
}

//Function that enables the max2769 board
void enable_max2769()
{
    //Enable power converter for max2769 and set nSHDN and nIDLE pins
	nrf_gpio_pin_set(PIN_POWER_ENABLE);
}

//Function that disables the max2769 board
void disable_max2769()
{
    //Disable power converter for max2769 and clear nSHDN and nIDLE pins
	nrf_gpio_pin_clear(PIN_POWER_ENABLE);
}

//Function that writes max2769 registers over SPI to establish an application specific configuration
//This functions should be executed once after each max2769 reset or power cycle
void configure_max2769()
{
    //Info: Default values for IF center frequency are taken: fCENTER = 4MHz, BW = 2.5MHz
    
    //Set refdivider to 4 (divide 16.368 MHz crystal by 4 to have 4.092 MHz clock output) 
    set_refdiv_to_quarter();   
    //set adc to one bit resolution
    select_one_bit_adc();

    //Configure max2769 for minimal power consuption:
    //reduce_lna1_current();
    //reduce_lna2_current();
    //reduce_lo_current(); 
    //reduce_mixer_current();
    //select_3order_butfilter();
    //disable_antenna_bias();  
    //reduce_vco_current();    
    //reduce_pll_current();         //main division ratio must be divisible by 32 when reducing current!
}

void max2769_capture_snapshot(unsigned int snapshot_size_bytes, uint8_t* snapshot_buf)
{
    spis_receive(snapshot_buf, snapshot_size_bytes);
	for(int k = 0; k < snapshot_size_bytes; k++)
	{
		printf_("%02X\n", snapshot_buf[k]);
	}	
}

//Function that implements writing a 28bit value to a max2769 register with a 4 bit address
void write_max2769_register(uint8_t address, uint32_t value)
{
    //value to be written to register is located in bits 31 downto 4 (first 3 bytes + upper nibble of fourth byte)
    tx_buf[0] = (uint8_t) ((value & 0x0FF00000) >> 20); 
    tx_buf[1] = (uint8_t) ((value & 0x000FF000) >> 12); 
    tx_buf[2] = (uint8_t) ((value & 0x00000FF0) >> 4);
    tx_buf[3] = (uint8_t) ((value & 0x0000000F) << 4);
    //register address is located in bits 3 downto 0 (lower nibble of fourth byte)
    tx_buf[3] = (tx_buf[3] & 0xF0) + (address & 0x0F);
    spic_transfer(tx_buf, n_tx, rx_buf, n_rx);
}

void select_lna1()
{
    //00: LNA selection gated by the antenna bias circuit
    //01: LNA2 is active
    //10: LNA1 is active
    //11: both LNA1 and LNA2 are off
    conf1_value |= (1 << (MAX2769_CONF1_LNAMODE+1));
    conf1_value &= ~(1 << MAX2769_CONF1_LNAMODE);
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void select_lna2()
{
    //00: LNA selection gated by the antenna bias circuit
    //01: LNA2 is active
    //10: LNA1 is active
    //11: both LNA1 and LNA2 are off
    conf1_value |= (1 << MAX2769_CONF1_LNAMODE);
    conf1_value &= ~(1 << (MAX2769_CONF1_LNAMODE+1));
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void enable_antenna_bias()
{
    conf1_value |= (1 << MAX2769_CONF1_ANTEN);
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void disable_antenna_bias()
{
    conf1_value &= ~(1 << MAX2769_CONF1_ANTEN);
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void set_refdiv_to_quarter()
{
    //00: clock frequency = XTAL frequency x 2
    //01: clock frequency = XTAL frequency / 4
    //10: clock frequency = XTAL frequency / 2
    //11: clock frequency = XTAL
    pllconf_value |= (1 << MAX2769_PLL_REFDIV);
    pllconf_value &= ~(1 << (MAX2769_PLL_REFDIV+1));
    write_max2769_register(REG_MAX2769_PLLCONF, pllconf_value);
}

void set_refdiv_to_half()
{
    //00: clock frequency = XTAL frequency x 2
    //01: clock frequency = XTAL frequency / 4
    //10: clock frequency = XTAL frequency / 2
    //11: clock frequency = XTAL
    pllconf_value |= (1 << (MAX2769_PLL_REFDIV+1));
    pllconf_value &= ~(1 << MAX2769_PLL_REFDIV);
    write_max2769_register(REG_MAX2769_PLLCONF, pllconf_value);
}

void select_one_bit_adc()
{
    //Number of bits in the ADC. 000: 1 bit, 001: 1.5 bits; 010: 2 bits; 011: 2.5 bits, 100: 3 bits
    conf2_value &= ~(1 << MAX2769_CONF2_BITS);
    conf2_value &= ~(1 << (MAX2769_CONF2_BITS+1));
    conf2_value &= ~(1 << (MAX2769_CONF2_BITS+2));
    write_max2769_register(REG_MAX2769_CONF2, conf2_value);
}

void select_two_bit_adc()
{
    //Number of bits in the ADC. 000: 1 bit, 001: 1.5 bits; 010: 2 bits; 011: 2.5 bits, 100: 3 bits
    conf2_value &= ~(1 << MAX2769_CONF2_BITS);
    conf2_value |= (1 << (MAX2769_CONF2_BITS+1));
    conf2_value &= ~(1 << (MAX2769_CONF2_BITS+2));
    write_max2769_register(REG_MAX2769_CONF2, conf2_value);
}

void enable_device()
{
    conf1_value |= (1 << MAX2769_CONF1_CHIPEN);
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void disable_device()
{
    conf1_value &= ~(1 << MAX2769_CONF1_CHIPEN);
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void reduce_lna1_current()
{
    conf1_value &= ~(1 << MAX2769_CONF1_ILNA1);
    conf1_value |= (1 << (MAX2769_CONF1_ILNA1+1));
    conf1_value &= ~(1 << (MAX2769_CONF1_ILNA1+2));
    conf1_value &= ~(1 << (MAX2769_CONF1_ILNA1+3));
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void reduce_lna2_current()
{
    conf1_value &= ~(1 << MAX2769_CONF1_ILNA2);
    conf1_value &= ~(1 << (MAX2769_CONF1_ILNA2+1));
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void reduce_lo_current()
{
    conf1_value &= ~(1 << MAX2769_CONF1_ILO);
    conf1_value &= ~(1 << (MAX2769_CONF1_ILO+1));
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void reduce_mixer_current()
{
    conf1_value &= ~(1 << MAX2769_CONF1_IMIX);
    conf1_value &= ~(1 << (MAX2769_CONF1_IMIX+1));
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void select_3order_butfilter()
{
    conf1_value |= (1 << MAX2769_CONF1_F3OR5);
    write_max2769_register(REG_MAX2769_CONF1, conf1_value);
}

void reduce_vco_current()
{
    pllconf_value |= (1 << MAX2769_PLL_IVCO);
    write_max2769_register(REG_MAX2769_PLLCONF, pllconf_value);
}

void reduce_pll_current()
{
    pllconf_value |= (1 << MAX2769_PLL_PWRSAV);
    write_max2769_register(REG_MAX2769_PLLCONF, pllconf_value);
}

void enable_q_channel()
{
    conf2_value |= (1 << MAX2769_CONF2_IQEN);
    write_max2769_register(REG_MAX2769_CONF2, conf2_value);
    conf3_value |= (1 << MAX2769_CONF3_PGAQEN);
    write_max2769_register(REG_MAX2769_CONF3, conf3_value);
}

void disable_q_channel()
{
    conf2_value &= ~(1 << MAX2769_CONF2_IQEN);
    write_max2769_register(REG_MAX2769_CONF2, conf2_value);
    conf3_value &= ~(1 << MAX2769_CONF3_PGAQEN);
    write_max2769_register(REG_MAX2769_CONF3, conf3_value);
}