
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "common/compiler.h"
#include "core/esma_logger.h"
#include "esma_serial_port.h"

static int check_status(struct esma_serial_port *port)
{
	if (unlikely(NULL == port)) {
		esma_core_log_err("%s() - port is NULL.\n", __func__);
		return 1;
	}

	if (unlikely(0 == port->status)) {
		esma_core_log_err("%s() - port not open.\n", __func__);
		return 1;
	}

	return 0;
}

#define GET_TERMIOS(p, t)									\
if (tcgetattr(p->fd, t) < 0) {									\
	esma_core_log_sys("%s() - tcgetattr('%s': '%d'): failed\n", __func__, p->dev, p->fd);	\
	return 1;										\
}

#define SET_TERMIOS(p, t)									\
if (tcsetattr(p->fd, TCSANOW, t) < 0) {								\
	esma_core_log_sys("%s() - tcsetattr('%s': '%d'): failed\n", __func__, p->dev, p->fd);	\
	return 1;										\
}

int esma_serial_port_init(struct esma_serial_port *port, char *device)
{
	if (NULL == port) {
		esma_core_log_err("%s() - port is NULL\n", __func__);
		return 1;
	}

	if (NULL == device) {
		esma_core_log_err("%s() - device is NULL.\n", __func__);
		return 1;
	}

	port->dev = device;
	return 0;
}

int esma_serial_port_open(struct esma_serial_port *port)
{
	int fd;
	int err;

	if (unlikely(NULL == port)) {
		esma_core_log_err("%s() - port is NULL\n", __func__);
		return 1;
	}

	fd = open(port->dev, O_RDWR | O_NOCTTY);
	if (unlikely(-1 == fd)) {
		esma_core_log_sys("%s() - open('%s') failed.\n", __func__, port->dev);
		return 1;
	}

	err = tcflush(fd, TCIOFLUSH);
	if (unlikely(err)) {
		esma_core_log_sys("%s() - tcflush('%s', TCIOFLUSH) failed.\n", __func__, port->dev);
		return 1;
	}

	GET_TERMIOS(port, &port->old_term);

	port->fd = fd;
	port->status = 1;
	return 0;
}

int esma_serial_port_set_baudrate(struct esma_serial_port *port, u32 baudrate)
{
	struct termios termios;
	speed_t speed;
	int err;

	err = check_status(port);
	if (err)
		return 1;

	GET_TERMIOS(port, &termios);

	switch (baudrate) {
	case 110:
		speed = B110;
		break;
	case 300:
		speed = B300;
		break;
	case 600:
		speed = B600;
		break;
	case 1200:
		speed = B1200;
		break;
	case 2400:
		speed = B2400;
		break;
	case 4800:
		speed = B4800;
		break;
	case 9600:
		speed = B9600;
		break;
	case 19200:
		speed = B19200;
		break;
	case 38400:
		speed = B38400;
		break;
	#ifdef B57600
	case 57600:
		speed = B57600;
		break;
	#endif
	#ifdef B115200
	case 115200:
		speed = B115200;
		break;
	#endif
	#ifdef B230400
	case 230400:
		speed = B230400;
		break;
	#endif
	#ifdef B460800
	case 460800:
		speed = B460800;
		break;
	#endif
	#ifdef B500000
	case 500000:
		speed = B500000;
		break;
	#endif
	#ifdef B576000
	case 576000:
		speed = B576000;
		break;
	#endif
	#ifdef B921600
	case 921600:
		speed = B921600;
		break;
	#endif
	#ifdef B1000000
	case 1000000:
		speed = B1000000;
		break;
	#endif
	#ifdef B1152000
	case 1152000:
		speed = B1152000;
		break;
	#endif
	#ifdef B1500000
	case 1500000:
		speed = B1500000;
		break;
	#endif
	#ifdef B2500000
	case 2500000:
		speed = B2500000;
		break;
	#endif
	#ifdef B3000000
	case 3000000:
		speed = B3000000;
		break;
	#endif
	#ifdef B3500000
	case 3500000:
		speed = B3500000;
		break;
	#endif
	#ifdef B4000000
	case 4000000:
		speed = B4000000;
		break;
	#endif
	default:
		esma_core_log_err("%s()/%s - bad baudrate: %d\n", __func__, port->dev, baudrate);
		return 1;
	}

	cfsetispeed(&termios, speed);
	cfsetospeed(&termios, speed);

	SET_TERMIOS(port, &termios);
	port->baudrate = baudrate;

	return 0;
}

