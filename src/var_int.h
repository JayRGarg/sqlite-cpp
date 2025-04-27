#ifndef VAR_INT_H
#define VAR_INT_H

#include <iostream>
#include <fstream>

int64_t parse_var_int_to_int64(std::ifstream& db_file, unsigned short& offset);

#endif // VAR_INT_H
