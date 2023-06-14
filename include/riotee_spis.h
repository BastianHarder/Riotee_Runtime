#ifndef __RIOTEE_SPIS_H_
#define __RIOTEE_SPIS_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SPIS_MODE0_CPOL0_CPHA0,
  SPIS_MODE1_CPOL0_CPHA1,
  SPIS_MODE2_CPOL1_CPHA0,
  SPIS_MODE3_CPOL1_CPHA1,
} riotee_spis_mode_t;

typedef struct {
  riotee_spis_mode_t mode;
  unsigned int pin_cs_out;
  unsigned int pin_cs_in;
  unsigned int pin_sck;
  unsigned int pin_mosi;
} riotee_spis_cfg_t;

int spis_init(const riotee_spis_cfg_t* cfg);
int spis_receive(const riotee_spis_cfg_t* cfg, uint8_t *rx_buf, unsigned n_rx);

#ifdef __cplusplus
}
#endif

#endif /* __RIOTEE_SPIS_H_ */