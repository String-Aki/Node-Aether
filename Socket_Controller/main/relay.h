#ifndef RELAY_H
#define RELAY_H

#include "driver/gpio.h"

// Define your pins (Change these to match your actual wiring!)
#define RELAY_1_PIN GPIO_NUM_5  // Laptop Charger
#define RELAY_2_PIN GPIO_NUM_4  // Router Phone Charger

void relay_init(void);

// Pass 1 for Relay 1 (Laptop), 2 for Relay 2 (Phone)
void relay_set_state(int relay_num, bool on);
bool relay_get_state(int relay_num);

#endif // RELAY_H