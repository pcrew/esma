
#include <string.h>
#include <arpa/inet.h> /* for htons */

#include "esma_crc.h"

struct crc8_poly {
	u8 normal;
	u8 revers;
};

struct crc16_poly {
	u16 normal;
	u16 revers;
};

struct crc32_poly {
	u32 normal;
	u32 revers;
};

static u8 __crc8_false_false(u8 *d, u32 len, u8 crc, u8 poly)
{
	while (len--) {
		crc ^= *d++;
		for (u8 i = 0; i < 8; i++)
			crc = crc & 0x80 ? (crc << 1) ^ poly : crc << 1;
	}

	return crc;
}

static u8 __crc8_true_true(u8 *d, u32 len, u8 crc, u8 poly)
{
	while (len--) {
		crc ^= *d++;
		for (u8 i = 0; i < 8; i++)
			crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
	}

	return crc;
}

static struct crc8_poly crc8_poly[] = {
	[0] = {
		.normal = 0x07,
		.revers = 0xD0,
	},
	[1] = {
		.normal = 0x39, //0011 1001 / 1001 1100
		.revers = 0x9C,
	},
	[2] = {
		.normal = 0x31, //0011 0001 / 1000 1100
		.revers = 0x8C,
	},
	[3] = {
		.normal = 0x9B, //1001 1011 / 1101 1001
		.revers = 0xBD,
	},
	[4] = {
		.normal = 0x07,
		.revers = 0xD0,
	}
};

int esma_crc8(u8 *d, u32 len, u8 *crc, u32 name)
{

	u8 init;

	switch (name) {
	case CRC8_BASIC:
		init = 0x00;
		*crc = __crc8_false_false(d, len, init, crc8_poly[0].normal);
		break;

	case CRC8_CDMA2000:
		init = 0xFF;
		*crc = __crc8_false_false(d, len, init, crc8_poly[3].normal);
		break;

	case CRC8_DARC:
		init = 0x00;
		*crc = __crc8_true_true(d, len, init, crc8_poly[1].revers);
		break;

	case CRC8_WCDMA:
		init = 0x00;
		*crc = __crc8_true_true(d, len, init, crc8_poly[3].revers);
		break;

	case CRC8_MAXIM:
		init = 0xFF;
		*crc = __crc8_true_true(d, len, init, crc8_poly[2].revers);
		break;

	default:
		return 1;
	}

	return 0;
}

static u16 __crc16_false_false(u8 *d, u32 len, u16 crc, u16 poly)
{
	while (len--) {	
		crc ^= *d++ << 8;
		for (u8 i = 0; i < 8; i++)
			crc = crc & 0x8000 ? (crc << 1) ^ poly : crc << 1;
	}

	return crc;
}

static u16 __crc16_true_true(u8 *d, u32 len, u16 crc, u16 poly)
{
	while (len--) {
		crc ^= *d++;
		for (u8 i = 0; i < 8; i++)
			crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
	}

	return crc;
}

static struct crc16_poly crc16_poly[] = {
	[0] = {
		.normal = 0x1021,
		.revers = 0x8408,
	},
	[1] = {
		.normal = 0x8005,
		.revers = 0xA001,
	},
	[2] = {
		.normal = 0xC867,
		.revers = 0xE613,
	},
	[3] = {
		.normal = 0x0589,
		.revers = 0x91A0,
	},
	[4] = {
		.normal = 0x3D65,
		.revers = 0xA6BC,
	},
	[5] = {
		.normal = 0x8BB7,
		.revers = 0xEDD1,
	},
	[6] = {
		.normal = 0xA097,
		.revers = 0x0000,
	},
};

