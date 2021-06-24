
#include <string.h>
#include <arpa/inet.h> /* for htons */

#include "esma_crc.h"

struct crc16_poly {
	u16 normal;
	u16 revers;
};

struct crc32_poly {
	u32 normal;
	u32 revers;
};


static void __crc16_false_false(u8 *d, u32 len, u16 crc, u16 poly)
{
	u8 i;

	while (len--) {	
		crc ^= *d++ << 8;
		for (i = 0; i < 8; i++)
			crc = crc & 0x8000 ? (crc << 1) ^ poly : crc << 1;
	}
}

static void __crc16_true_true(u8 *d, u32 len, u16 crc, u16 poly)
{
	u8 i;

	while (len--) {
		crc ^= *d++;
		for (i = 0; i < 8; i++)
			crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
	}
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

u16 esma_crc16(u8 *d, u32 len, u32 name)
{
	u16 crc;
	switch (name) {

		case ARC:
			crc = 0x0000;
			__crc16_true_true(d, len, crc, crc16_poly[1].revers);
			return crc;

		case AUG_CCITT:
			crc = 0x1D0F;
			__crc16_false_false(d, len, crc, crc16_poly[0].normal);
			return crc;

		case BUYPASS:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[1].normal);
			return crc;

		case CDMA2000:
			crc = 0xFFFF;
			__crc16_false_false(d, len, crc, crc16_poly[2].normal);
			return crc;

		case DDS_110:
			crc = 0x800D;
			__crc16_false_false(d, len, crc, crc16_poly[1].normal);
			return crc;

		case DECT_R:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[3].normal);
			return crc ^ 0x0001;

		case DECT_X:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[3].normal);
			return crc;

		case DNP:
			crc = 0x0000;
			__crc16_true_true(d, len, crc, crc16_poly[4].revers);
			return ~crc;

		case EN_13757:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[4].normal);
			return ~crc;

		case GENIBUS:
			crc = 0xFFFF;
			__crc16_false_false(d, len, crc, crc16_poly[0].normal);
			return ~crc;

		case MAXIM:

			crc = 0x0000;
			__crc16_true_true(d, len, crc, crc16_poly[1].revers);
			return ~crc;

		case MCRF4XX:
			crc = 0xFFFF;
			__crc16_true_true(d, len, crc, crc16_poly[0].revers);
			return crc;

		case RIELLO:
			/*  B    2    A    A
			 * 1011 0010 1010 1010 - normal
			 * 0101 0101 0100 1101 - reversed
			 *  5    5    4    D
			 * */
			crc = 0x554D;
			__crc16_true_true(d, len, crc, crc16_poly[0].revers);
			return crc;

		case T10_DIF:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[5].normal);
			return crc;

		case TELEDISK:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[6].normal);
			return crc;

		case TMS37157:
			crc = 0x3791; /* reversed 0x89EX */
			__crc16_true_true(d, len, crc, crc16_poly[0].revers);
			return crc;

		case USB:
			crc = 0xFFFF;
			__crc16_true_true(d, len, crc, crc16_poly[1].revers);
			return ~crc;

		case A:
			/*  C    6    C    6
			 * 1100 0110 1100 0110 - normal
			 * 0110 0011 0110 0011 - reversed
			 *  6    3    6    3
			 *  */
			crc = 0x6363;
			__crc16_true_true(d, len, crc, crc16_poly[0].revers);
			return crc;

		case KERMIT:
			crc = 0x0000;
			__crc16_true_true(d, len, crc, crc16_poly[0].revers);
			return crc;

		case MODBUS:
			crc = 0xFFFF;
			__crc16_true_true(d, len, crc, crc16_poly[1].revers);
			return crc;

		case X_25:
			crc = 0xFFFF;
			__crc16_true_true(d, len, crc, crc16_poly[0].revers);
			return ~crc;

		case XMODEM:
			crc = 0x0000;
			__crc16_false_false(d, len, crc, crc16_poly[0].normal);
			return crc;

		default:
			return 0;
	}
}

static void __crc32_false_false(u8 *d, u32 len, u32 crc, u32 poly)
{
	u8 i;

	while (len--) {
		crc ^= *d++ << 24;
		for (i = 0; i < 8; i++)
			crc = crc & 0x80000000 ? (crc << 1) ^ poly : crc << 1;
	}
}

static void __crc32_true_true(u8 *d, u32 len, u32 crc, u32 poly)
{
	u8 i;

	while (len--) {
		crc ^= *d++;
		for (i = 0; i < 8; i++)
			crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
	}
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

u32 esma_crc32(u8 *d, u32 len, u32 name)
{
	u32 crc;

	switch (name) {

		case BASIC:
			crc = 0xFFFFFFFF;
			__crc32_true_true(d, len, crc, crc32_poly[0].revers);
			return ~crc;

		case BZIP2:
			crc = 0xFFFFFFFF;
			__crc32_false_false(d, len, crc, crc32_poly[0].normal);
			return ~crc;

		case C:
			crc = 0xFFFFFFFF;
			__crc32_true_true(d, len, crc, crc32_poly[1].revers);
			return ~crc;

		case D:
			crc = 0xFFFFFFFF;
			__crc32_true_true(d, len, crc, crc32_poly[2].revers);
			return ~crc;

		case MPEG2:
			crc = 0xFFFFFFFF;
			__crc32_false_false(d, len, crc, crc32_poly[0].normal);
			return crc;

		case POSIX:
			crc = 0x00000000;
			__crc32_false_false(d, len, crc, crc32_poly[0].normal);
			return ~crc;

		case Q:
			crc = 0x00000000;
			__crc32_false_false(d, len, crc, crc32_poly[3].normal);
			return crc;

		case JAMCRC:
			crc = 0xFFFFFFFF;
			__crc32_true_true(d, len, crc, crc32_poly[0].revers);
			return crc;

		case XFER:
			crc = 0x00000000;
			__crc32_false_false(d, len, crc, crc32_poly[4].normal);
			return crc;

		default:
			return 0;
	}
}
