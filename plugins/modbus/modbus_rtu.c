
#include <stdlib.h>
#include <unistd.h>

#include "modbus.h"
#include "common/compiler.h"
#include "core/esma_logger.h"

int modbus_rtu_init(struct modbus *device, struct esma_serial_port *port)
{
	if (unlikely(NULL == device || NULL == port)) {
		esma_plugin_log_err("%s() - device or port is NULL.\n", __func__);
		return 1;
	}

	device->type = MODBUS_RTU;
	device->device.rtu.port = *port;

	return 0;
}

int modbus_rtu_open(struct modbus *device)
{
	if (unlikely(NULL == device)) {
		esma_plugin_log_err("%s() - device is NULL\n", __func__);
		return 1;
	}

	if (device->device.rtu.port.fd < 1)
		return esma_serial_port_open(&device->device.rtu.port);

	return 0;
}

int modbus_rtu_setup(struct modbus *device, struct esma_serial_port_options *options)
{
	if (unlikely(NULL == device || NULL == options)) {
		esma_plugin_log_err("%s() - device or options is NULL\n", __func__);
		return 1;
	}

	return esma_serial_port_setup(&device->device.rtu.port, options);
}
