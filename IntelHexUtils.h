#ifndef INTEL_HEX_UTILS_H
#define INTEL_HEX_UTILS_H

#include <map>
#include <stdint.h>
#include <string>
#include <iosfwd>

namespace IntelHexUtils
{
    typedef std::map<uint32_t, unsigned char> ByteMap;

    // Читает Intel HEX, проверяет записи и возвращает данные с их адресами.
    ByteMap readHexFile(const std::string& fileName);

    // Записывает данные в BIN, заполняя промежутки байтами 0xFF.
    void writeBinaryFile(const ByteMap& bytes, const std::string& fileName);

    // Записывает одну запись Intel HEX с вычисленной контрольной суммой.
    void writeHexRecord(std::ostream& output, unsigned char type, unsigned int address, const unsigned char* data, unsigned int count);

    // Возвращает путь к папке, где находится запущенная программа.
    std::string programDirectory();

    // Убирает кавычки вокруг пути к файлу.
    std::string removeQuotes(std::string path);
}

#endif
