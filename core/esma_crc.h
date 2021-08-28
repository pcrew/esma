
#ifndef ESMA_CRC_H
#define ESMA_CRC_H

#include "common/numeric_types.h"

/* CRC 8 */
#define CRC8_BASIC	0x00
#define CRC8_CDMA2000	0x01
#define CRC8_DARC	0x02
#define CRC8_WCDMA	0x03
#define CRC8_MAXIM	0x04

/* CRC 16 */
#define CRC16_ARC	0x00
#define CRC16_AUG_CCITT	0x01
#define CRC16_BUYPASS	0x02
#define CRC16_CDMA2000	0x03
#define CRC16_DDS_110	0x04
#define CRC16_DECT_R	0x05
#define CRC16_DECT_X	0x06
#define CRC16_DNP	0x07
#define CRC16_EN_13757	0x08
#define CRC16_GENIBUS	0x09
#define CRC16_MAXIM	0x0a
#define CRC16_MCRF4XX	0x0b
#define CRC16_RIELLO	0x0c
#define CRC16_T10_DIF	0x0d
#define CRC16_TELEDISK	0x0e
#define CRC16_TMS37157	0x0f
#define CRC16_USB	0x10
#define CRC16_A		0x11
#define CRC16_KERMIT	0x12
#define CRC16_MODBUS	0x13
#define CRC16_X_25	0x14
#define CRC16_XMODEM	0x15

/* CRC 32 */
#define CRC32_BASIC	0x00
#define CRC32_BZIP2	0x01
#define CRC32_C		0x02
#define CRC32_D		0x03
#define CRC32_MPEG2	0x04
#define CRC32_POSIX	0x05
#define CRC32_Q		0x06
#define CRC32_JAMCRC	0x07
#define CRC32_XFER	0x08

int esma_crc8(u8 *d, u32 len, u8 *crc, u32 name);
int esma_crc16(u8 *d, u32 len, u16 *crc, u32 name);
int esma_crc32(u8 *d, u32 len, u32 *crc, u32 name);

#endif
