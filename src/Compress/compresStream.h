#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "../streams/IStream.h"

/**
 * @brief Декоратор, добавляющий RLE-сжатие к потоку вывода.
 *
 * Алгоритм сжатия группирует последовательности одинаковых байт.
 * Каждая группа кодируется парой: (count, byte), где count - количество повторений.
 * Максимальная длина группы - 255.
 */
class CompressingOutputStream : public IOutputDataStream {
   public:
    CompressingOutputStream(IOutputPtr&& fileOutputStream)
        : _WrappedFileOutputStream(std::move(fileOutputStream)) {}

    void WriteByte(uint8_t data) override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }

        if (_Count == 0) {
            _Char = data;
            _Count = 1;
        } else if (_Char == data && _Count < 255) {
            ++_Count;
        } else {
            Flush();
            _Char = data;
            _Count = 1;
        }
    };

    void WriteBlock(const void* srcData, std::streamsize size) override {
        if (_IsClosed == true) {
            throw std::logic_error("Stream is closed");
        }

        const auto* data = static_cast<const uint8_t*>(srcData);
        for (std::streamsize i = 0; i < size; ++i) {
            WriteByte(data[i]);
        }
    };

    void Close() override {
        if (_IsClosed == false) {
            Flush();
            _WrappedFileOutputStream->Close();
            _IsClosed = true;
        }
    };

    ~CompressingOutputStream() override {
        try {
            Close();
        } catch (...) {
        }
    }

   private:
    void Flush() {
        if (_Count > 0) {
            _WrappedFileOutputStream->WriteByte(_Count);
            _WrappedFileOutputStream->WriteByte(_Char);
            _Count = 0;
        }
    }
    IOutputPtr _WrappedFileOutputStream;
    uint8_t _Char = 0;
    uint8_t _Count = 0;
    bool _IsClosed = false;
};

/**
 * @brief Декоратор, добавляющий RLE-декомпрессию к потоку ввода.
 *
 * Читает данные, сжатые с помощью CompressingOutputStream, и восстанавливает
 * исходную последовательность байт.
 */
class DecompressingInputStream : public IInputDataStream {
   public:
    DecompressingInputStream(IInputPtr&& fileInputStream)
        : _WrappedFileInputStream(std::move(fileInputStream)) {}

    bool IsEOF() const override {
        if (_IsClosed) {
            throw std::logic_error("Stream is closed");
        }
        return (_Count == 0 && _WrappedFileInputStream->IsEOF());
    };

    uint8_t ReadByte() override {
        if (_IsClosed) {
            throw std::logic_error("Stream is closed");
        }

        if (_Count == 0) {
            //_Count = _WrappedFileInputStream->ReadByte();
            //_Char = _WrappedFileInputStream->ReadByte();

            // Читаем пару [count, char] как блок.
            uint8_t pair[2];
            std::streamsize bytesRead = _WrappedFileInputStream->ReadBlock(pair, 2);

            if (bytesRead < 2) {
                // файл поврежден (имеет нечетное количество байт).
                throw std::ios_base::failure("RLE format error: truncated data pair");
            }
            _Count = pair[0];
            _Char = pair[1];
        }

        --_Count;
        return _Char;
    }

    std::streamsize ReadBlock(void* dstBuffer, std::streamsize size) override {
        if (_IsClosed) {
            throw std::logic_error("Stream is closed");
        }

        auto* buffer = static_cast<uint8_t*>(dstBuffer);
        std::streamsize readSize = 0;

        for (; readSize < size && !IsEOF(); ++readSize) {
            buffer[readSize] = ReadByte();
        }
        return readSize;
    };

    void Close() override {
        if (_IsClosed == false) {
            _WrappedFileInputStream->Close();
            _IsClosed = true;
        }
    };

    ~DecompressingInputStream() override { Close(); };

   private:
    IInputPtr _WrappedFileInputStream;
    uint8_t _Char = 0;
    uint8_t _Count = 0;
    bool _IsClosed = false;
};
