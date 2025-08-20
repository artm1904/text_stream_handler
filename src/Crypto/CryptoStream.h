#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <random>
#include <vector>

#include "../streams/IStream.h"

/**
 * @brief Декоратор, добавляющий шифрование к потоку вывода.
 *
 * Оборачивает существующий IOutputDataStream и шифрует все записываемые
 * в него данные с помощью шифра простой замены. Таблица замен генерируется
 * на основе целочисленного ключа.
 */
class EncryptingOutputStream : public IOutputDataStream {
   public:
    /**
     * @brief Конструктор, принимающий поток для декорирования и ключ шифрования.
     * @param stream Умный указатель на оборачиваемый поток вывода.
     * @param key Целочисленный ключ для генерации таблицы шифрования.
     */
    EncryptingOutputStream(IOutputPtr&& fileOutputStream, uint_fast32_t key)
        : _WrappedFileOutputStream(std::move(fileOutputStream)) {
        _EncryptTable.resize(256);
        std::iota(_EncryptTable.begin(), _EncryptTable.end(), 0);
        std::shuffle(_EncryptTable.begin(), _EncryptTable.end(), std::mt19937(key));
    }

    /**
     * @brief Шифрует один байт и записывает его в обернутый поток.
     * @param data Байт для шифрования и записи.
     * @throw std::ios_base::failure в случае ошибки записи в обернутый поток.
     */
    void WriteByte(uint8_t data) override {
        _WrappedFileOutputStream->WriteByte(_EncryptTable[data]);
    };

    /**
     * @brief Шифрует блок данных и записывает его в обернутый поток.
     * @param srcData Указатель на начало блока данных для шифрования.
     * @param size Размер блока данных в байтах.
     * @throw std::ios_base::failure в случае ошибки записи в обернутый поток.
     */
    void WriteBlock(const void* srcData, std::streamsize size) override {
        std::vector<uint8_t> buffer(size);
        for (std::streamsize i = 0; i < size; ++i) {
            uint8_t data = static_cast<const uint8_t*>(srcData)[i];
            buffer[i] = _EncryptTable[data];
        }
        _WrappedFileOutputStream->WriteBlock(buffer.data(), size);
    };

    /**
     * @brief Закрывает обернутый поток.
     *
     * Передает вызов методу Close() нижележащего потока.
     */
    void Close() override { _WrappedFileOutputStream->Close(); };

    /**
     * @brief Деструктор, гарантирующий закрытие потока.
     */
    ~EncryptingOutputStream() override { Close(); };

   private:
    IOutputPtr _WrappedFileOutputStream;
    std::vector<uint8_t> _EncryptTable;
};