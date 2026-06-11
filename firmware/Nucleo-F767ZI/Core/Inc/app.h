
#ifndef INC_APP_H_
#define INC_APP_H_

#include <stdint.h>

void app_init(void);
void app_loop(void);
void app_1ms(void);

void app_delay_us(uint32_t us);

#endif /* INC_APP_H_ */
