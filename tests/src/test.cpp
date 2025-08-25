#include <gtest/gtest.h>

#include <cstdio>

#include "Compress/compresStream.h"
#include "Crypto/cryptoStream.h"
#include "streams/readStream.h"
#include "streams/writeStream.h"

TEST(StreamIntegrationTest, WriteThenReadBlock) {
    const std::string tempFile{"temp_test_file_block.bin"};
    const std::string testData{"Hello, world! This is a test string"};

    // Этап 1: Запись в файл блоком
    {
        FileOutputStream output{tempFile};
        output.WriteBlock(testData.c_str(), testData.size());
    }

    // Этап 2: Чтение из файла блоком
    std::string readData;
    {
        FileInputStream input{tempFile};
        std::vector<char> buffer(testData.size());
        input.ReadBlock(buffer.data(), buffer.size());
        readData.assign(buffer.data(), buffer.size());
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}

TEST(StreamIntegrationTest, WriteThenReadByteByByte) {
    const std::string tempFile{"temp_test_file_byte.bin"};
    const std::string testData{"Hello, world! This is a test string"};

    // Этап 1: Запись по одному байту
    {
        FileOutputStream output{tempFile};
        for (const char byte : testData) {
            output.WriteByte(static_cast<uint8_t>(byte));
        }
    }

    // Этап 2: Чтение по одному байту до конца файла
    std::string readData;
    {
        FileInputStream input{tempFile};
        while (!input.IsEOF()) {
            readData += input.ReadByte();
        }
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}

TEST(StreamExceptionTest, InputThrowsOnNonExistentFile) {
    // Проверяем, что конструктор FileInputStream бросает исключение,
    // если файл не существует.
    ASSERT_THROW(FileInputStream("non_existent_file_12345.tmp"), std::ios_base::failure);
}

// Проверяем, что все операции записи бросают std::logic_error после закрытия потока
TEST(StreamExceptionTest, InputThrowsAfterClose) {
    const std::string tempFile = "test_for_close_input.tmp";
    // Создаем пустой файл для теста
    {
        FileOutputStream output{tempFile};
    }

    FileInputStream input{tempFile};
    input.Close();

    ASSERT_THROW(input.IsEOF(), std::logic_error);
    ASSERT_THROW(input.ReadByte(), std::logic_error);
    char buffer;
    ASSERT_THROW(input.ReadBlock(&buffer, 1), std::logic_error);

    std::remove(tempFile.c_str());
}

// Проверяем, что все операции записи бросают std::logic_error после закрытия потока
TEST(StreamExceptionTest, OutputThrowsAfterClose) {
    const std::string tempFile = "test_for_close_output.tmp";
    FileOutputStream output{tempFile};
    output.Close();

    ASSERT_THROW(output.WriteByte(0), std::logic_error);
    char buffer = 'a';
    ASSERT_THROW(output.WriteBlock(&buffer, 1), std::logic_error);

    std::remove(tempFile.c_str());
}

TEST(CryptoStreamIntegrationTest, EncryptThenDecryptBlock) {
    const std::string tempFile{"temp_crypto_test_block.bin"};
    const std::string testData{"Hello, world! This is a test string"};
    const unsigned key = 42;

    // Этап 1: Создаем файловый поток, оборачиваем его декоратором шифрования и записываем блок
    {
        EncryptingOutputStream output{std::make_unique<FileOutputStream>(tempFile), key};
        output.WriteBlock(testData.c_str(), testData.size());
    }

    // Этап 2: Создаем файловый поток, оборачиваем его декоратором дешифрования и читаем блок
    std::string readData;
    {
        DecryptingInputStream input{std::make_unique<FileInputStream>(tempFile), key};
        std::vector<char> buffer(testData.size());
        input.ReadBlock(buffer.data(), buffer.size());
        readData.assign(buffer.data(), buffer.size());
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}

TEST(CryptoStreamIntegrationTest, EncryptThenDecryptByteByByte) {
    const std::string tempFile{"temp_crypto_test_byte.bin"};
    const std::string testData{"Another crypto test!"};
    const unsigned key = 123;

    // Этап 1: Шифрование и запись по одному байту
    {
        EncryptingOutputStream output(std::make_unique<FileOutputStream>(tempFile), key);
        for (const char byte : testData) {
            output.WriteByte(static_cast<uint8_t>(byte));
        }
    }

    // Этап 2: Чтение и дешифрование по одному байту
    std::string readData;
    {
        DecryptingInputStream input(std::make_unique<FileInputStream>(tempFile), key);
        while (!input.IsEOF()) {
            readData += input.ReadByte();
        }
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}

TEST(CompressStreamIntegrationTest, CompressThenDecompressBlock) {
    const std::string tempFile{"temp_compress_test_block.bin"};
    //const std::string testData = "AAAAAABBBBBBBBBBBBBBBCCCCCCCCCCDDDDDEEEEE";
    const std::string testData = "AE!";

    // Этап 1: Создаем файловый поток, оборачиваем его декоратором сжатия и записываем блок
    {
        CompressingOutputStream output{std::make_unique<FileOutputStream>(tempFile)};
        output.WriteBlock(testData.c_str(), testData.size());
    }

    // Этап 2: Создаем файловый поток, оборачиваем его декоратором сжатия и читаем блок

    std::string readData;
    {
        DecompressingInputStream input{std::make_unique<FileInputStream>(tempFile)};

        char buffer[10];

        
            while (!input.IsEOF()) {
                std::streamsize readSize = input.ReadBlock(buffer, sizeof(buffer));
                readData.append(buffer, readSize);
            }

      
        //   std::vector<char> buffer(testData.size());
        // input.ReadBlock(buffer.data(), buffer.size());
        // readData.assign(buffer.data(), buffer.size());
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}

TEST(CompressStreamIntegrationTest, CompressThenDecompressByteByByte) {
    const std::string tempFile{"temp_compress_test_byte.bin"};
    const std::string testData = "AAAAAABBBBBBBBBBBBBBBCCCCCCCCCCDDDDDEEEEE";

    // Этап 1: Создаем файловый поток, оборачиваем его декоратором сжатия и записываем блок
    {
        CompressingOutputStream output{std::make_unique<FileOutputStream>(tempFile)};
        output.WriteBlock(testData.c_str(), testData.size());
    }

    // Этап 2: Создаем файловый поток, оборачиваем его декоратором сжатия и читаем блок

    std::string readData;
    {
        DecompressingInputStream input(std::make_unique<FileInputStream>(tempFile));
        while (!input.IsEOF()) {
            readData += input.ReadByte();
        }
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}

TEST(CompressStreamIntegrationTest, WorstCaseScenario) {
    const std::string tempFile{"temp_compress_worst_case.bin"};
    // Данные без повторений, где RLE увеличит размер файла
    const std::string testData{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};

    // Этап 1: Сжатие и запись
    {
        CompressingOutputStream output(std::make_unique<FileOutputStream>(tempFile));
        output.WriteBlock(testData.c_str(), testData.size());
    }

    // Этап 2: Чтение и декомпрессия
    std::string readData;
    {
        DecompressingInputStream input(std::make_unique<FileInputStream>(tempFile));
        while (!input.IsEOF()) {
            readData += input.ReadByte();
        }
    }

    // Этап 3: Проверка и очистка
    ASSERT_EQ(testData, readData);
    std::remove(tempFile.c_str());
}
