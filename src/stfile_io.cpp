#include "stfile_io.h"


void STFileIO::remove_file_ext(string& file_name){
	auto p_index = file_name.find(".");
	if (p_index != string::npos){ 
		file_name = file_name.substr(0, p_index);	
	}
}



STFileIO::STFileIO(string file_name_no_ext, bool write_file){
	remove_file_ext(file_name_no_ext);
	string data_file_name = file_name_no_ext + ".stdata";
	string label_file_name = file_name_no_ext + ".ldata";

	if (write_file){
		fptr_data_ = fopen(data_file_name.c_str(), "wb");
		fptr_label_ = fopen(label_file_name.c_str(), "wb");
	}
	else{
		fptr_data_ = fopen(data_file_name.c_str(), "rb");
		fptr_label_ = fopen(label_file_name.c_str(), "rb");
	}
	write_size_ = false;
}

STFileIO::~STFileIO(){
	fclose(fptr_data_);
	fclose(fptr_label_);
}

void print_data(vector<uint16_t> input_vec){
	for(auto&& i : input_vec){
		printf("%08d ", i);
	}
	cout<<endl;
}

void STFileIO::init_size(size_t buff_size){
	fwrite(&buff_size, sizeof(size_t) , 1, fptr_label_);
	buff_size_ = buff_size;
	write_size_ = true;
}

void STFileIO::write_data(uint16_t* data, chrono::high_resolution_clock::time_point& curr_time){
	//make sure write the size first
	assert(write_size_);
	fwrite(&curr_time, sizeof(curr_time), 1, fptr_data_);
	fwrite(data, sizeof(uint16_t), buff_size_, fptr_data_);
}

void STFileIO::write_label(int label, chrono::high_resolution_clock::time_point& curr_time){
	cout<<label<<endl;
	fwrite(&curr_time, sizeof(curr_time), 1, fptr_label_);
	fwrite(&label, sizeof(int), 1, fptr_label_);
}

void STFileIO::read_data(STFileIO::DataStruct& dstruct){
	//fptr_ = fopen(file_name.c_str(), "rb");
	//
	//
	fread(&buff_size_, sizeof(size_t), 1, fptr_label_);	
	
	uint16_t *temp_data = new uint16_t[buff_size_];

	int temp_label = 0;
	chrono::high_resolution_clock::time_point temp_time;

	while(1){
		int read_size = fread(&temp_time, sizeof(temp_time), 1, fptr_label_);
		//int read_size = fread(&temp_label, sizeof(int), 1, fptr_label_);
		if (read_size != 1) break;
		fread(&temp_label, sizeof(temp_label), 1, fptr_label_);
		dstruct.label.push_back(temp_label);
		dstruct.label_time.push_back(temp_time);

		//fread(temp_data, sizeof(uint16_t), buff_size_, fptr_data_);
		//dlabel.push_back(temp_label);
		//data.push_back(vector<uint16_t> (temp_data, temp_data + buff_size_));
	}
	while(1){
		int read_size = fread(&temp_time, sizeof(temp_time), 1, fptr_data_);
		if (read_size != 1) break;
		fread(temp_data, sizeof(uint16_t), buff_size_, fptr_data_);
		dstruct.data_time.push_back(temp_time);
		dstruct.data.push_back(vector<uint16_t> (temp_data, temp_data + buff_size_));

		//cout<<"label: " << label[label.size()-1]<<" ";
		//print_data(dstruct.data[dstruct.data.size()-1]);
	}
}
