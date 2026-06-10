/*
 * Author: Andy Kirkham
 */

#include <stdint.h>
#include <stdatomic.h>

#include "flags.h"

enum {
	FLAG_10MS,
	FLAG_100MS,
	FLAG_1000MS,
	FLAG_BNO085_INT,
};

static const uint32_t FLAG_MASK_10MS       = (1UL << FLAG_10MS);
static const uint32_t FLAG_MASK_100MS      = (1UL << FLAG_100MS);
static const uint32_t FLAG_MASK_1000MS     = (1UL << FLAG_1000MS);
static const uint32_t FLAG_MASK_BNO085_INT = (1UL << FLAG_BNO085_INT);

static volatile uint32_t flags = 0;

/*
 * Functions generator macro. Creates three functions:-
 *   flag_set_<type>()  ... Set a flag (return previous state).
 *   flag_get_<type>()  ... Get a flag (and if set reset it).
 *   flag_peek_<type>() ... Get a flag (leave flag untouched).
 */
#define MAKE_C(suffix) \
	bool flag_set_##suffix(void) { \
		bool rval = (flags & FLAG_MASK_##suffix); \
		atomic_fetch_or(&flags, FLAG_MASK_##suffix); \
		return rval; \
	} \
	bool flag_get_##suffix(void) { \
		bool rval = (flags & FLAG_MASK_##suffix); \
		if(rval) atomic_fetch_and(&flags, ~FLAG_MASK_##suffix); \
		return rval; \
	} \
	bool flag_peek_##suffix(void) { \
		bool rval = (flags & FLAG_MASK_##suffix); \
		return rval; \
	}

MAKE_C(10MS)
MAKE_C(100MS)
MAKE_C(1000MS)
MAKE_C(BNO085_INT)
