#ifndef __NET_ERR_H__
#define __NET_ERR_H__

// Define network error codes
typedef enum {
    NET_ERR_OK = 0,       // No error
    NET_ERR_SYS = -1,// System error
    NET_ERR_MEM = -2,// Memory allocation error
    NET_ERR_FULL = -3,// Queue full error
    NET_ERR_TMS = -4,// Timeout error
    NET_ERR_SIZE = -5,// Size error
    NET_ERR_NONE = -6,// No data error
    NET_ERR_PARAM = -7,// Parameter error
} net_err_t;

#endif // __NET_ERR_H__