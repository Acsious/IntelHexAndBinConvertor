#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>

namespace Converter
{
    // Преобразует Intel HEX в BIN, заполняя отсутствующие адреса байтом 0xFF.
    void hexToBin(const std::string& inputName, const std::string& outputName);

    // Преобразует BIN в Intel HEX с контрольными суммами и записью EOF.
    void binToHex(const std::string& inputName, const std::string& outputName);
}

#endif
