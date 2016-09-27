#include "stfile_io.h"
#include <string>
#include <vector>

using namespace std;

int main(int argc, char* argv[]){
	if (argc < 2){
		cout<<"Usage read_data file_name.stdata"<<endl; 
		exit(1);
	}

	STFileIO file_io(string(argv[1]), false);

	STFileIO::DataStruct dstruct;

	file_io.read_data(dstruct);	

	for (auto&& i :	dstruct.data){
		for (auto && j : i){
			cout<<j<< " ";
		}
		cout<<endl;
	}
	for (auto&& i : dstruct.label){
		cout<<i<<" ";
	}
	cout<<endl;

	return 0;
}
