#pragma once
#include <fstream>
#include <string>

#include "IStream.h"

class FileInputStream : public IInputDataStream {
   public:
    /**
     *  @brief  Конструктор, открывающий файл.
     *  @throw  В случае ошибки открытия файла выбрасывает исключение std::ios_base::failure
     */
    explicit FileInputStream(const std::string& fileName) {
        _FileStream.open(fileName, std::ios::binary);

        if (_FileStream.is_open() == false) {
            throw std::ios_base::failure("Failed to open file!");
        }

        _FileStream.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    }

    /**
     *  @brief  Возвращает признак достижения конца данных потока. Если мы в конце, peek() вернет
     * EOF и установит флаг eofbit.
     *  @throw  Выбрасывает исключение std::ios_base::failuer в случае ошибки чтения или
     * std::logic_error, если поток был закрыт
     *  @return true/false достижения конца файла
     */
    bool IsEOF() const override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }
        return _FileStream.peek() == EOF;
    }

    /**
     *  @brief  Считывает байт из потока.
     *  @throw  Выбрасывает исключение std::ios_base::failuer в случае ошибки чтения или
     * std::logic_error, если поток был закрыт
     *  @return Один прочитанный байт из потока
     */
    uint8_t ReadByte() override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }
        return static_cast<uint8_t>(_FileStream.get());
    }

    /**
     *  @brief  Считывает из потока блок данных размером size байт, записывая его в память по адресу
     * dstBuffer
     *  @throw  Выбрасывает исключение std::ios_base::failuer в случае ошибки чтения или
     * std::logic_error, если поток был закрыт
     *  @return Возвращает количество реально прочитанных байт.
     */
    std::streamsize ReadBlock(void* dstBuffer, std::streamsize size) override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }
        try {
            _FileStream.read(static_cast<char*>(dstBuffer), size);
        } catch (const std::ios_base::failure&) {
            // Игнорируем исключение, только если оно вызвано достижением конца файла.
            if (_FileStream.eof() == false) {
                throw;
            }
        }
        return _FileStream.gcount();
    }

    /**
     *  @brief  Закрывает поток. Операции над ним после этого должны выбрасывать исключение
     * logic_error
     */
    void Close() override {
        if (_IsClosed == false) {
            _FileStream.close();
            _IsClosed = true;
        }
    }

    ~FileInputStream() override { Close(); }

   private:
    // Для чтения из файла используем ifstream
    mutable std::ifstream _FileStream;
    bool _IsClosed = false;
};