int esma_crc16(u8 *d, u32 len, u16 *crc, u32 name)
{
	u16 init;
	switch (name) {
	case CRC16_ARC:
		init = 0x0000;
		*crc = __crc16_true_true(d, len, init, crc16_poly[1].revers);
		break;

	case CRC16_AUG_CCITT:
		init = 0x1D0F;
		*crc = __crc16_false_false(d, len, init, crc16_poly[0].normal);
		break;

	case CRC16_BUYPASS:
		init = 0x0000;
		*crc = __crc16_false_false(d, len, init, crc16_poly[1].normal);
		break;

	case CRC16_CDMA2000:
		init = 0xFFFF;
		*crc = __crc16_false_false(d, len, init, crc16_poly[2].normal);
		break;

	case CRC16_DDS_110:
		init = 0x800D;
		*crc = __crc16_false_false(d, len, init, crc16_poly[1].normal);
		break;

	case CRC16_DECT_R:
		init = 0x0000;
		*crc = 0x0001 ^ __crc16_false_false(d, len, init, crc16_poly[3].normal);
		break;

	case CRC16_DECT_X:
		init = 0x0000;
		*crc =  __crc16_false_false(d, len, init, crc16_poly[3].normal);
		break;

	case CRC16_DNP:
		init = 0x0000;
		*crc = ~__crc16_true_true(d, len, init, crc16_poly[4].revers);
		break;

	case CRC16_EN_13757:
		init = 0x0000;
		*crc = ~__crc16_false_false(d, len, init, crc16_poly[4].normal);
		break;

	case CRC16_GENIBUS:
		init = 0xFFFF;
		*crc =  ~__crc16_false_false(d, len, init, crc16_poly[0].normal);
		break;

	case CRC16_MAXIM:
		init = 0x0000;
		*crc = ~__crc16_true_true(d, len, init, crc16_poly[1].revers);
		break;

	case CRC16_MCRF4XX:
		init = 0xFFFF;
		*crc = __crc16_true_true(d, len, init, crc16_poly[0].revers);
		break;
	case CRC16_RIELLO:
		init = 0x554D;
		*crc = __crc16_true_true(d, len, init, crc16_poly[0].revers);
		break;

	case CRC16_T10_DIF:
		init = 0x0000;
		*crc = __crc16_false_false(d, len, init, crc16_poly[5].normal);
		break;

	case CRC16_TELEDISK:
		init = 0x0000;
		*crc = __crc16_false_false(d, len, init, crc16_poly[6].normal);
		break;

	case CRC16_TMS37157:
		init = 0x3791;
		*crc = __crc16_true_true(d, len, init, crc16_poly[0].revers);
		break;

	case CRC16_USB:
		init = 0xFFFF;
		*crc = ~__crc16_true_true(d, len, init, crc16_poly[1].revers);
		break;
	case CRC16_A:
		init = 0x6363;
		*crc = __crc16_true_true(d, len, init, crc16_poly[0].revers);
		break;

	case CRC16_KERMIT:
		init = 0x0000;
		*crc = __crc16_true_true(d, len, init, crc16_poly[0].revers);
		break;

	case CRC16_MODBUS:
		init = 0xFFFF;
		*crc = __crc16_true_true(d, len, init, crc16_poly[1].revers);
		break;

	case CRC16_X_25:
		init = 0xFFFF;
		*crc = ~__crc16_true_true(d, len, init, crc16_poly[0].revers);
		break;

	case CRC16_XMODEM:
		init = 0x0000;
		*crc = __crc16_false_false(d, len, init, crc16_poly[0].normal);
		break;
	default:
		return 1;
	}

	return 0;
}

static u32 __crc32_false_false(u8 *d, u32 len, u32 crc, u32 poly)
{
	while (len--) {
		crc ^= *d++ << 24;
		for (u8 i = 0; i < 8; i++)
			crc = crc & 0x80000000 ? (crc << 1) ^ poly : crc << 1;
	}

	return crc;
}

static u32 __crc32_true_true(u8 *d, u32 len, u32 crc, u32 poly)
{
	while (len--) {
		crc ^= *d++;
		for (u8 i = 0; i < 8; i++)
			crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
	}

	return crc;
}

/*  1    E    D    C    6    F    4    1
 * 0001 1110 1101 1100 0110 1111 0100 0001
 * 1000 0010 1111 0110 0011 1011 0111 1000
 *  8    2    F    6    3    B    7    8
 *  */

/*  A    8    3    3    9    8    2    B
 * 1010 1000 0011 0011 1001 1000 0010 1011
 * 1101 0100 0001 1001 1100 1100 0001 0101
 *  D    4    1    9    C    C    1    5
 *  */

/*  8    1    4    1    4    1    A    B
 * 1000 0001 0100 0001 0100 0001 1010 1011
 * 1101 0101 1000 0010 1000 0010 1000 0001
 *  D    5    8    2    8    2    8    1
 *  */

static struct crc32_poly crc32_poly[] = {
	[0] = {
		.normal = 0x04C11DB7,
		.revers = 0xEDB88320,
	},
	[1] = {
		.normal = 0x1EDC6F41,
		.revers = 0x82F63B78,
	},
	[2] = {
		.normal = 0xA833982B,
		.revers = 0xD419CC15,
	},
	[3] = {
		.normal = 0x814141AB,
		.revers = 0xD5828281,
	},
	[4] = {
		.normal = 0x000000AF,
		.revers = 0xF5000000,
	},
};

int esma_crc32(u8 *d, u32 len, u32 *crc, u32 name)
{
	u32 init;

	switch (name) {
	case CRC32_BASIC:
		init = 0xFFFFFFFF;
		*crc = ~__crc32_true_true(d, len, init, crc32_poly[0].revers);
		break;

	case CRC32_BZIP2:
		init = 0xFFFFFFFF;
		*crc = ~__crc32_false_false(d, len, init, crc32_poly[0].normal);
		break;

	case CRC32_C:
		init = 0xFFFFFFFF;
		*crc = ~__crc32_true_true(d, len, init, crc32_poly[1].revers);
		break;

	case CRC32_D:
		init = 0xFFFFFFFF;
		*crc = ~__crc32_true_true(d, len, init, crc32_poly[2].revers);
		break;

	case CRC32_MPEG2:
		init = 0xFFFFFFFF;
		*crc = __crc32_false_false(d, len, init, crc32_poly[0].normal);
		break;

	case CRC32_POSIX:
		init = 0x00000000;
		*crc = ~__crc32_false_false(d, len, init, crc32_poly[0].normal);
		break;

	case CRC32_Q:
		init = 0x00000000;
		*crc = __crc32_false_false(d, len, init, crc32_poly[3].normal);
		break;

	case CRC32_JAMCRC:
		init = 0xFFFFFFFF;
		*crc = __crc32_true_true(d, len, init, crc32_poly[0].revers);
		break;

	case CRC32_XFER:
		init = 0x00000000;
		*crc = __crc32_false_false(d, len, init, crc32_poly[4].normal);
		break;

	default:
		return 1;
	}

	return 0;
}
