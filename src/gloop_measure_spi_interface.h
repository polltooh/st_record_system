/* ========================================
*
* Copyright Oculus VR, 2016
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* CONFIDENTIAL AND PROPRIETARY INFORMATION
* WHICH IS THE PROPERTY OF OCULUS VR.
*
* ========================================
*/

#ifndef GLOOP_MEASURE_SPI_INTERFACE_H
#define GLOOP_MEASURE_SPI_INTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

///
/// Macro Definitions
///
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

typedef struct
{
	uint16_t vid;
	uint16_t pid;
	uint32_t spi_frequency_hz;
	bool spi_clock_polarity;
	bool spi_clock_phase;
	uint8_t select_address;
} spi_context_t;


#define GLOOP_DEFAULT_VID			(0x04B4)
#define GLOOP_DEFAULT_PID			(0x0004)
#define GLOOP_DEFAULT_PHASE			(1U)
#define GLOOP_DEFAULT_POLARITY		(0U)
#define GLOOP_DEFAULT_FREQ_HZ		(700000U)
#define GLOOP_DEFAULT_SELECT_ADDR   (0U)

#define GLOOP_MSGTYPE_DATA	(0x00U)
#define GLOOP_MSGTYPE_CONF  (0x01U)

#define GLOOP_ODR_OFF	 (0x00U)
#define GLOOP_ODR_25HZ	 (0x01U)
#define GLOOP_ODR_50HZ	 (0x02U)
#define GLOOP_ODR_100HZ	 (0x03U)
#define GLOOP_ODR_150HZ	 (0x04U)
#define GLOOP_ODR_200HZ	 (0x05U)
#define GLOOP_ODR_250HZ	 (0x06U)
#define GLOOP_ODR_500HZ	 (0x07U)

/*! \brief Continuous Sampling*/
#define GLOOP_TRGR_CONT	 (0x00U)
/*! \brief 'Not Applicable' */
#define GLOOP_TRGR_NA	 (0x01U)
/*! \brief External Trigger */
#define GLOOP_TRGR_EXT  (0x02U)

/*! \brief Default filter length[1pt] */
#define GLOOP_FILTER_DEFAULT	(0x01U)

/*! \brief 1 picofarad resolution */
#define GLOOP_RES_1PF	(0x00U)
/*! \brief 0.1 picofarad resolution */
#define GLOOP_RES_01PF	(0x01U)
/*! \brief 0.01 picofarad resolution */
#define GLOOP_RES_001PF	(0x02U)
/*! \brief 0.001 picofarad resolution */
#define GLOOP_RES_0001PF (0x03U)

typedef union
{
	struct
	{
		uint8_t message_type;
		uint8_t odr;
		uint8_t trgr_mode;
		uint8_t zeroes[9];
	}; 
	uint8_t buf[12];
} gloop_configuration_spicmd_t;

typedef union
{
	struct
	{
		uint8_t message_type;
		uint8_t odr;
		uint8_t interrupt_mode;
		uint8_t trgr_mode;
		uint8_t filter;
		uint8_t resolution;
		uint8_t zeroes[16];
	};
	uint8_t buf[22];
} gloop_configuration_spicmd2_t;

typedef union
{
	struct
	{
		uint8_t message_type;
		uint8_t sequence_no;
		uint16_t data1;
		uint16_t data2;
		uint16_t data3;
		uint16_t data4;
		uint16_t data5;

	};
	uint8_t buf[12];
} gloop_data_spidat_t;

typedef union
{
	struct
	{
		uint8_t message_type;
		uint8_t sequence_no;
		uint16_t data1;
		uint16_t data2;
		uint16_t data3;
		uint16_t data4;
		uint16_t data5;
		uint16_t data6;
		uint16_t data7;
		uint16_t data8;
		uint16_t data9;
		uint16_t data10;

	};
	uint8_t buf[22];
} gloop_data_spidat2_t;

/* \brief Provide with NULL memory pointer to get allocation size needed */
EXTERN int32_t gloop_create_default_context(spi_context_t* context);

EXTERN int32_t gloop_create_default_context_with_id(spi_context_t* context,
						    uint8_t deviceNumber,
						    uint8_t interfaceNum);

EXTERN int32_t gloop_create_context_with_id(uint16_t vid, uint16_t pid,
					    uint32_t spi_frequency_hz,
					    bool spi_clock_polarity,
					    bool spi_clock_phase,
					    uint8_t spi_select_address,
					    void* memory,
					    uint8_t deviceNumber,
					    uint8_t interfaceNum);
  
/* \brief Provide with NULL memory pointer to get allocation size needed */
EXTERN int32_t gloop_create_context(uint16_t vid, uint16_t pid, uint32_t spi_frequency_hz,
									bool spi_clock_polarity, bool spi_clock_phase,
									uint8_t spi_select_address,
									void* memory);

EXTERN bool gloop_send_config_cmd(spi_context_t* context, uint8_t odr, uint8_t trgr_mode);
EXTERN bool gloop_send_config_cmd2(spi_context_t* context, uint8_t odr, uint8_t trgr_mode, 
	                               uint8_t filter, uint8_t resolution);

EXTERN bool gloop_read_data(spi_context_t* context, gloop_data_spidat_t* data_out);
EXTERN bool gloop_read_data2(spi_context_t* context, gloop_data_spidat2_t* data_out);

/* Debug version of read_data*/
EXTERN bool gloop_read_data_slow(spi_context_t* context, gloop_data_spidat_t* data_out);

EXTERN bool gloop_trigger_conversion(spi_context_t* context);

EXTERN bool gloop_free_resources(spi_context_t* context);

#endif //GLOOP_MEASURE_SPI_INTERFACE_H
