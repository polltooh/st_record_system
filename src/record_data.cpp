// GloopMeasureSpiDemo.cpp : Defines the entry point for the console application.
//
#include "stfile_io.h"
#include "gloop_measure_spi_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <unordered_map>
#include <chrono>

using namespace std;

unordered_map<int,string> label_map{
	{0, "neutral"},
	{1, "smile"},
	{2, "angry"},
	{3, "blink_left_eye"},
	{4, "blink_right_eye"},
	{5, "close_eyes_gently"},
	{6, "close_eyes_tight"},
	{7, "eyebrow_raised"},
	{8, "scrunch_your_nose"}
};

const int quit_key = 113;


void play_audio(string audio_name){
	string command = "aplay " + audio_name;
	system(command.c_str());
}

void key_press(bool& write_to_dict, int& label, bool &quit){
	while(1){
		// make sure label changed before 
		// loading the next image
		while(write_to_dict){
			usleep(10);
		}
		auto image_name = label_map[label] + ".jpg";
		auto audio_name = label_map[label] + ".wav";
		thread audio_thread(play_audio, audio_name);  

		cv::Mat img = cv::imread(image_name);
		cv::imshow("image", img);
		auto key = cv::waitKey();
		audio_thread.join();	
		if (key == quit_key){
			quit = true;
			return;
		}
		std::cout<<"key is pressed: "<< key<<std::endl;
		write_to_dict = true;
	}
}


//void write_data(FILE* fptr, uint16_t* data, int data_size, int label){
//	cout<<"write data"<<endl;
//	fwrite(&label, sizeof(int), 1, fptr);
//	fwrite(data, sizeof(uint16_t), data_size, fptr);
//}

void add_check_ext(string& input_str){

}

int main(int argc, char** argv){

	string default_input_str = "test_data";
	auto file_name_no_ext = default_input_str;


	if (argc == 2){
			file_name_no_ext = string(argv[1]); 
	}

	bool write_to_dict = false;
	int label = 0;
	bool quit = false;

	STFileIO file_io(file_name_no_ext, true);

	thread first (key_press, ref(write_to_dict), ref(label), ref(quit));  

	spi_context_t* context;
	//gloop_data_spidat_t data;
	gloop_data_spidat2_t data;
	//int32_t size = gloop_create_default_context(NULL);
	//
	uint8_t deviceNum = 22;
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

			//int temp_buffer[sizeof(data)] = {0};
			size_t buff_size = sizeof(data)/2;

			cout<<"buff_size is: "<< buff_size << endl;
			uint16_t* temp_buff = new uint16_t[buff_size]{0};

			//auto* fptr = fopen(data_file_name.c_str(), "wb");
	//fwrite(&sdata, sizeof(size_t) , 1, fptr);
	file_io.init_size(buff_size);

	for (;;)//(size = 0; size < 1000; ++size)
	{
		int i = 0;
		//usleep(5000);//Sleep(16);
		usleep(5);//Sleep(16);
		//gloop_read_data(context, &data);
		gloop_read_data2(context, &data);
		end_time = clock();
		time_span = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

		if (j < 16)
		{
			//printf("\n%f,", time_span); //computer time
			//printf("0x%x,", data.buf[i++]); // data type
			//printf("0x%x,", data.buf[i++]); // sequence number
			for (; i < sizeof(data); i += 2)
			{
				//printf("%08d,", (((uint16_t) data.buf[i]) << 8) | ((uint16_t) data.buf[i + 1])); // datum
				temp_buff[i/2] = (((uint16_t) data.buf[i]) << 8) | ((uint16_t) data.buf[i + 1]); // datum
				//printf("%08d,", (((uint16_t) data.buf[i]) << 8) | ((uint16_t) data.buf[i + 1])); // datum
			}
			++j;
		}
		if (quit){
			break;
		}
		auto curr_time = chrono::high_resolution_clock::now();
		if (write_to_dict){
			//file_io.write_data(temp_buff, label);
			file_io.write_label(label, curr_time);
			write_to_dict = false;
			label = (label + 1) % label_map.size();
		}

		file_io.write_data(temp_buff, curr_time);

		if (true)//(GetAsyncKeyState(VK_RETURN))
		{
			j = 0;
			//system("cls");
		}
	}

	first.join();	
	//fclose(fptr);

	gloop_free_resources(context);
	free(context);
	return 0;
}

