#include <cstring>
#include <iostream>
#include <fstream>
#include "var_int.h"

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    int DB_HEADER_SIZE = 100;
    int PG_HEADER_SIZE = 8;

    if (command == ".dbinfo") {
        std::ifstream database_file(database_file_path, std::ios::binary);
        if (!database_file) {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        // Uncomment this to pass the first stage
        database_file.seekg(16);  // Skip the first 16 bytes of the header
         
        char buffer[2];
        database_file.read(buffer, 2);
         
        unsigned short page_size = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

        database_file.seekg(DB_HEADER_SIZE + 3);
        database_file.read(buffer, 2);

        unsigned short num_tables = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
         
        std::cout << "database page size: " << page_size << std::endl;
        std::cout << "number of tables: " << num_tables << std::endl;
    } else if (command == ".tables") {
        std::ifstream database_file(database_file_path, std::ios::binary);
        if (!database_file) {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        char buffer[2];
        database_file.seekg(DB_HEADER_SIZE + 3);
        database_file.read(buffer, 2);

        unsigned short num_cells = (static_cast<unsigned char> (buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

        unsigned short cell_offset, curr_offset, record_header_offset, table_name_offset;
        unsigned short byte_buffer;
		int64_t RECORD_SIZE, ROW_ID, RECORD_HEADER_SIZE, TYPE_SIZE, NAME_SIZE, TABLE_NAME_SIZE;
		int64_t TYPE_SERIAL_TYPE, NAME_SERIAL_TYPE, TABLE_NAME_SERIAL_TYPE;

        for (int i = 0; i < num_cells; i++) {
            database_file.seekg(DB_HEADER_SIZE + PG_HEADER_SIZE + (i*2));
            database_file.read(buffer, 2);
            cell_offset = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
			curr_offset = cell_offset;
			RECORD_SIZE = parse_var_int_to_int64(database_file, curr_offset);
			ROW_ID = parse_var_int_to_int64(database_file, curr_offset);
			record_header_offset = curr_offset;
			RECORD_HEADER_SIZE = parse_var_int_to_int64(database_file, curr_offset);
			TYPE_SERIAL_TYPE = parse_var_int_to_int64(database_file, curr_offset);
			NAME_SERIAL_TYPE = parse_var_int_to_int64(database_file, curr_offset);
			TABLE_NAME_SERIAL_TYPE = parse_var_int_to_int64(database_file, curr_offset);

			if (TYPE_SERIAL_TYPE < 13) {
				std::cerr << "TYPE SERIAL-TYPE is less than 13!" << std::endl;
				return 1;
			}
			if (NAME_SERIAL_TYPE < 13) {
				std::cerr << "NAME SERIAL-TYPE is less than 13!" << std::endl;
				return 1;
			}
			if (TABLE_NAME_SERIAL_TYPE < 13) {
				std::cerr << "TABLE_NAME SERIAL-TYPE is less than 13!" << std::endl;
				return 1;
			}

			TYPE_SIZE = (TYPE_SERIAL_TYPE - 13) / 2;
			NAME_SIZE = (NAME_SERIAL_TYPE - 13) / 2;
			TABLE_NAME_SIZE = (TABLE_NAME_SERIAL_TYPE - 13) / 2;

			/*std::cout << std::hex << std::setw(2) << std::setfill('0') << RECORD_SIZE << std::endl;*/
			/*std::cout << std::hex << std::setw(2) << std::setfill('0') << ROW_ID << std::endl;*/
			/*std::cout << std::hex << std::setw(2) << std::setfill('0') << RECORD_HEADER_SIZE << std::endl;*/
			/*std::cout << std::hex << std::setw(2) << std::setfill('0') << TYPE_SIZE << std::endl;*/
			/*std::cout << std::hex << std::setw(2) << std::setfill('0') << NAME_SIZE << std::endl;*/
			/*std::cout << std::hex << std::setw(2) << std::setfill('0') << TABLE_NAME_SIZE << std::endl;*/
			table_name_offset = record_header_offset + RECORD_HEADER_SIZE + TYPE_SIZE + NAME_SIZE;
			database_file.seekg(table_name_offset);
			char table_name_buffer[TABLE_NAME_SIZE];
			database_file.read(table_name_buffer, TABLE_NAME_SIZE);
			for (int j=0; j < TABLE_NAME_SIZE; j++) {
				std::cout << table_name_buffer[j];
			}
            std::cout << " ";
        }
        std::cout << std::endl;





    }
    {
        /* code */
    }
    

    return 0;
}
