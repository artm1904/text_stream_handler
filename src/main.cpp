#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "Compress/compresStream.h"
#include "Crypto/cryptoStream.h"
#include "streams/readStream.h"
#include "streams/writeStream.h"

void TransformData(IInputDataStream& input, IOutputDataStream& output) {
    std::vector<char> buffer(4096);
    while (!input.IsEOF()) {
        std::streamsize size = input.ReadBlock(buffer.data(), buffer.size());
        if (size > 0) {
            output.WriteBlock(buffer.data(), size);
        }
    }
}

int main(int argc, char** argv)

{
    if (argc < 3) {
        std::cerr << "Wrong input parameters" << std::endl;
        std::cerr << "Invalid arguments. Usage:" << std::endl;
        std::cerr << "  " << argv[0] << " [options] <input-file> <output-file>" << std::endl;
        return 1;
    }

    try {
        std::string inputFile = argv[argc - 2];
        std::string outputFile = argv[argc - 1];

        IInputPtr inputStream = std::make_unique<FileInputStream>(inputFile);
        IOutputPtr outputStream = std::make_unique<FileOutputStream>(outputFile);

        // "Оборачиваем" потоки декораторами в соответствии с опциями в порядке передачи параметров
        for (int i = 1; i < argc - 2; ++i) {
            std::string option = argv[i];

            if (option == "--compress") {
                outputStream = std::make_unique<CompressingOutputStream>(std::move(outputStream));
            } else if (option == "--decompress") {
                inputStream = std::make_unique<DecompressingInputStream>(std::move(inputStream));
            } else if (option == "--encrypt") {
                if (i + 1 >= argc - 2) {
                    throw std::invalid_argument("Missing key for --encrypt option");
                }
                i++;  // Переходим к аргументу с ключом
                try {
                    uint32_t key = (uint32_t)std::stoul(argv[i]);
                    outputStream =
                        std::make_unique<EncryptingOutputStream>(std::move(outputStream), key);
                } catch (const std::exception&) {
                    throw std::invalid_argument("Invalid key for --encrypt option: " +
                                                std::string(argv[i]));
                }
            } else if (option == "--decrypt") {
                if (i + 1 >= argc - 2) {
                    throw std::invalid_argument("Missing key for --decrypt option");
                }
                i++;  // Переходим к аргументу с ключом
                try {
                    uint32_t key = (uint32_t)std::stoul(argv[i]);
                    inputStream =
                        std::make_unique<DecryptingInputStream>(std::move(inputStream), key);
                } catch (const std::exception&) {
                    throw std::invalid_argument("Invalid key for --decrypt option: " +
                                                std::string(argv[i]));
                }
            } else {
                throw std::invalid_argument("Invalid option: " + option);
            }

            // Using the constracted decorator for input and output stream
            TransformData(*inputStream, *outputStream);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return 1;
    }

    return 0;
}
