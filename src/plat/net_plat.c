#include "net_plat.h"
#include "net_err.h"
#include "dbg.h"
net_err_t net_plat_init() {
    dbg_info(DBG_PLAT, "Initializing platform...\n");
    dbg_info(DBG_PLAT, "Platform done.\n");
    return NET_ERR_OK;
}