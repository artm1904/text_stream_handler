#include <gtest/gtest.h>

#include <cstdio>

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
