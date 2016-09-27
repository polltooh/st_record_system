#include "gloop_measure_spi_interface.h"
//#include "Windows.h"
#include "CyUSBSerial.h"
#include <string.h>
#include <unistd.h>
/*GPIO to use for manual chip select*/
#define SW_NSS_GPIO	(7U)

typedef struct
{
	uint8_t device_index;
	CY_HANDLE handle;
	bool initialized;
	void* unused;
} spi_context_private_t;

typedef struct
{
	uint16_t vid;
	uint16_t pid;
	uint32_t spi_frequency_hz;
	bool spi_clock_polarity;
	bool spi_clock_phase;
	uint8_t spi_select_address;
	spi_context_private_t impl;
} spi_context_impl_t;

static CY_RETURN_STATUS readwrite_single_spi_words(CY_HANDLE handle, uint8_t* read_buffer,
	uint8_t* write_buffer, size_t length);


int32_t gloop_create_default_context(spi_context_t* context)
{
	return
	gloop_create_context(GLOOP_DEFAULT_VID, GLOOP_DEFAULT_PID, GLOOP_DEFAULT_FREQ_HZ, GLOOP_DEFAULT_POLARITY,
						 GLOOP_DEFAULT_PHASE, GLOOP_DEFAULT_SELECT_ADDR, (void*) context);
}

/* \brief Provide with NULL memory pointer to get allocation size needed */
int32_t gloop_create_context(uint16_t vid, uint16_t pid, uint32_t spi_frequency_hz,
	bool spi_clock_polarity, bool spi_clock_phase, uint8_t spi_select_address,
	void* memory)
{
	CY_DEVICE_INFO cy_device_info_array[16];
	UINT8 number_of_devices_found, device_ids_found[16];
	CY_SPI_CONFIG cy_spi_config;
	CY_VID_PID cy_vid_pid;
	CY_RETURN_STATUS status = 0U;
	int i;

	spi_context_impl_t* context;
	if (memory == NULL)
	{
		return sizeof(spi_context_impl_t);
	}

	memset(memory, 0U, sizeof(spi_context_impl_t));
	context = (spi_context_impl_t*) memory;
	context->vid = vid;
	context->pid = pid;
	context->spi_frequency_hz = spi_frequency_hz;
	context->spi_clock_phase = spi_clock_phase;
	context->spi_clock_polarity = spi_clock_polarity;
	context->spi_select_address = spi_select_address;

	cy_vid_pid.pid = pid;
	cy_vid_pid.vid = vid;
	CyLibraryInit();
	status = CyGetDeviceInfoVidPid(cy_vid_pid, device_ids_found, (PCY_DEVICE_INFO) &cy_device_info_array, 
		&number_of_devices_found, sizeof(device_ids_found) / sizeof(device_ids_found[0]));

	if ((status != CY_SUCCESS) || (number_of_devices_found == 0))
	{
		return -1;
	}

	status = CY_SUCCESS + 1;

	for (i = 0; i < number_of_devices_found; ++i)
	{
		if (1)//(cy_device_info_array[i].deviceClass == SerialBlock_SCB0)
		{
			context->impl.device_index = i;
			status = CyOpen(, 0, &context->impl.handle);
			break;
		}
	}

	if (status != CY_SUCCESS)
	{
		return -1;
	}

	status = CyGetSpiConfig(context->impl.handle, &cy_spi_config);

	cy_spi_config.isMaster = true;
	cy_spi_config.dataWidth = 8;
	cy_spi_config.isContinuousMode = true;
	cy_spi_config.isSelectPrecede = false;
	cy_spi_config.isMsbFirst = 1;
	cy_spi_config.protocol = CY_SPI_MOTOROLA;
	cy_spi_config.frequency = spi_frequency_hz;
	cy_spi_config.isCpol = (spi_clock_polarity == 1);
	cy_spi_config.isCpha = (spi_clock_phase == 1);

	status |= CySetSpiConfig(context->impl.handle, &cy_spi_config);

	if (status != CY_SUCCESS)
	{
		return -1;
	}

	return 0;
}

bool gloop_free_resources(spi_context_t* context)
{
	spi_context_impl_t* impl = (spi_context_impl_t*)context;
	return (CyClose(impl->impl.handle) == CY_SUCCESS);
}