int esma_serial_port_set_databits(struct esma_serial_port *port, u8 databits)
{
	struct termios termios;
	int err;

	err = check_status(port);
	if (err)
		return 1;

	GET_TERMIOS(port, &termios);

	termios.c_cflag &= ~CSIZE;
	switch (databits) {
	case 5:
		termios.c_cflag |= CS5;
		break;
	case 6:
		termios.c_cflag |= CS6;
		break;
	case 7:
		termios.c_cflag |= CS7;
		break;
	case 8:
		termios.c_cflag |= CS8;
		break;
	default:
		esma_core_log_err("%s()/%s - bad databits: %d\n", __func__, port->dev, databits);
		return 1;
	}

	SET_TERMIOS(port, &termios);
	port->databits = databits;

	return 0;
}

int esma_serial_port_set_parity(struct esma_serial_port *port, u8 parity)
{
	struct termios termios;
	int err;

	err = check_status(port);
	if (err)
		return 1;

	GET_TERMIOS(port, &termios);

	switch (parity) {
	case 'n':
	case 'N':
		termios.c_cflag &= ~PARENB;
		termios.c_iflag &= ~INPCK;
		break;
	case 'e':
	case 'E':
		termios.c_cflag &= ~PARODD;
		termios.c_cflag |= PARENB;
		termios.c_iflag |= INPCK;
		break;
	case 'o':
	case 'O':
		termios.c_cflag |= PARENB;
		termios.c_cflag |= PARODD;
		termios.c_iflag |= INPCK;
		break;
	default:
		esma_core_log_err("%s()/%s - bad parity: '%c'\n", __func__, port->dev, parity);
		return 1;
	}

	SET_TERMIOS(port, &termios);
	port->parity = parity;

	return 0;
}

int esma_serial_port_set_stopbits(struct esma_serial_port *port, u8 stopbits)
{
	struct termios termios;
	int err;

	err = check_status(port);
	if (err)
		return 1;

	GET_TERMIOS(port, &termios);

	switch (stopbits) {
	case 1:
		termios.c_cflag &=~CSTOPB;
		break;
	case 2:
		termios.c_cflag |= CSTOPB;
		break;
	default:
		esma_core_log_err("%s()/%s - bad stopbits: %d\n", __func__, port->dev, stopbits);
		return 1;
	}

	SET_TERMIOS(port, &termios);
	port->stopbits = stopbits;

	return 0;
}

int esma_serial_port_set_flow(struct esma_serial_port *port, u8 flow)
{
	struct termios termios;
	int err;

	err = check_status(port);
	if (err)
		return 1;

	GET_TERMIOS(port, &termios);

	switch (flow) {
	case 0:
		termios.c_cflag &= ~CRTSCTS;
		termios.c_iflag &= ~(IXON | IXOFF | IXANY);
		break;
	case 1:
		termios.c_cflag |= CRTSCTS;
		termios.c_iflag &= ~(IXON | IXOFF | IXANY);
		break;
	case 2:
		termios.c_cflag &= ~CRTSCTS;
		termios.c_iflag |= (IXON | IXOFF | IXANY);
		break;
	default:
		esma_core_log_err("%s()/%s - bad flow: '%c'\n", __func__, port->dev, flow);
		return 1;
	}

	SET_TERMIOS(port, &termios);
	port->flow = flow;

	return 0;
}

int esma_serial_port_set_raw_output(struct esma_serial_port *port)
{
	struct termios termios;
	int err;

	err = check_status(port);
	if (err)
		return 1;

	GET_TERMIOS(port, &termios);
	termios.c_oflag &= ~OPOST;
	SET_TERMIOS(port, &termios);

	return 0;
}

int esma_serial_port_close(struct esma_serial_port *port)
{
	if (NULL == port) {
		return 0;
	}

	if (0 == port->status)
		return 0;

	port->status = 0;
	return close(port->fd);
}

int esma_serial_port_clear(struct esma_serial_port *port)
{
	if (NULL == port)
		return 0;

	memset(port, 0, sizeof(struct esma_serial_port));
	return 0;
}
