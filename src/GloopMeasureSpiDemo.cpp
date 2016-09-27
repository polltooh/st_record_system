// GloopMeasureSpiDemo.cpp : Defines the entry point for the console application.
//
#include "gloop_measure_spi_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>




int main(int argc, char** argv)
{
	spi_context_t* context;
	//gloop_data_spidat_t data;
	gloop_data_spidat2_t data;
	//int32_t size = gloop_create_default_context(NULL);
	//
	uint8_t deviceNum = 12;
	uint8_t interfaceNum = 0;
	//unsigned char deviceNum = 0;
	//unsigned char interfaceNum = 0;

	int32_t size = gloop_create_default_context_with_id(NULL, deviceNum, interfaceNum);
	clock_t start_time, end_time;
	
	double time_span;
	int j = 0;
	if (size < 0)
	{
		printf("error sizing context.\n");
		return -1;
	}
	context = (spi_context_t*)malloc(size);
	size = gloop_create_default_context_with_id(context, deviceNum, interfaceNum);
	if (size <0 ) printf("wrong\n");

	//size = gloop_create_default_context(context);
	if (size < 0)
	{
		printf("error creating spi context.\n");
		return -1;
	}
	//gloop_send_config_cmd(context, GLOOP_ODR_100HZ, GLOOP_TRGR_CONT);
	gloop_send_config_cmd2(context, GLOOP_ODR_500HZ, GLOOP_TRGR_CONT,
		16, GLOOP_RES_001PF);
	start_time = clock();
	for (;;)//(size = 0; size < 1000; ++size)
	{
		int i = 0;
		usleep(5000);//Sleep(16);
		//gloop_read_data(context, &data);
		gloop_read_data2(context, &data);
		end_time = clock();
		time_span = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

		if (j < 16)
		{
			printf("\n%f,", time_span); //computer time
			printf("0x%x,", data.buf[i++]); // data type
			printf("0x%x,", data.buf[i++]); // sequence number
			for (; i < sizeof(data); i += 2)
			{
				printf("%08d,", (((uint16_t) data.buf[i]) << 8) | ((uint16_t) data.buf[i + 1])); // datum
			}
			++j;
		}
		if (0)//(GetAsyncKeyState(VK_ESCAPE))
		{
			break;
		}
		if (true)//(GetAsyncKeyState(VK_RETURN))
		{
			j = 0;
			//system("cls");
		}
	}

	gloop_free_resources(context);
	free(context);

	return 0;
}

