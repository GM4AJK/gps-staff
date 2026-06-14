/*
 * Author: Andy Kirkham
 */

#ifndef INC_FLAGS_H_
#define INC_FLAGS_H_

#include <stdbool.h>

#define MAKE_H(suffix) \
	bool flag_set_##suffix(void); \
	bool flag_get_##suffix(void); \
	bool flag_peek_##suffix(void);

MAKE_H(10MS)
MAKE_H(100MS)
MAKE_H(1000MS)
MAKE_H(SX1262_DIO1)

#endif /* INC_FLAGS_H_ */
