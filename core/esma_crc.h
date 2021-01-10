
#ifndef ESMA_CRC_H
#define ESMA_CRC_H

#include "common/numeric_types.h"

/* CRC 8 */
#define BASIC		0x00
#define AUTOSTAR	0x01
#define BLUETOOTH	0x02
#define CCITT		0x03
#define DALLAS		0x04
#define MAXIM		DALLAS
#define DARC		0x06
#define GSM_B		0x07
#define SAE_J1850	0x08
#define WCDMA		0x09

/* CRC 16 */
#define ARC		0x0A
#define AUG_CCITT	0x0B
#define BUYPASS		0x0C
#define CCITT_FALE	0x0D
#define CDMA2000	0x0E
#define DDS_110		0x0F
#define DECT_R		0x10
#define DECT_X		0x11
#define DNP		0x12
#define EN_13757	0x13
#define GENIBUS		0x14
#define MAXIM		DALLAS
#define MCRF4XX		0x16
#define RIELLO		0x17
#define T10_DIF		0x18
#define TELEDISK	0x19
#define TMS37157	0x1A
#define USB		0x1B
#define A		0x1C
#define KERMIT		0x1D
#define MODBUS		0x1E
#define X_25		0x1F
#define XMODEM		0x20

/* CRC 32 */
#define BZIP2		0x22
#define C		0x23
#define D		0x24
#define MPEG2		0x25
#define POSIX		0x26
#define Q		0x27
#define JAMCRC		0x28
#define XFER		0x29

u8 esma_crc8(u8 *d, u32 len, u32 name);
u16 esma_crc16(u8 *d, u32 len, u32 name);
u32 esma_crc32(u8 *d, u32 len, u32 name);
u64 esma_crc64(u8 *d, u32 len, u32 name);

#endif
