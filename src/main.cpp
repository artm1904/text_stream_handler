#include <cstdint>
#include <iostream>
#include <string>
#include "streams/writeStream.h"
#include "streams/readStream.h"




int main(int argc, char** argv)

{

	// if (argc < 2){
	// 	std::cerr <<"Wrong input parameters"<<std::endl;
	// 	std::cout<<"transform [опции] <input-file> <output-file>"<<std::endl;
	// 	return 1;
	// }



	std::string str{"i want to write to file this perfect string!!!"};
	FileOutputStream file {"data.txt"};
	file.WriteBlock((void*)str.c_str(), str.size());
	file.Close();



	return 0;
}