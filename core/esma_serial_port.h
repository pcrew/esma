/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_SERIAL_PORT_H
#define ESMA_SERIAL_PORT_H

#include <termios.h>

/**
 * @brief Structure to represent serial port.
 */
struct esma_serial_port {
	int fd;				/**< The file descriptor. */

	u32 baudrate;			/**< Baudrate. */
	u8  databits;			/**< Data bits. */
	u8  parity;			/**< Parity. */
	u8  stopbits;			/**< Stop bits. */
	u8  flow;			/**< Flow. */

	u8  status;			/**< Current status: opened or not. */

	struct termios old_term;	/**< Old termios settings. */
	char *dev;			/**< Pointer to the device name. */
};

/**
 * @brief Initialises serial port.
 * @param [out] port	Pointer to the serial port.
 * @param [in] dev	Device name.
 * @return 0 if serial port initilised successfuly; 1 otherwise.
 */
int esma_serial_port_init(struct esma_serial_port *port, char *dev);

/**
 * @brief Opens serial port.
 * @param [out] port	Pointer to the serial port.
 * @return 0 if serial port opened successfuly; 1 otherwise.
 */
int esma_serial_port_open(struct esma_serial_port *port);

/**
 * @brief Sets baudrate for the serial port.
 * @param [out] port		Pointer to the serial port.
 * @param [in] baudrate		Baudrate.
 * @return 0 if baudrate seted successfuly; 1 otherwise.
 */
int esma_serial_port_set_baudrate(struct esma_serial_port *port, u32 baudrate);

/**
 * @brief Sets data bits for the serial port.
 * @param [out] port		Pointer to the serial port.
 * @param [in] databits		Databits.
 * @return 0 if data bits seted successfuly; 1 otherwise.
 */
int esma_serial_port_set_databits(struct esma_serial_port *port, u8 databits);

/**
 * @brief Sets parity for the serial port.
 * @param [out] port		Pointer to the serial port.
 * @param [in] parity		Parity.
 * @return 0 if parity seted successfuly; 1 otherwise.
 */
int esma_serial_port_set_parity(struct esma_serial_port *port, u8 parity);

/**
 * @brief Sets stop bits for the serial port.
 * @param [out] port		Pointer to the serial port.
 * @param [in] stop_bits	Stop bits.
 * @return 0 if stop bits seted successfuly; 1 otherwise.
 */
int esma_serial_port_set_stopbits(struct esma_serial_port *port, u8 stopbits);

/**
 * @brief Sets flow for the serial port.
 * @param [out] port	Pointer to the serial port.
 * @param [in] flow	Flow type.
 * @return 0 if flow seted successfuly; 1 otherwise.
 */
int esma_serial_port_set_flow(struct esma_serial_port *port, u8 flow);

/**
 * @brief Set raw output for the serial port.
 * @param [out] port	Pointer to the serial port.
 * @return 0 if raw output seted successfuly; 1 otherwise.
 */
int esma_serial_port_set_raw_output(struct esma_serial_port *port);

/**
 * @brief Clears serial port.
 * @param [out] port	Pointer to the serial port.
 * @return 0 if serial port cleared successfuly; 1 otherwise.
 */
int esma_serial_port_clear(struct esma_serial_port *port);

/**
 * @brief Closes serial port.
 * @param [out] port	Pointer to the serial port.
 * @return 0 if serial port closed successfuly; 1 otherwise.
 */
int esma_serial_port_close(struct esma_serial_port *port);

#endif
