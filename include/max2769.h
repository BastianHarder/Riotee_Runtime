#ifndef __MAX2769_H_
#define __MAX2769_H_

#include <stdint.h>
#include "riotee.h"

//define the length of the snapshots to be captured
//For a 12ms snapshot and 4.092MHz sampling rate and 1Bit resolution:
//Calculation: 4092000 Hz * 0.012s / 8 = 6138 Byte
#define SNAPSHOT_SIZE_BYTES 6138
//Global buffer to store snapshot
extern uint8_t snapshot_buf[SNAPSHOT_SIZE_BYTES];

//MAX2769 Pin Definition
#define PIN_POWER_ENABLE    PIN_D6        //This pin must be connected to power enable, nSHDN and nIDLE pins of max2769 eval board

/* MAX2769 Register addresses */
// Name                      address         default value
#define REG_MAX2769_CONF1    0b0000       // 0xA2919A3
#define REG_MAX2769_CONF2    0b0001       // 0x0550288
#define REG_MAX2769_CONF3    0b0010       // 0xEAFF1DC
#define REG_MAX2769_PLLCONF  0b0011       // 0x9EC0008
#define REG_MAX2769_PLLIDR   0b0100       // 0x0C00080
#define REG_MAX2769_FDIV     0b0101       // 0x8000070
#define REG_MAX2769_STRM     0b0110       // 0x8000000
#define REG_MAX2769_CFDR     0b0111       // 0x10061B2

/* MAX2769 Register defaults */
#define MAX2769_CONF1_DEF    0xA2919A3
#define MAX2769_CONF2_DEF    0x0550288
#define MAX2769_CONF3_DEF    0xEAFF1DC
#define MAX2769_PLLCONF_DEF  0x9EC0008
#define MAX2769_PLLIDR_DEF   0x0C00080
#define MAX2769_FDIV_DEF     0x8000070
#define MAX2769_STRM_DEF     0x8000000
#define MAX2769_CFDR_DEF     0x10061B2

/* MAX2769 Register bit positions */
/* CONF1 */
#define  MAX2769_CONF1_CHIPEN        ((uint8_t)(27))    /* Chip enable */
#define  MAX2769_CONF1_IDLE          ((uint8_t)(26))    /* Idle enable */
#define  MAX2769_CONF1_ILNA1         ((uint8_t)(22))    /* LNA1 current */
#define  MAX2769_CONF1_ILNA2         ((uint8_t)(20))    /* LNA2 current */
#define  MAX2769_CONF1_ILO           ((uint8_t)(18))    /* LO buffer current */
#define  MAX2769_CONF1_IMIX          ((uint8_t)(16))    /* Mixer Current */
#define  MAX2769_CONF1_MIXPOLE       ((uint8_t)(15))    /* Mixer Pole selection */
#define  MAX2769_CONF1_LNAMODE       ((uint8_t)(13))    /* LNA Mode Select */
#define  MAX2769_CONF1_MIXEN         ((uint8_t)(12))    /* Mixer enable */
#define  MAX2769_CONF1_ANTEN         ((uint8_t)(11))    /* Antenna Bias Enable */
#define  MAX2769_CONF1_FCEN          ((uint8_t)(5))     /* IF Center Frequency */
#define  MAX2769_CONF1_FBW           ((uint8_t)(3))     /* IF Filter Center Bandwidth */
#define  MAX2769_CONF1_F3OR5         ((uint8_t)(2))     /* Filter Order Select */
#define  MAX2769_CONF1_FCENX         ((uint8_t)(1))     /* Polyphase filter Select */
#define  MAX2769_CONF1_FGAIN         ((uint8_t)(0))     /* IF Filter Gain Select */

/* CONF2 */
#define  MAX2769_CONF2_IQEN          ((uint8_t)(27))    /* I and Q Channels Enable */
#define  MAX2769_CONF2_GAINREF       ((uint8_t)(15))    /* AGC Gain */
#define  MAX2769_CONF2_AGCMODE       ((uint8_t)(11))    /* AGC Mode */
#define  MAX2769_CONF2_FORMAT        ((uint8_t)(9))     /* Output Data Format */
#define  MAX2769_CONF2_BITS          ((uint8_t)(6))     /* Num of bits in ADC */
#define  MAX2769_CONF2_DRVCFG        ((uint8_t)(4))     /* Output Driver Configuration */
#define  MAX2769_CONF2_LOEN          ((uint8_t)(3))     /* LO buffer enable */
#define  MAX2769_CONF2_DIEID         ((uint8_t)(0))     /* Identify version of IC */

