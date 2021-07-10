
#ifndef ESMA_SERIAL_PORT_H
#define ESMA_SERIAL_PORT_H

#include "esma_logger.h"

struct esma_serial_port_options {
	int baudrate;
	int databits;
	int stopbits;
	char parity;
};

struct esma_serial_port {
	int fd;
	char *dev;
};

int esma_serial_port_init(struct esma_serial_port *port, char *dev);
int esma_serial_port_open(struct esma_serial_port *port);
int esma_serial_port_setup(struct esma_serial_port *port, struct esma_serial_port_options *options);
int esma_serial_port_close(struct esma_serial_port *port);
int esma_serial_port_clear(struct esma_serial_port *port);

#endif
