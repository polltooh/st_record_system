#include "stfile_io.h"
#include <fstream>
#include <climits>
#include <string>
#include <vector>
#include "boost/filesystem.hpp"
#include <boost/algorithm/string/predicate.hpp>
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

void remove_file_ext(string& file_name){
	auto p_index = file_name.find(".");
	if (p_index != string::npos){ 
		file_name = file_name.substr(0, p_index);	
	}
}

vector<string> get_list_files(string file_path){

	boost::filesystem::path p(file_path);
	boost::filesystem::directory_iterator end_itr;
	vector<string> file_list; 

    for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
    {
		string current_file = itr->path().string();
        if (boost::filesystem::is_regular_file(itr->path()) && boost::algorithm::ends_with(current_file, "ldata")){
			remove_file_ext(current_file);
			file_list.push_back(current_file);
            cout << current_file << endl;
        }
    }

	return file_list;
}

vector<vector<float>> normalize_data(const vector<vector<uint16_t>>& data){

	vector<vector<float>> fdata(data.size(), vector<float>(data[0].size(), 0.0));

	for (int i = 0; i < data[0].size(); ++i){
		float min_v = UINT_MAX;
		float max_v = 0;
		//auto minmax = std::minmax_element(data[i].begin(), data[i].end());
		for (int j = 0; j < data.size(); ++j){
			min_v = float(min(min_v, (float)data[j][i]));
			max_v = float(max(max_v, (float)data[j][i]));
		}
		for (int j = 0; j < data.size(); ++j){
			fdata[j][i] = ((float)data[j][i] - min_v) / (max_v - min_v);
		}
	}


	return fdata;
}

void save_data(string path, vector<int>& label, vector<vector<float>>& data){
	auto data_name = path + "combined.data";
	auto label_name = path + "combined.label";
	
    //std::ofstream output_filel(label_name, ofstream::binary);
	auto* output_filel = fopen(label_name.c_str(), "w");
    for (auto &&l : label) fwrite(&l, sizeof(int), 1, output_filel);
	//for (auto &&l : label) output_filel << l;
	fclose(output_filel);
    //std::ofstream output_filed(data_name, ofstream::binary);
	auto* output_filed = fopen(data_name.c_str(), "w");

    for (auto &&da : data)
		for(auto&& d : da){
			fwrite(&d, sizeof(float), 1, output_filed);
		}

	fclose(output_filed);
}
// file_name
// data to store the sensor response
// label corresponding to the data
// win_size: > 0

void get_data_label(string file_name, vector<vector<float>>& data, vector<int>& label, int win_size){
	assert(win_size > 0);

	STFileIO file_io(file_name, false);
	STFileIO::DataStruct dstruct;
	file_io.read_data(dstruct);
		
	auto fdata = normalize_data(dstruct.data);

	int data_start_index = 0;

	auto end_index = label.size();

	for (int i = 0; i < dstruct.label_time.size(); ++i){
		label.push_back(dstruct.label[i]);
		int data_index = get_data_index(dstruct.label_time[i], dstruct.data_time, data_start_index);
		data_start_index = data_index + 1;
		if (data_index < win_size) continue;

		vector<float> data_vec;
		for (int j = win_size - 1; j >= 0; --j){
			//first element is not used
			data_vec.insert(data_vec.end(), fdata[data_index - j].begin() +1, fdata[data_index - j].end());
		}
				
		//vector<float> data_vec(fdata[data_index].begin() + 1, fdata[data_index].end());
		data.push_back(data_vec);
		cout<<label[end_index]<<" ";
		for (auto&& d : data[end_index]) cout<<d<<" ";
		cout<<endl;
		end_index++;
	}
}


int main(int argc, char* argv[]){
	if (argc < 2){
		cout<<"Usage convert_data path"<<endl;
		exit(1);
	}
	string input_file_name = string(argv[1]);
	auto file_list = get_list_files(input_file_name);
	vector<vector<float>> data;
	vector<int> label;
	int win_size = 5;
	for (auto&& file_name : file_list){
		get_data_label(file_name, data, label, win_size);
	}
	cout<<"total data is "<<label.size()<<endl;
	save_data(input_file_name, label, data);
}
