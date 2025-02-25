#include <cstring>
#include <iostream>
#include <fstream>

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
    } else if (command == ".table") {
        std::ifstream database_file(database_file_path, std::ios::binary);
        if (!database_file) {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        char buffer[2];
        database_file.seekg(DB_HEADER_SIZE + 3);
        database_file.read(buffer, 2);

        unsigned short num_cells = (static_cast<unsigned char> (buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

        //std::cout << num_cells << std::endl;

        unsigned short offset;
        unsigned short byte_buffer, RECORD_HEADER_SIZE, TYPE_SIZE, NAME_SIZE;

        for (int i = 0; i < num_cells; i++) {
            database_file.seekg(DB_HEADER_SIZE + PG_HEADER_SIZE + (i*2));
            database_file.read(buffer, 2);
            offset = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));
            //std::cout << offset << std::endl;
            database_file.seekg(offset);
            database_file.read(buffer, 2);
            //std::cout << (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8)) << std::endl;
            database_file.seekg(offset + 2);
            database_file.read(buffer, 2);
            RECORD_HEADER_SIZE = (unsigned short)buffer[0];
            TYPE_SIZE = ((unsigned short)buffer[1] - 13) / 2;
            //std::cout << "RECORD_HEADER_SIZE: " << RECORD_HEADER_SIZE << std::endl;
            //std::cout << "TYPE_SIZE: " << TYPE_SIZE << std::endl;
            database_file.seekg(offset + 4);
            database_file.read(buffer, 2);
            NAME_SIZE = ((unsigned short)buffer[0] - 13) / 2;
            //std::cout << "NAME_SIZE: " << NAME_SIZE << std::endl;

            char name_buffer[NAME_SIZE];
            database_file.seekg(offset + 2 + RECORD_HEADER_SIZE + TYPE_SIZE);
            database_file.read(name_buffer, NAME_SIZE);
            for (int j = 0; j < NAME_SIZE; j++) {
                std::cout << name_buffer[j];
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
