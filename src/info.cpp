#include <fstream>
#include <vector>
#include <iostream>
#include "var_int.h"

unsigned short get_page_size(std::ifstream& db_file, int& db_header_str_size) {
	db_file.seekg(db_header_str_size);  // Skip the first 16 bytes of the header
	 
	char buffer[2];
	db_file.read(buffer, 2);
	 
	unsigned short page_size = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

	return page_size;
}

unsigned short get_num_tables(std::ifstream& db_file, int& db_header_size) {
	db_file.seekg(db_header_size + 3);

	char buffer[2];
	db_file.read(buffer, 2);

	unsigned short num_tables = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

	return num_tables;
}

unsigned short get_cell_offset(std::ifstream& db_file, const int& i, int& db_header_size, int& page_header_size) {
	char buffer[2];
	unsigned short cell_offset;
	db_file.seekg(db_header_size + page_header_size + (i*2));
	db_file.read(buffer, 2);
	cell_offset = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
	return cell_offset;
}

std::vector<char> get_table_info(std::ifstream& db_file, unsigned short& cell_offset, const std::string& field) {
	std::vector<char> result;
	unsigned short curr_offset, record_header_offset, table_name_offset, root_page_offset;
	int64_t RECORD_SIZE, ROW_ID, RECORD_HEADER_SIZE, TYPE_SIZE, NAME_SIZE, TABLE_NAME_SIZE, ROOT_PAGE_SIZE;
	int64_t TYPE_SERIAL_TYPE, NAME_SERIAL_TYPE, TABLE_NAME_SERIAL_TYPE, ROOT_PAGE_SERIAL_TYPE;
	curr_offset = cell_offset;
	RECORD_SIZE = parse_var_int_to_int64(db_file, curr_offset);
	ROW_ID = parse_var_int_to_int64(db_file, curr_offset);
	record_header_offset = curr_offset;
	RECORD_HEADER_SIZE = parse_var_int_to_int64(db_file, curr_offset);
	TYPE_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
	NAME_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
	TABLE_NAME_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
	ROOT_PAGE_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);

	if (TYPE_SERIAL_TYPE < 13) {
		std::cerr << "TYPE SERIAL-TYPE is less than 13!" << std::endl;
		return result;
	}
	if (NAME_SERIAL_TYPE < 13) {
		std::cerr << "NAME SERIAL-TYPE is less than 13!" << std::endl;
		return result;
	}
	if (TABLE_NAME_SERIAL_TYPE < 13) {
		std::cerr << "TABLE_NAME SERIAL-TYPE is less than 13!" << std::endl;
		return result;
	}
	if (ROOT_PAGE_SERIAL_TYPE != 1) {
		std::cerr << "ROOT_PAGE SERIAL-TYPE is not 1!" << std::endl;
		return result;
	}

	TYPE_SIZE = (TYPE_SERIAL_TYPE - 13) / 2;
	NAME_SIZE = (NAME_SERIAL_TYPE - 13) / 2;
	TABLE_NAME_SIZE = (TABLE_NAME_SERIAL_TYPE - 13) / 2;
	ROOT_PAGE_SIZE = 1;

	table_name_offset = record_header_offset + RECORD_HEADER_SIZE + TYPE_SIZE + NAME_SIZE;
	root_page_offset = table_name_offset + TABLE_NAME_SIZE;
	char buffer;

	if (field == "tbl_name") {
		for (int i=0; i < TABLE_NAME_SIZE; i++) {
			db_file.seekg(table_name_offset + i);
			db_file.read(&buffer, 1);
			result.push_back(buffer);
		}
	} else if (field == "root_page") {
		db_file.seekg(root_page_offset);
		db_file.read(&buffer, 1);
		result.push_back(buffer);
	}
	return result;
}

unsigned short get_num_rows(std::ifstream& db_file, unsigned short& page_offset) {
	db_file.seekg(page_offset + 3);

	char buffer[2];
	db_file.read(buffer, 2);

	unsigned short num_rows = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

	return num_rows;
}
