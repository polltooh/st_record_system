#include "stfile_io.h"
#include <string>
#include <vector>
#include <chrono>

using namespace std;

typedef chrono::high_resolution_clock::time_point tpoint;

int get_data_index(tpoint& label_time, vector<tpoint>& data_time, int start_index){
	for (int i = start_index; i < data_time.size(); ++i){
		if (data_time[i] == label_time) return i;
	}
	return -1;
}


int main(int argc, char* argv[]){

	if (argc < 2){
			cout<<"Usage convert_data file_name"<<endl;
	}
	
	STFileIO file_io(string(argv[1]), false);

	STFileIO::DataStruct dstruct;
	file_io.read_data(dstruct);
	
	vector<vector<uint16_t>> data;
	vector<int> label;
	int data_start_index = 0;
	for (int i = 0; i < dstruct.label_time.size(); ++i){
		label.push_back(dstruct.label[i]);
		int data_index = get_data_index(dstruct.label_time[i], dstruct.data_time, data_start_index);
		data.push_back(dstruct.data[data_index]);
		data_start_index = data_index + 1;
		cout<<label[i]<<" ";
		for (auto&& d : data[i]) cout<<d<<" ";
		cout<<endl;
	}
}
