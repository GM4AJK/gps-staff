#pragma once
#include "host/ble_hs.h"

void wifi_prov_register_svcs(void);
void wifi_prov_init(void);
void wifi_prov_start_task(void);
void wifi_prov_on_connect(uint16_t conn_handle);
void wifi_prov_on_disconnect(void);
