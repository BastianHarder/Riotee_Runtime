#include <stdint.h>
#include "printf.h"
#include "nrf_gpio.h"
#include "riotee_spic.h"
#include "riotee_spis.h"
#include "max2769.h"

//Internal function prototypes
static void write_max2769_register(uint8_t address, uint32_t value);
static void set_sampling_frequency(const max2769_cfg_t *cfg);
static void set_adc_resolution(const max2769_cfg_t *cfg);
static void select_lna1();
static void select_lna2();
static void enable_antenna_bias();
static void disable_antenna_bias();
static void enable_device();
static void disable_device();
static void reduce_lna1_current();
static void reduce_lna2_current();
static void reduce_lo_current();
static void reduce_mixer_current();
static void select_3order_butfilter();
static void reduce_vco_current();
static void reduce_pll_current();
static void enable_q_channel();
static void disable_q_channel();

static uint8_t tx_buf[4];           //buffer to store bytes to be transmitted to max2769
static uint8_t rx_buf[1];           //dummy buffer, no data is expected to be received from max2769
static const size_t n_tx = 4;       //number of bytes to be transmitted to max2769 is fixed to 4
static const size_t n_rx = 0;       //no data is expected to be received from max2769

//global structure to track max2769 register status as max2769 registers cannot be read
static max2769_registers_t max2769_reg =  {.conf1_value = MAX2769_CONF1_DEF,
                                           .conf2_value = MAX2769_CONF2_DEF,
                                           .conf3_value = MAX2769_CONF3_DEF,
                                           .pllconf_value = MAX2769_PLLCONF_DEF,
                                           .pllidr_value = MAX2769_PLLIDR_DEF,
                                           .fdiv_value = MAX2769_FDIV_DEF,
                                           .strm_value = MAX2769_STRM_DEF,
                                           .cfdr_value = MAX2769_CFDR_DEF};

//Function that configures all pins that are connected to max2769 
//This functions should be executed once after system reset
void max2769_init(const max2769_cfg_t *cfg)
{ 
    //configure pin used for max2769 power_enable, shutdown_n and idle_n and set it low to keep the device off
	nrf_gpio_cfg_output(cfg->pin_pe);
	nrf_gpio_pin_clear(cfg->pin_pe);
}

//Function that enables the max2769 board
void enable_max2769(const max2769_cfg_t *cfg)
{
    //Enable power converter for max2769 and set nSHDN and nIDLE pins
	nrf_gpio_pin_set(cfg->pin_pe);
}

//Function that disables the max2769 board
void disable_max2769(const max2769_cfg_t *cfg)
{
    //Disable power converter for max2769 and clear nSHDN and nIDLE pins
	nrf_gpio_pin_clear(cfg->pin_pe);
}

//Function that writes max2769 registers over SPI to establish an application specific configuration
//This functions should be executed once after each max2769 reset or power cycle
void configure_max2769(const max2769_cfg_t *cfg)
{
    set_sampling_frequency(cfg);
    set_adc_resolution(cfg);
    //Check if max2769 shall be configure for minimal power consuption
    if(cfg->min_power_option == MAX2769_MIN_POWER_OPTION_ENABLE)
    {
        reduce_lna1_current();
        reduce_lna2_current();
        reduce_lo_current(); 
        reduce_mixer_current();
        select_3order_butfilter();
        disable_antenna_bias();  
        reduce_vco_current();    
        reduce_pll_current();         //main division ratio must be divisible by 32 when reducing current!
    }
    //Info: Default values for IF center frequency are taken: fCENTER = 4MHz, BW = 2.5MHz
}

//Function that enables an customized spi slave to receive a snapshot from max2769
//Note: max2769 must be enabled and configured in advance
void max2769_capture_snapshot(const riotee_spis_cfg_t* spis_cfg, const max2769_cfg_t *max2769_cfg, uint8_t* snapshot_buf)
{
    spis_receive(spis_cfg, snapshot_buf, max2769_cfg->snapshot_size_bytes);
	for(int k = 0; k < max2769_cfg->snapshot_size_bytes; k++)
	{
		printf_("%02X\n", snapshot_buf[k]);
	}	
}

//Function that implements writing a 28bit value to a max2769 register with a 4 bit address
static void write_max2769_register(uint8_t address, uint32_t value)
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

