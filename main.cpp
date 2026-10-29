#include "Converter.h"
#include "IntelHexUtils.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace std;

int main()
{
    system("chcp 65001 > nul");

    try
    {
        int conversion;
        cout << "Преобразование файлов Intel HEX и BIN\n"
             << "1 - HEX в BIN\n"
             << "2 - BIN в HEX\n"
             << "Выберите действие: ";
        if (!(cin >> conversion) || (conversion != 1 && conversion != 2))
            throw runtime_error("Введите 1 или 2.");
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        int pathChoice;
        cout << "1 - использовать файлы по умолчанию\n"
             << "2 - указать полный путь к файлам\n"
             << "Выберите действие: ";
        if (!(cin >> pathChoice) || (pathChoice != 1 && pathChoice != 2))
            throw runtime_error("Введите 1 или 2.");
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string inputName;
        string outputName;
        if (pathChoice == 1)
        {
            string directory = IntelHexUtils::programDirectory();
            inputName = directory +
                (conversion == 1 ? "hexInput.hex" : "binInput.bin");
            outputName = directory +
                (conversion == 1 ? "binOutput.bin" : "hexOutput.hex");
            cout << "Исходный файл: " << inputName << '\n'
                 << "Результирующий файл: " << outputName << '\n';
        }
        else
        {
            cout << "Полный путь к исходному файлу: ";
            getline(cin, inputName);
            cout << "Полный путь к результирующему файлу: ";
            getline(cin, outputName);
            inputName = IntelHexUtils::removeQuotes(inputName);
            outputName = IntelHexUtils::removeQuotes(outputName);
        }

        if (inputName.empty() || outputName.empty())
            throw runtime_error("Укажите оба имени файлов.");
        if (inputName == outputName)
            throw runtime_error("Исходный и результирующий файлы должны различаться.");

        if (conversion == 1)
            Converter::hexToBin(inputName, outputName);
        else
            Converter::binToHex(inputName, outputName);

        cout << "Преобразование завершено.\n";
    }
    catch (const exception& error)
    {
        cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
