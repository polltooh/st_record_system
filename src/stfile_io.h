#ifndef STFILE_IO_H_
#define STFILE_IO_H_

#include <string>
#include <stdio.h>
#include <cassert>
#include <vector>
#include <iostream>

#include <chrono>

using namespace std;



class STFileIO{
	FILE* fptr_data_;
	FILE* fptr_label_;
	bool write_size_;
	size_t buff_size_;
	void remove_file_ext(string& file_name);
public:
	struct DataStruct{
		vector<chrono::high_resolution_clock::time_point> label_time;
		vector<int> label;
		vector<chrono::high_resolution_clock::time_point> data_time;
		vector<vector<uint16_t>> data;
	};
	//init the file pointer, if write_file is true
	//it will open the file for writing
	//otherwise, it will open the file for reading
	STFileIO(string file_name, bool write_file);
	~STFileIO();
	void init_size(size_t buff_size);
	// The data format will be buffer size, label, data
	void write_data(uint16_t* data, chrono::high_resolution_clock::time_point& curr_time);
	void write_label(int label, chrono::high_resolution_clock::time_point& curr_time);
	
	//void write_data(uint16_t* data, int label);
	void read_data(DataStruct& dstruct);

};

#endif
