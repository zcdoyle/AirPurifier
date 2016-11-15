#ifndef __CRC__
#define __CRC__

#include <stdint.h>

uint16_t CRC16(unsigned char *puchMsg, uint16_t usDataLen);
uint32_t CRC32(unsigned char *puchMsg, uint16_t uSize);
#endif
