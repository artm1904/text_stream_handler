#include <cstdint>
#include <iostream>
#include <string>





int main(int argc, char** argv)

{

	if (argc < 2){
		std::cerr <<"Wrong input parameters"<<std::endl;
		std::cout<<"transform [опции] <input-file> <output-file>"<<std::endl;
		return 1;
	}



	return 0;
}