/* CONF3 */
#define  MAX2769_CONF3_GAININ        ((uint8_t)(22))   /* PGA Gain */
#define  MAX2769_CONF3_FSLOWEN       ((uint8_t)(21))   /* Low value of ADC full-scale...*/
#define  MAX2769_CONF3_HILOADEN      ((uint8_t)(20))   /* Output driver strength HI-load*/
#define  MAX2769_CONF3_ADCEN         ((uint8_t)(19))   /* ADC Enable */
#define  MAX2769_CONF3_DRVEN         ((uint8_t)(18))   /* Output Driver Enable */
#define  MAX2769_CONF3_FOFSTEN       ((uint8_t)(17))   /* Filter DC...*/
#define  MAX2769_CONF3_FILTEN        ((uint8_t)(16))   /* IF Filter Enable */
#define  MAX2769_CONF3_FHIPEN        ((uint8_t)(15))   /* Highpass Coupling Enable */
#define  MAX2769_CONF3_PGAIEN        ((uint8_t)(13))   /* I-channel PGA Enable */
#define  MAX2769_CONF3_PGAQEN        ((uint8_t)(12))   /* Q-channel PGA Enable */
#define  MAX2769_CONF3_STRMEN        ((uint8_t)(11))   /* DSP interface... */
#define  MAX2769_CONF3_STRMSTART     ((uint8_t)(10))   /* ... */
#define  MAX2769_CONF3_STRMSTOP      ((uint8_t)(9))    /* ... */
#define  MAX2769_CONF3_STRMCOUNT     ((uint8_t)(6))    /* ... */
#define  MAX2769_CONF3_STRMBITS      ((uint8_t)(4))    /* ... */
#define  MAX2769_CONF3_STAMPEN       ((uint8_t)(3))    /* ... */
#define  MAX2769_CONF3_TIMESYNCEN    ((uint8_t)(2))    /* ... */
#define  MAX2769_CONF3_DATSYNCEN    ((uint8_t)(1))    /* ... */
#define  MAX2769_CONF3_STRMRST       ((uint8_t)(0))    /* ... */

/* PLL */
#define  MAX2769_PLL_VCOEN           ((uint8_t)(27))   /* VCO Enable */
#define  MAX2769_PLL_IVCO            ((uint8_t)(26))   /* VCO Current Select */
#define  MAX2769_PLL_REFOUTEN        ((uint8_t)(24))   /* Clock buffer enable */
#define  MAX2769_PLL_REFDIV          ((uint8_t)(21))   /* Clock Output Divider Ratio */
#define  MAX2769_PLL_IXTAL           ((uint8_t)(19))   /* Current for XTAL */
#define  MAX2769_PLL_XTALCAP         ((uint8_t)(14))   /* Digital Load Cap for XTAL */
#define  MAX2769_PLL_LDMUX           ((uint8_t)(10))   /* LD pin output select */
#define  MAX2769_PLL_ICP             ((uint8_t)(9))    /* Charge Pump current */
#define  MAX2769_PLL_PFDEN           ((uint8_t)(8))    /* ...  */
#define  MAX2769_PLL_CPTEST          ((uint8_t)(4))    /* Charge Pump Test  */
#define  MAX2769_PLL_INT_PLL         ((uint8_t)(3))    /* PLL Mode Ctl */
#define  MAX2769_PLL_PWRSAV          ((uint8_t)(2))    /* PLL Power Save */

/* PLL Integer Division Ratio */
#define  MAX2769_PLLIDR_NDIV         ((uint8_t)(13))   /* PLL Integer Division Ratio */
#define  MAX2769_PLLIDR_RDIV         ((uint8_t)(3))    /* PLL Reference Division Ratio */

/* PLL Integer Division Ratio */
#define  MAX2769_PLLDR_FDIV          ((uint8_t)(8))    /* PLL Fractional Divider Ratio */

/* DSP Interface */
#define  MAX2769_DSP_FRAMECOUNT      ((uint8_t)(27))   /* ... */

/* Clock Fractional Division Ratio */
#define  MAX2769_CFDR_L_CNT          ((uint8_t)(16))    /* L Counter Value */
#define  MAX2769_CFDR_M_CNT          ((uint8_t)(4))     /* M Counter Value */
#define  MAX2769_CFDR_FCLKIN         ((uint8_t)(3))     /* Frac. Clock Divider */
#define  MAX2769_CFDR_ADCCLK         ((uint8_t)(2))     /* ADC Clock Selection */
#define  MAX2769_CFDR_SERCLK         ((uint8_t)(1))     /* Serializer Clock Selection */
#define  MAX2769_CFDR_MODE           ((uint8_t)(0))     /* DSP Interface Mode Selection */

//function prototypes
void max2769_init();
void enable_max2769();
void disable_max2769();
void configure_max2769();
void max2769_capture_snapshot(unsigned int snapshot_size_bytes, uint8_t* snapshot_buf);
void write_max2769_register(uint8_t address, uint32_t value);
void select_lna1();
void select_lna2();
void enable_antenna_bias();
void disable_antenna_bias();
void set_refdiv_to_quarter();
void set_refdiv_to_half();
void select_one_bit_adc();
void select_two_bit_adc();
void reduce_lna1_current();
void reduce_lna2_current();
void reduce_lo_current();
void reduce_mixer_current();
void select_3order_butfilter();
void reduce_vco_current();
void reduce_pll_current();
void enable_q_channel();
void disable_q_channel();

#endif /* __MAX2769_H_ */