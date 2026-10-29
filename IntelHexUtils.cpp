#include "IntelHexUtils.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
    int hexDigit(char symbol)
    {
        if (symbol >= '0' && symbol <= '9')
            return symbol - '0';
        if (symbol >= 'A' && symbol <= 'F')
            return symbol - 'A' + 10;
        if (symbol >= 'a' && symbol <= 'f')
            return symbol - 'a' + 10;
        return -1;
    }

    unsigned char hexByte(const std::string& line, size_t position)
    {
        int high = hexDigit(line[position]);
        int low = hexDigit(line[position + 1]);
        if (high < 0 || low < 0)
            throw std::runtime_error("В HEX-файле найден неверный символ.");
        return static_cast<unsigned char>(high * 16 + low);
    }
}

namespace IntelHexUtils
{
    ByteMap readHexFile(const std::string& fileName)
    {
        std::ifstream input(fileName.c_str());
        if (!input)
            throw std::runtime_error("Не удалось открыть HEX-файл.");

        ByteMap bytes;
        uint32_t addressBase = 0;
        bool endOfFileFound = false;
        std::string line;

        while (std::getline(input, line))
        {
            while (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
                line.erase(0, 1);
            while (!line.empty() &&
                   (line[line.size() - 1] == ' ' ||
                    line[line.size() - 1] == '\t' ||
                    line[line.size() - 1] == '\r'))
                line.erase(line.size() - 1);
            if (line.empty())
                continue;
            if (endOfFileFound)
                throw std::runtime_error("В HEX-файле есть данные после записи EOF.");
            if (line[0] != ':' || line.size() < 11)
                throw std::runtime_error("Неверный формат записи Intel HEX.");

            unsigned char count = hexByte(line, 1);
            if (line.size() != 11 + static_cast<size_t>(count) * 2)
                throw std::runtime_error("Длина записи Intel HEX указана неверно.");

            std::vector<unsigned char> record;
            unsigned int checksum = 0;
            for (size_t i = 1; i < line.size(); i += 2)
            {
                unsigned char value = hexByte(line, i);
                record.push_back(value);
                checksum += value;
            }
            if (checksum % 256 != 0)
                throw std::runtime_error("Контрольная сумма HEX-записи неверна.");

            unsigned int address = record[1] * 256 + record[2];
            unsigned char type = record[3];

            if (type == 0)
            {
                uint64_t firstAddress =
                    static_cast<uint64_t>(addressBase) + address;
                if (firstAddress + count > 0x100000000ULL)
                    throw std::runtime_error("Адрес данных выходит за пределы Intel HEX.");
                for (unsigned int i = 0; i < count; ++i)
                    bytes[static_cast<uint32_t>(firstAddress + i)] = record[4 + i];
            }
            else if (type == 1)
            {
                if (count != 0 || address != 0)
                    throw std::runtime_error("Неверная запись EOF.");
                endOfFileFound = true;
            }
            else if (type == 2 || type == 4)
            {
                if (count != 2 || address != 0)
                    throw std::runtime_error("Неверная запись расширенного адреса.");
                unsigned int value = record[4] * 256 + record[5];
                addressBase = type == 2 ? value * 16 : value * 65536;
            }
            else if (type == 3 || type == 5)
            {
                if (count != 4 || address != 0)
                    throw std::runtime_error("Неверная запись начального адреса.");
            }
            else
            {
                throw std::runtime_error("Неизвестный тип записи Intel HEX.");
            }
        }

        if (!endOfFileFound)
            throw std::runtime_error("В HEX-файле отсутствует запись EOF.");
        return bytes;
    }

    void writeBinaryFile(const ByteMap& bytes, const std::string& fileName)
    {
        std::ofstream output(fileName.c_str(), std::ios::binary);
        if (!output)
            throw std::runtime_error("Не удалось создать BIN-файл.");
        if (bytes.empty())
            return;

        uint64_t address = bytes.begin()->first;
        char fillBuffer[4096];
        std::fill(fillBuffer, fillBuffer + 4096, static_cast<char>(0xFF));

        for (ByteMap::const_iterator item = bytes.begin();
             item != bytes.end(); ++item)
        {
            while (address < item->first)
            {
                size_t count = static_cast<size_t>(
                    std::min<uint64_t>(item->first - address, sizeof(fillBuffer)));
                output.write(fillBuffer, count);
                address += count;
            }
            output.put(static_cast<char>(item->second));
            address = static_cast<uint64_t>(item->first) + 1;
        }

        output.flush();
        if (!output)
            throw std::runtime_error("Ошибка записи BIN-файла.");
    }

    void writeHexRecord(std::ostream& output, unsigned char type, unsigned int address, const unsigned char* data, unsigned int count)
    {
        unsigned int checksum = count + (address >> 8) + (address & 0xFF) + type;

        output << ':' << std::uppercase << std::hex << std::setfill('0')
               << std::setw(2) << count
               << std::setw(4) << address
               << std::setw(2) << static_cast<unsigned int>(type);

        for (unsigned int i = 0; i < count; ++i)
        {
            checksum += data[i];
            output << std::setw(2) << static_cast<unsigned int>(data[i]);
        }
        output << std::setw(2) << ((~checksum + 1) & 0xFF) << '\n';
        if (!output)
            throw std::runtime_error("Ошибка записи HEX-файла.");
    }

    std::string programDirectory()
    {
#ifdef _WIN32
        char path[MAX_PATH];
        DWORD length = GetModuleFileNameA(0, path, MAX_PATH);
        if (length == 0 || length >= MAX_PATH)
            throw std::runtime_error("Не удалось определить папку программы.");

        std::string directory(path, length);
        size_t slash = directory.find_last_of("\\/");
        if (slash != std::string::npos)
            directory.erase(slash + 1);
        return directory;
#else
        return "";
#endif
    }

    std::string removeQuotes(std::string path)
    {
        if (path.size() >= 2 &&
            path[0] == '"' && path[path.size() - 1] == '"')
            path = path.substr(1, path.size() - 2);
        return path;
    }
}