//Function that configures the max2769 reference divider to match the specified sampling frequency
static void set_sampling_frequency(const max2769_cfg_t *cfg)
{
    //clear bits that correspond to the reference divider value
    max2769_reg.pllconf_value &= ~(0b11 << MAX2769_PLL_REFDIV);
    //write bits corresponding to the reference divider according to specification of sampling frequency
    max2769_reg.pllconf_value |= (cfg->sampling_frequency << MAX2769_PLL_REFDIV);  
    write_max2769_register(REG_MAX2769_PLLCONF, max2769_reg.pllconf_value);
}


static void set_adc_resolution(const max2769_cfg_t *cfg)
{
    //clear bits that correspond to the adc resolution
    max2769_reg.conf2_value &= ~(0b111 << MAX2769_CONF2_BITS);    
    //write bits that correspond to the adc resolution according to specification of adc resolution                 
    max2769_reg.conf2_value |= (cfg->adc_resolution << MAX2769_CONF2_BITS);   
    write_max2769_register(REG_MAX2769_CONF2, max2769_reg.conf2_value);
}

static void select_lna1()
{
    //00: LNA selection gated by the antenna bias circuit
    //01: LNA2 is active
    //10: LNA1 is active
    //11: both LNA1 and LNA2 are off
    max2769_reg.conf1_value |= (1 << (MAX2769_CONF1_LNAMODE+1));
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_LNAMODE);
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void select_lna2()
{
    //00: LNA selection gated by the antenna bias circuit
    //01: LNA2 is active
    //10: LNA1 is active
    //11: both LNA1 and LNA2 are off
    max2769_reg.conf1_value |= (1 << MAX2769_CONF1_LNAMODE);
    max2769_reg.conf1_value &= ~(1 << (MAX2769_CONF1_LNAMODE+1));
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void enable_antenna_bias()
{
    max2769_reg.conf1_value |= (1 << MAX2769_CONF1_ANTEN);
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void disable_antenna_bias()
{
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_ANTEN);
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void enable_device()
{
    max2769_reg.conf1_value |= (1 << MAX2769_CONF1_CHIPEN);
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void disable_device()
{
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_CHIPEN);
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void reduce_lna1_current()
{
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_ILNA1);
    max2769_reg.conf1_value |= (1 << (MAX2769_CONF1_ILNA1+1));
    max2769_reg.conf1_value &= ~(1 << (MAX2769_CONF1_ILNA1+2));
    max2769_reg.conf1_value &= ~(1 << (MAX2769_CONF1_ILNA1+3));
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void reduce_lna2_current()
{
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_ILNA2);
    max2769_reg.conf1_value &= ~(1 << (MAX2769_CONF1_ILNA2+1));
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void reduce_lo_current()
{
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_ILO);
    max2769_reg.conf1_value &= ~(1 << (MAX2769_CONF1_ILO+1));
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void reduce_mixer_current()
{
    max2769_reg.conf1_value &= ~(1 << MAX2769_CONF1_IMIX);
    max2769_reg.conf1_value &= ~(1 << (MAX2769_CONF1_IMIX+1));
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void select_3order_butfilter()
{
    max2769_reg.conf1_value |= (1 << MAX2769_CONF1_F3OR5);
    write_max2769_register(REG_MAX2769_CONF1, max2769_reg.conf1_value);
}

static void reduce_vco_current()
{
    max2769_reg.pllconf_value |= (1 << MAX2769_PLL_IVCO);
    write_max2769_register(REG_MAX2769_PLLCONF, max2769_reg.pllconf_value);
}

static void reduce_pll_current()
{
    max2769_reg.pllconf_value |= (1 << MAX2769_PLL_PWRSAV);
    write_max2769_register(REG_MAX2769_PLLCONF, max2769_reg.pllconf_value);
}

static void enable_q_channel()
{
    max2769_reg.conf2_value |= (1 << MAX2769_CONF2_IQEN);
    write_max2769_register(REG_MAX2769_CONF2, max2769_reg.conf2_value);
    max2769_reg.conf3_value |= (1 << MAX2769_CONF3_PGAQEN);
    write_max2769_register(REG_MAX2769_CONF3, max2769_reg.conf3_value);
}

static void disable_q_channel()
{
    max2769_reg.conf2_value &= ~(1 << MAX2769_CONF2_IQEN);
    write_max2769_register(REG_MAX2769_CONF2, max2769_reg.conf2_value);
    max2769_reg.conf3_value &= ~(1 << MAX2769_CONF3_PGAQEN);
    write_max2769_register(REG_MAX2769_CONF3, max2769_reg.conf3_value);
}