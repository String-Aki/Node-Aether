#ifndef ETHERNET_WOL_H
#define ETHERNET_WOL_H

#include "esp_eth.h"

/* ============================================================================
 * MASTER SWITCH — Ethernet / W5500 / Wake-on-LAN
 * Set to 1 to re-enable. Set to 0 to fully disable:
 *   - No SPI bus init, no W5500 driver install (saves the eth driver's
 *     internal buffers + DMA descriptors)
 *   - No WOL button task created in main.c (saves its 4096-byte stack)
 *   - eth_handle stays NULL forever, so send_wake_on_lan() safely no-ops
 *     wherever it's called (UDP "wake" command, etc.) — nothing else
 *     needs to change.
 * This is the ONLY line to flip when you plug the module back in.
 * ========================================================================== */
#define ENABLE_ETHERNET_WOL 0

extern esp_eth_handle_t eth_handle;

void init_w5500_ethernet(void);
void send_wake_on_lan(void);
void wol_button_task(void *arg);

#endif