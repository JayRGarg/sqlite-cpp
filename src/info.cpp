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
	unsigned short curr_offset, record_header_offset, table_name_offset, root_page_offset, sql_offset;
	int64_t RECORD_SIZE, ROW_ID, RECORD_HEADER_SIZE, TYPE_SIZE, NAME_SIZE, TABLE_NAME_SIZE, ROOT_PAGE_SIZE, SQL_SIZE;
	int64_t TYPE_SERIAL_TYPE, NAME_SERIAL_TYPE, TABLE_NAME_SERIAL_TYPE, ROOT_PAGE_SERIAL_TYPE, SQL_SERIAL_TYPE ;
	curr_offset = cell_offset;
	RECORD_SIZE = parse_var_int_to_int64(db_file, curr_offset);
	ROW_ID = parse_var_int_to_int64(db_file, curr_offset);
	record_header_offset = curr_offset;
	RECORD_HEADER_SIZE = parse_var_int_to_int64(db_file, curr_offset);
	TYPE_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
	NAME_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
	TABLE_NAME_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
	ROOT_PAGE_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);
    SQL_SERIAL_TYPE = parse_var_int_to_int64(db_file, curr_offset);

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
	if (SQL_SERIAL_TYPE < 13) {
		std::cerr << "SQL SERIAL-TYPE is less than 13!" << std::endl;
		return result;
	}

	TYPE_SIZE = (TYPE_SERIAL_TYPE - 13) / 2;
	NAME_SIZE = (NAME_SERIAL_TYPE - 13) / 2;
	TABLE_NAME_SIZE = (TABLE_NAME_SERIAL_TYPE - 13) / 2;
	ROOT_PAGE_SIZE = 1;
    SQL_SIZE = (SQL_SERIAL_TYPE - 13) / 2;

	table_name_offset = record_header_offset + RECORD_HEADER_SIZE + TYPE_SIZE + NAME_SIZE;
	root_page_offset = table_name_offset + TABLE_NAME_SIZE;
    sql_offset = root_page_offset + ROOT_PAGE_SIZE;
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
	} else if (field == "sql_create") {
		for (int i=0; i < SQL_SIZE; i++) {
			db_file.seekg(sql_offset + i);
			db_file.read(&buffer, 1);
			result.push_back(buffer);
		}
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

unsigned short get_leaf_cell_page_offset(std::ifstream& db_file, unsigned short& page_offset, int row_number) {
	char buffer[2];
	unsigned short leaf_cell_page_offset;
    unsigned short page_header_size = 8;
	db_file.seekg(page_offset + page_header_size + (row_number*2));
	db_file.read(buffer, 2);
	leaf_cell_page_offset = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
	return leaf_cell_page_offset;
}

std::vector<std::string> get_leaf_cell_values(std::ifstream& db_file, unsigned short& page_offset, unsigned short leaf_cell_page_offset, unsigned short num_cols) {
    unsigned short curr_offset = page_offset + leaf_cell_page_offset;
	int64_t PAYLOAD_SIZE = parse_var_int_to_int64(db_file, curr_offset);
    int64_t row_id = parse_var_int_to_int64(db_file, curr_offset);
    /*std::cout << "PAYLOAD SIZE: " << PAYLOAD_SIZE << ", row_id: " << row_id << std::endl;*/
	int64_t RECORD_HEADER_SIZE;
	int64_t TYPE_SERIAL_TYPE;
    unsigned short record_header_offset = curr_offset;
	RECORD_HEADER_SIZE = parse_var_int_to_int64(db_file, curr_offset);
    /*std::cout << "RECORD_HEADER_SIZE: " << RECORD_HEADER_SIZE << std::endl;*/
    std::vector<int64_t> types;
    /*while (curr_offset-record_header_offset<RECORD_HEADER_SIZE) {*/
    /*    types.push_back(parse_var_int_to_int64(db_file, curr_offset));*/
    /*}*/
    for (int i = 0; i < num_cols; i++) {
        types.push_back(parse_var_int_to_int64(db_file, curr_offset));
    }
    std::vector<int64_t> sizes;
    for (int i=0; i < types.size(); i++) {
        if (types[i] < 13) {
            if (types[i] != 0) {
                std::cout << "Right Type not expected! Expected 0 or 13 but got: " << types[i] << std::endl;
            }
            /*std::cout << "Type: " << types[i] << ", Size: " << (types[i]-13)/2 << std::endl;*/
            sizes.push_back(0);

        } else {
            /*std::cout << "Type: " << types[i] << ", Size: " << (types[i]-13)/2 << std::endl;*/
            sizes.push_back((types[i]-13)/2);
        }
    }


	char buffer;
    std::vector<char> value;
    std::vector<std::string> result;
    std::string val_str;
    int val_offset;

    for (int i=0; i < sizes.size(); i++) {
        for (int j=0; j < sizes[i]; j++) {
            if (i == 0) {
                val_offset = record_header_offset + RECORD_HEADER_SIZE + j;
            } else {
                val_offset = record_header_offset + RECORD_HEADER_SIZE + sizes[i-1] + j;
            }
            db_file.seekg(val_offset);
            db_file.read(&buffer, 1);
            value.push_back(buffer);
        }
        val_str = std::string(value.data(), value.size());
        value.clear();
        result.push_back(val_str);
    }
    
    return result;

}

