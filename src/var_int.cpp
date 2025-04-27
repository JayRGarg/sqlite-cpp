#include "var_int.h"
#include <iostream>
#include <fstream>

int64_t parse_var_int_to_int64(std::ifstream& db_file, unsigned short& offset) {

	db_file.seekg(offset);
	unsigned int byte;
	char* buffer = reinterpret_cast<char*>(&byte);
	db_file.read(buffer, 1);
	int64_t total = 0;
	int count = 0;
	
	while ((byte & 0b10000000) && count < 9) {
		total <<= 7;
		total |= static_cast<int64_t>(byte & 0b01111111);
		count++;
		offset++;
		db_file.seekg(offset);
		db_file.read(buffer, 1);
	}

	if (count == 9) {
		total <<= 8;
		total |= static_cast<int64_t>(byte);
	} else {
		total <<= 7;
		total |= static_cast<int64_t>(byte & 0b01111111);
	}
	count++;
	offset++;
	db_file.seekg(offset);

	return total;
}
