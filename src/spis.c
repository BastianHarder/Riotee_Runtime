#include "nrf.h"
#include "nrf_gpio.h"
#include "riotee_spis.h"

//This spi slave is used to receive data from a data source that provides continuous data (clk and data signal) and no cs signal
//Therefore the cs signal has to be driven by the spi slave as well
//define cs pin as global variable
unsigned int spis_cs_out;

int spis_init(riotee_spis_cfg_t* cfg)
{
    //External data source only provides clk and data bit. CS signal has to be generated by the receiver itself whenever data shall be recorded
	NRF_SPIS2->PSEL.CSN = cfg->pin_cs_in;
	NRF_SPIS2->PSEL.SCK = cfg->pin_sck;
	NRF_SPIS2->PSEL.MOSI = cfg->pin_mosi;
    spis_cs_out = cfg->pin_cs_out;
	//configure cs output
	nrf_gpio_pin_clear(spis_cs_out);
	nrf_gpio_cfg_output(spis_cs_out);
    //Set cs pin high because it should be idle high
	nrf_gpio_pin_set(spis_cs_out);			
    //Define default characters
	NRF_SPIS2->ORC = 0x01;															//Over-read character
    NRF_SPIS2->DEF = 0x99;															//ignored transaction character
	//Define spi mode
    switch (cfg->mode) {
        case SPIS_MODE0_CPOL0_CPHA0:
            NRF_SPIS2->CONFIG =
                (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPIS_MODE1_CPOL0_CPHA1:
            NRF_SPIS2->CONFIG =
                (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPIS_MODE2_CPOL1_CPHA0:
            NRF_SPIS2->CONFIG =
                (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        case SPIS_MODE3_CPOL1_CPHA1:
            NRF_SPIS2->CONFIG =
                (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
    }
    //No need to use the semaphore because the receive events are controlled by the slave (the CS signal is self generated)
    //Enable ENDRX interrupt event to indicate when the expected amount of bytes has been received (rx buffer is filled)
    NRF_SPIS2->INTENSET = (SPIS_INTENSET_ENDRX_Set << SPIS_INTENSET_ENDRX_Pos);
    //No Transmit functionality, only receiver
    NRF_SPIS2->TXD.MAXCNT = 0;
	//Enable SPI Slave 2					
	NRF_SPIS2->ENABLE = (SPIS_ENABLE_ENABLE_Enabled << SPIS_ENABLE_ENABLE_Pos);
    //After Enabling a SPI Slave, the semaphore is automatically given to the CPU. It is released now to enable the spi slave to receive data
    NRF_SPIS2->TASKS_RELEASE = (SPIS_TASKS_RELEASE_TASKS_RELEASE_Trigger << SPIS_TASKS_RELEASE_TASKS_RELEASE_Pos);
	return 0;
}

int spis_receive(uint8_t *rx_buf, unsigned n_rx)
{
    //Define how many bytes shall be received
	NRF_SPIS2->RXD.MAXCNT = n_rx;
    //Define where the received data shall be stored
	NRF_SPIS2->RXD.PTR = (uint32_t)rx_buf;
    //Clear Chip Select to start receiving bytes
	nrf_gpio_pin_clear(spis_cs_out);
	/* Busy wait until the expected amount of bytes is received */
	while (NRF_SPIS2->EVENTS_ENDRX == 0) {
	};
	//abort receiving bytes
	nrf_gpio_pin_set(spis_cs_out);
	//Clear Event
	NRF_SPIS2->EVENTS_ENDRX = 0;
	return 0;
}