#include "Converter.h"

#include "IntelHexUtils.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <stdint.h>

namespace Converter
{
    void hexToBin(const std::string& inputName, const std::string& outputName)
    {
        IntelHexUtils::ByteMap bytes = IntelHexUtils::readHexFile(inputName);
        IntelHexUtils::writeBinaryFile(bytes, outputName);
    }

    void binToHex(const std::string& inputName, const std::string& outputName)
    {
        std::ifstream input(inputName.c_str(), std::ios::binary);
        if (!input)
            throw std::runtime_error("Не удалось открыть BIN-файл.");

        std::ofstream output(outputName.c_str());
        if (!output)
            throw std::runtime_error("Не удалось создать HEX-файл.");

        uint64_t position = 0;
        unsigned int previousUpperAddress = 0x10000;
        unsigned char data[16];

        while (position < 0x100000000ULL)
        {
            unsigned int address = static_cast<unsigned int>(position & 0xFFFF);
            unsigned int countToRead = std::min(16U, 0x10000U - address);
            input.read(reinterpret_cast<char*>(data), countToRead);
            unsigned int count = static_cast<unsigned int>(input.gcount());

            if (count == 0)
            {
                if (!input.eof())
                    throw std::runtime_error("Ошибка чтения BIN-файла.");
                break;
            }

            unsigned int upperAddress = static_cast<unsigned int>(position >> 16);
            if (upperAddress != previousUpperAddress)
            {
                unsigned char upperBytes[2] = {
                    static_cast<unsigned char>(upperAddress >> 8),
                    static_cast<unsigned char>(upperAddress)
                };
                IntelHexUtils::writeHexRecord(output, 4, 0, upperBytes, 2);
                previousUpperAddress = upperAddress;
            }

            IntelHexUtils::writeHexRecord(output, 0, address, data, count);
            position += count;
        }

        if (!input.eof() && position < 0x100000000ULL)
            throw std::runtime_error("Ошибка чтения BIN-файла.");
        if (position == 0x100000000ULL && input.peek() != EOF)
            throw std::runtime_error("BIN-файл превышает максимальный размер Intel HEX.");

        unsigned char emptyData = 0;
        IntelHexUtils::writeHexRecord(output, 1, 0, &emptyData, 0);
        output.flush();
        if (!output)
            throw std::runtime_error("Ошибка записи HEX-файла.");
    }
}
