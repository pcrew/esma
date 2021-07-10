
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>


#include "common/compiler.h"
#include "esma_serial_port.h"

int esma_serial_port_init(struct esma_serial_port *port, char *dev)
{
	if (NULL == port) {
		esma_core_log_err("%s() - port is NULL\n", __func__);
		return 1;
	}

	port->dev = dev;
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
		esma_core_log_sys("%s() - oprn('%s') failed.\n", __func__, port->dev);
		return 1;
	}

	err = tcflush(fd, TCIOFLUSH);
	if (unlikely(err)) {
		esma_core_log_sys("%s() - tcflush('%s', TCIOFLUSH) failed.\n", __func__, port->dev);
		return 1;
	}

	port->fd = fd;
	return 0;
}

int esma_serial_port_setup(struct esma_serial_port *port, struct esma_serial_port_options *options)
{
	struct termios termios;
	speed_t baudrate;
	tcflag_t data;
	int err;

	if (unlikely(NULL == port || NULL == options)) {
		esma_core_log_err("%s() - port or options is NULL\n", __func__);
		return 1;
	}

	if (unlikely(-1 == port->fd)) {
		esma_core_log_err("%s() - port: invalid fd.\n", __func__);
		return 1;
	}

	switch (options->baudrate) {
	case 2400:
		baudrate = B2400;
		break;
	case 4800:
		baudrate = B4800;
		break;
	case 9600:
		baudrate = B9600;
		break;
	case 19200:
		baudrate = B19200;
		break;
	case 38400:
		baudrate = B38400;
		break;
	case 57600:
		baudrate = B57600;
		break;
	case 115200:
		baudrate = B115200;
		break;
	default:
		esma_core_log_err("%s()/%s - bad baudrate: %d\n", __func__, port->dev, options->baudrate);
		return 1;
	}

	cfsetispeed(&termios, baudrate);
	cfsetospeed(&termios, baudrate);

	switch (options->databits) {
	case 5:
		data = CS5;
		break;
	case 6:
		data = CS6;
		break;
	case 7:
		data = CS7;
		break;
	case 8:
		data = CS8;
		break;
	default:
		esma_core_log_err("%s()/%s - bad databits: %d\n", __func__, options->databits);
		return 1;
	}

	termios.c_cflag &= ~CSIZE;
	termios.c_cflag |= data;

	switch (options->parity) {
	case 'n':
	case 'N':
		termios.c_cflag &= ~(PARENB | PARODD);
		break;
	case 'e':
	case 'E':
		termios.c_cflag |= PARENB;
		break;
	case 'o':
	case 'O':
		termios.c_cflag |= PARODD;
		break;
	default:
		esma_core_log_err("%s() - bad parity: '%c'\n", __func__, options->parity);
		return 1;
	}

	switch (options->stopbits) {
	case 1:
		termios.c_cflag &=~CSTOPB;
		break;
	case 2:
		termios.c_cflag |= CSTOPB;
		break;
	default:
		esma_core_log_err("%s() - bad stopbits: %d\n", __func__, options->stopbits);
		return 1;
	}

	termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	termios.c_cflag |= CLOCAL | CREAD;
	termios.c_cflag |= ~CRTSCTS;

	termios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);
	termios.c_oflag &= ~OPOST;

	/* need??? */
	termios.c_lflag = 0;
	termios.c_oflag = 0;

	err = tcsetattr(port->fd, TCSANOW, &termios);
	if (err) {
		esma_core_log_err("%s() - tcsetattr() failed\n", __func__);
		return 1;
	}

	return 0;
}

int esma_serial_port_close(struct esma_serial_port *port)
{
	if (NULL == port) {
		return 0;
	}

	if (port->fd)
		return 0;

	return close(port->fd);
}

int esma_serial_port_clear(struct esma_serial_port *port)
{
	if (NULL == port)
		return 0;

	memset(port, 0, sizeof(struct esma_serial_port));
	return 0;
}
