
#ifndef ESMA_SERIAL_PORT_H
#define ESMA_SERIAL_PORT_H

#include <termios.h>

struct esma_serial_port {
	int fd;

	u32 baudrate;
	u8  databits;
	u8  parity;
	u8  stopbits;
	u8  flow;

	u8  status;

	struct termios old_term;
	char *dev;
};

int esma_serial_port_init(struct esma_serial_port *port, char *dev);
int esma_serial_port_open(struct esma_serial_port *port);

int esma_serial_port_set_baudrate(struct esma_serial_port *port, u32);
int esma_serial_port_set_databits(struct esma_serial_port *port, u8);
int esma_serial_port_set_parity(struct esma_serial_port *port, u8);
int esma_serial_port_set_stopbits(struct esma_serial_port *port, u8);
int esma_serial_port_set_flow(struct esma_serial_port *port, u8);
int esma_serial_port_set_raw_output(struct esma_serial_port *port);

int esma_serial_port_clear(struct esma_serial_port *port);
int esma_serial_port_close(struct esma_serial_port *port);

#endif
