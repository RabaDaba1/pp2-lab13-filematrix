/**
  @note Nie trzeba tutaj nic implementowac, ale jesli bysmy chcieli
  to nalezy miec ponizsze deklaracje.
  Pewne konkretyzacje jawne są zakomentowane, ze względu na powtarzający się typ. Na innych systemach może trzeba będzie zakomentować inne typy.
**/
#include <cstdint>
#include <string>
#include <memory> // std::unique_ptr
#include <fstream>
#include <algorithm> // std::equal

#include "FileMatrix.hpp"

template class FileMatrix<int8_t, std::size_t>;
template class FileMatrix<int16_t, std::size_t>;
template class FileMatrix<int32_t, std::size_t>;
template class FileMatrix<int64_t, std::size_t>;
template class FileMatrix<double, std::size_t>;
template class FileMatrix<float, std::size_t>;
template class FileMatrix<unsigned long, std::size_t>;
template class FileMatrix<char, std::size_t>;
//template class FileMatrix<short, std::size_t>;
//template class FileMatrix<int, std::size_t>;
template class FileMatrix<unsigned, std::size_t>;
template class FileMatrix<long long, std::size_t>;
template class FileMatrix<unsigned long long, std::size_t>;

