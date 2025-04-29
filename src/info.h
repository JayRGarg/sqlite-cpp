#ifndef INFO_H 
#define INFO_H

#include <fstream>
#include <vector>
#include <string>

unsigned short get_page_size(std::ifstream& db_file, int& db_header_str_size);
unsigned short get_num_tables(std::ifstream& db_file, int& db_header_size);
unsigned short get_cell_offset(std::ifstream& db_file, const int& i, int& db_header_size, int& page_header_size);
std::vector<char> get_table_info(std::ifstream& db_file, unsigned short& cell_offset, const std::string& field);
unsigned short get_num_rows(std::ifstream& db_file, unsigned short& page_offset);

#endif
