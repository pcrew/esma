
#ifndef MODBUS_H
#define MODBUS_H

#include "common/api.h"
#include "core/esma_dbuf.h"
#include "core/esma_serial_port.h"

/* Modbus function codes */
#define MODBUS_READ_COILS			0x01
#define MODBUS_READ_DISCRETE_INPUTS		0x02

#define MODBUS_WRITE_SINGLE_COIL		0x05
#define MODBUS_WRITE_MULTIPLE_COILS		0x0F

#define MODBUS_READ_INPUT_REGISTERS		0x04
#define MODBUS_READ_HOLDING_REGISTERS		0x03

#define MODBUS_WRITE_SINGLE_REGISTER		0x06

#define MODBUS_WRITE_MULTIPLE_REGISTERS		0x10
#define MODBUS_READ_AND_WRITE_REGISTERS		0x17
#define MODBUS_MASK_WRITE_REGISTERS		0x16

#define ESMA_MDOBUS_READ_FIFO_QUEUE		0x24
#define ESMA_MDOBUS_READ_FILE_QUEUE		0x20

#define MODBUS_WRITE_FILE_RECORD		0x21
#define MODBUS_READ_EXCEPTION_STATUS		0x07
#define MODBUS_DIAGNOSTIC			0x08

#define MODBUS_GET_COM_EVENT_COUNTER		0x11
#define MODBUS_GET_COM_EVENT_LOG		0x12


#define MODBUS_REPORT_SLAVE_ID			0x17
#define MODBUS_READ_DEVICE_IDENTIFICATION	0x43
/*  */

#define MODBUS_PDU_MAX_SIZE			253
#define MODBUS_ADU_MAX_SIZE			260

#define MODBUS_RTU	0
#define MODBUS_TCP	1

struct modbus;
struct modbus_rtu;

api_declaration(modbus_api) {
	int (*send)(struct modbus *mb, struct esma_dbuf *buf);
	int (*recv)(struct modbus *mb, struct esma_dbuf *buf);
	int (*check)(struct modbus *mv, u8 *buf, u32 buf_len);	


	int (*open)(struct modbus *mb);
	int (*flush)(struct modbus *mb);
	int (*close)(struct modbus *mb);
};

struct modbus_rtu {
	struct esma_serial_port 	port;
	struct esma_serial_port_options	port_options;
};

struct modbus {
	u8 slave_id;
	u8 type;
	union {
		struct modbus_rtu rtu;
	} device;

	struct esma_dbuf ibuf;
	struct esma_dbuf obuf;
	struct modbus_api *api;
};

int modbus_rtu_init(struct modbus *mb, struct esma_serial_port *port);
int modbus_rtu_open(struct modbus *mb);
int modbus_rtu_setup(struct modbus *mv, struct esma_serial_port_options *o);

#endif
