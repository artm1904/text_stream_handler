#pragma once
#include <fstream>
#include <string>

#include "IStream.h"

class FileOutputStream : public IOutputDataStream {
   public:
    /**
     *  @brief  Конструктор, открывающий файл.
     *  @throw  В случае ошибки открытия файла выбрасывает исключение std::ios_base::failure
     */
    explicit FileOutputStream(const std::string& fileName) {
        _FileStream.open(fileName, std::ios::binary);

        if (_FileStream.is_open() == false) {
            throw std::ios_base::failure("Failed to open file!");
        }

        _FileStream.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    }

    /**
     *  @brief  Записывает в поток данных байт
     *  @throw  Выбрасывает исключение std::ios_base::failuer в случае ошибки чтения или
     * std::logic_error, если поток был закрыт
     */
    void WriteByte(uint8_t data) override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }
        _FileStream.put(data);
    }

    /**
     *  @brief Записывает в поток блок данных размером size байт, располагающийся по адресу srcData,
     *  @throw  Выбрасывает исключение std::ios_base::failuer в случае ошибки чтения или
     * std::logic_error, если поток был закрыт
     */
    void WriteBlock(const void* srcData, std::streamsize size) override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }

        _FileStream.write(static_cast<const char*>(srcData), size);
    }

    /**
     *  @brief Закрывает поток. Операции над ним после этого должны выбрасывать исключение
     * logic_error
     */
    void Close() override {
        if (_IsClosed == false) {
            _FileStream.close();
            _IsClosed = true;
        }
    }

    ~FileOutputStream() { Close(); };

   private:
    mutable std::ofstream _FileStream;
    bool _IsClosed = false;
};