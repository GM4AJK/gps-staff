
#ifndef INC_APP_H_
#define INC_APP_H_

#include <stdbool.h>

extern volatile bool BNO085_INT_STATE;

void app_init(void);
void app_loop(void);
void app_1ms(void);

#endif /* INC_APP_H_ */