bool gloop_send_config_cmd(spi_context_t* context, uint8_t odr, uint8_t trgr_mode)
{
	CY_RETURN_STATUS status;
	CY_DATA_BUFFER cy_read_buf, cy_write_buf;
	gloop_configuration_spicmd_t read_struct, write_struct;
	spi_context_impl_t* impl = (spi_context_impl_t*) context;

	cy_read_buf.buffer = read_struct.buf;
	cy_read_buf.length = sizeof(read_struct);
	cy_write_buf.buffer = write_struct.buf;
	cy_write_buf.length = sizeof(write_struct);
	memset(&write_struct, 0U, sizeof(write_struct));
	memset(&read_struct, 0U, sizeof(write_struct));

	write_struct.message_type = GLOOP_MSGTYPE_CONF;
	write_struct.odr = odr;
	write_struct.trgr_mode = trgr_mode;

	status = CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 0);
	status |= CySpiReadWrite(impl->impl.handle, &cy_read_buf, &cy_write_buf, 100);
	status |= CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 1);


	return (status == CY_SUCCESS);
}
bool gloop_send_config_cmd2(spi_context_t* context, uint8_t odr, uint8_t trgr_mode,
	uint8_t filter, uint8_t resolution)
 {
	CY_RETURN_STATUS status;
	CY_DATA_BUFFER cy_read_buf, cy_write_buf;
	gloop_configuration_spicmd2_t read_struct, write_struct;
	spi_context_impl_t* impl = (spi_context_impl_t*) context;

	cy_read_buf.buffer = read_struct.buf;
	cy_read_buf.length = sizeof(read_struct);
	cy_write_buf.buffer = write_struct.buf;
	cy_write_buf.length = sizeof(write_struct);
	memset(&write_struct, 0U, sizeof(write_struct));
	memset(&read_struct, 0U, sizeof(write_struct));

	write_struct.message_type = GLOOP_MSGTYPE_CONF;
	write_struct.odr = odr;
	write_struct.trgr_mode = trgr_mode;
	write_struct.filter = filter;
	write_struct.resolution = resolution;

	status = CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 0);
	status |= CySpiReadWrite(impl->impl.handle, &cy_read_buf, &cy_write_buf, 100);
	status |= CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 1);


	return (status == CY_SUCCESS);
}

bool gloop_read_data(spi_context_t* context, gloop_data_spidat_t* data_out)
{
	CY_RETURN_STATUS status;
	CY_DATA_BUFFER cy_read_buf, cy_write_buf;
	gloop_data_spidat_t  write_struct;
	spi_context_impl_t* impl = (spi_context_impl_t*) context;

	cy_read_buf.buffer = (uint8_t*)data_out;
	cy_read_buf.length = sizeof(write_struct);
	cy_write_buf.buffer = write_struct.buf;
	cy_write_buf.length = sizeof(write_struct);
	memset(&write_struct, 0U, sizeof(gloop_data_spidat_t));
	memset(&data_out, 0U, sizeof(gloop_data_spidat_t));

	write_struct.message_type = GLOOP_MSGTYPE_DATA;

	status = CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 0);
	usleep(10);
	status |= CySpiReadWrite(impl->impl.handle, &cy_read_buf, &cy_write_buf, 100);
	usleep(10);
	status |= CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 1);
	usleep(10);

	return (status == CY_SUCCESS);
}

bool gloop_read_data2(spi_context_t* context, gloop_data_spidat2_t* data_out)
{
	CY_RETURN_STATUS status;
	CY_DATA_BUFFER cy_read_buf, cy_write_buf;
	gloop_data_spidat2_t  write_struct;
	spi_context_impl_t* impl = (spi_context_impl_t*) context;

	cy_read_buf.buffer = (uint8_t*) data_out;
	cy_read_buf.length = sizeof(write_struct);
	cy_write_buf.buffer = write_struct.buf;
	cy_write_buf.length = sizeof(write_struct);
	memset(&write_struct, 0U, sizeof(write_struct));
	memset(&data_out, 0U, sizeof(write_struct));

	write_struct.message_type = GLOOP_MSGTYPE_DATA;

	status = CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 0);
	usleep(10);
	status |= CySpiReadWrite(impl->impl.handle, &cy_read_buf, &cy_write_buf, 100);
	usleep(10);
	status |= CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 1);
	usleep(10);

	return (status == CY_SUCCESS);
}

//// Below here debuggging code

bool gloop_read_data_slow(spi_context_t* context, gloop_data_spidat_t* data_out)
{
	
	CY_RETURN_STATUS status;
	
	gloop_data_spidat_t  write_struct;
	spi_context_impl_t* impl = (spi_context_impl_t*) context;

	memset(&write_struct, 0U, sizeof(write_struct));


	write_struct.message_type = GLOOP_MSGTYPE_DATA;

	
	{
		status = CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 0);
		status |= readwrite_single_spi_words(impl->impl.handle, (uint8_t*)data_out, (uint8_t*) &write_struct,
			sizeof(write_struct));
		status |= CySetGpioValue(impl->impl.handle, SW_NSS_GPIO, 1);
	}

	return (status == CY_SUCCESS);
}

static CY_RETURN_STATUS readwrite_single_spi_words(CY_HANDLE handle, uint8_t* read_buffer,
	                                               uint8_t* write_buffer, size_t length)
{
	size_t i;
	CY_RETURN_STATUS status = CY_SUCCESS;
	CY_DATA_BUFFER cy_read_buf, cy_write_buf;


	cy_read_buf.buffer = read_buffer;
	cy_read_buf.length = 1;
	cy_write_buf.buffer = write_buffer;
	cy_write_buf.length = 1;
	memset(read_buffer, 0U, length);
	usleep(10);
	for (i = 0; i < length; ++i)
	{
		status = CySpiReadWrite(handle, &cy_read_buf, &cy_write_buf, 100);
		usleep(1);
		cy_read_buf.buffer++;
		cy_read_buf.length = 1;
		cy_read_buf.transferCount = 0;
		cy_write_buf.buffer++;
		cy_write_buf.length = 1;
		cy_write_buf.transferCount = 0;
		if (status != CY_SUCCESS)
		{
			break;
		}
	}
	usleep(10);
	return status;
}