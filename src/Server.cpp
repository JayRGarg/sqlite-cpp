#include <cstring>
#include <iostream>
#include <fstream>
#include "var_int.h"
#include <vector>
#include <sstream>
#include "info.h"

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here" << std::endl;

	/*std::cout << "argc: " << argc << std::endl;*/
	/*for (int i = 0; i < argc; ++i) {*/
	/*	std::cout << "Argument " << i << ": " << argv[i] << std::endl;*/
	/*}*/

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    int DB_HEADER_SIZE = 100;
	int DB_HEADER_STR_SIZE = 16;
    int PG_HEADER_SIZE = 8;
    
	std::ifstream database_file(database_file_path, std::ios::binary);

    if (command == ".dbinfo") {
        if (!database_file) {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

		unsigned short page_size = get_page_size(database_file, DB_HEADER_STR_SIZE);
		unsigned short num_tables = get_num_tables(database_file, DB_HEADER_SIZE);

        // Uncomment this to pass the first stage
        /*database_file.seekg(16);  // Skip the first 16 bytes of the header*/
         
        /*char buffer[2];*/
        /*database_file.read(buffer, 2);*/
        /**/
        /*unsigned short page_size = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));*/
        /**/
        /*database_file.seekg(DB_HEADER_SIZE + 3);*/
        /*database_file.read(buffer, 2);*/

        /*unsigned short num_tables = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));*/
         
        std::cout << "database page size: " << page_size << std::endl;
        std::cout << "number of tables: " << num_tables << std::endl;
    } else if (command == ".tables") {
        if (!database_file) {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

		unsigned short num_cells = get_num_tables(database_file, DB_HEADER_SIZE);

        unsigned short cell_offset;
        unsigned short byte_buffer;

		char buffer[2];

        for (int i = 0; i < num_cells; i++) {
			cell_offset = get_cell_offset(database_file, i, DB_HEADER_SIZE, PG_HEADER_SIZE);
			std::vector<char> table_name_buffer = get_table_info(database_file, cell_offset, "tbl_name");
			for (int j=0; j < table_name_buffer.size(); j++) {
				std::cout << table_name_buffer[j];
			}
            std::cout << " ";
        }
        std::cout << std::endl;
    } else {
		//std::cout << command << std::endl;
		std::vector<std::string> tokens;
		std::stringstream ss(command);
		std::string token;
		// Extract tokens from the stringstream using whitespace as delimiters
		while (ss >> token) {
			tokens.push_back(token);
		}
		/*for (std::string t : tokens) {*/
		/*	std::cout << t << std::endl;*/
		/*}*/
		if (tokens[0] == "SELECT" || tokens[0] == "select") {
            int root_page;
            unsigned short page_offset;
            std::string table_name = tokens[3];
            unsigned short cell_offset;
            std::string i_table_name;
            unsigned short num_cells = get_num_tables(database_file, DB_HEADER_SIZE);
            unsigned short page_size = get_page_size(database_file, DB_HEADER_STR_SIZE);
            for (int i = 0; i < num_cells; i++) {
                cell_offset = get_cell_offset(database_file, i, DB_HEADER_SIZE, PG_HEADER_SIZE);
                std::vector<char> table_name_buffer = get_table_info(database_file, cell_offset, "tbl_name");
                std::string i_table_name(table_name_buffer.begin(), table_name_buffer.end());
                if (table_name == i_table_name) {
                    root_page = (int)(get_table_info(database_file, cell_offset, "root_page")[0]);
                    break;
                }
            }
            page_offset = page_size * (root_page - 1);
            /*std::cout << "page_offset: " << page_offset << std::endl;*/
            if ((tokens[1] == "COUNT(*)" || tokens[1] == "count(*)") && (tokens[2] == "FROM" || tokens[2] == "from")) {
                unsigned short num_rows = get_num_rows(database_file, page_offset);
                std::cout << num_rows << std::endl;
            } else {
                std::string search_column = tokens[1];
                std::vector<char> sql_create = get_table_info(database_file, cell_offset, "sql_create");
                std::string sql_create_statement(sql_create.begin(), sql_create.end());
                //std::cout << sql_create_statement << std::endl;

                size_t startPos = sql_create_statement.find('(');
                size_t endPos = sql_create_statement.find(')');
                std::string columns_str;

                if (startPos != std::string::npos && endPos != std::string::npos && startPos < endPos) {
                    // Found both parentheses, extract the substring between them
                    columns_str = sql_create_statement.substr(startPos + 1, endPos - startPos - 1);
                } else {
                    // Parentheses not found or in the wrong order, return an empty string
                    std::cout << "Parentheses not found in CREATE statement!";
                }
                
                std::stringstream ss(columns_str);
                std::vector<std::string> columns;
                std::string col;
                int col_number = 0;
                while (std::getline(ss, col, ',')) {
                    columns.push_back(col);
                    /*std::cout << col_number << ": " << col << std::endl;*/
                    col_number++;
                }
                std::string column_name;
                int column_index;
                for (int i=0; i<columns.size(); i++) {
                    ss.str("");
                    ss.clear();
                    ss << columns[i];
                    ss >> column_name;
                    /*std::cout << "column name: " << column_name << std::endl;*/
                    if (search_column == column_name) {
                        /*std::cout << "found column: " << search_column << ", column number: " << i << std::endl;*/
                        column_index = i;
                        break;
                    }

                }
                
                unsigned short num_rows = get_num_rows(database_file, page_offset);
                unsigned short num_cols = columns.size();
                unsigned short leaf_cell_page_offset;
                for (int r=0; r < num_rows; r++) {
                    /*std::cout << "page_offset: " << page_offset << std::endl;*/
                    leaf_cell_page_offset = get_leaf_cell_page_offset(database_file, page_offset, r);
                    //std::cout << "leaf_cell_page_offset: " << leaf_cell_page_offset << std::endl;
                    //std::cout << "leaf_cell_file_offset: " << page_offset + leaf_cell_page_offset << std::endl;
                    std::vector<std::string> row_v = get_leaf_cell_values(database_file, page_offset, leaf_cell_page_offset, num_cols);
                    /*for (std::string v : row_v) {*/
                    /*    std::cout << v << ", ";*/
                    /*}*/
                    /*std::cout << std::endl;*/
                    std::cout << row_v[column_index] << std::endl;
                }
            }
        }

	}
    

    return 0;
}
