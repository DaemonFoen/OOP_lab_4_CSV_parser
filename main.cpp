#include "CSV_parser.h"

// Empty data will cause empty-error

int main(int argc, char* argv[]) {
    try {
        if (argc < 3)
            throw std::runtime_error("bad number of arguments");

        std::ifstream file("test1.csv");
        if (!file.is_open())
            throw std::runtime_error("file error");

        CSV_parser<int, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string> parser(argv, file, 1);
        for (auto it = parser.begin(); it != parser.end(); ++it ) {
            //std::get<5>(it.res_tp) -=100;
            std::cout << *it;
            std::cout << std::endl;
        }
        //std::cout << end - beg;
        return 0;
    }
    catch (std::exception &ex) {
        std::cerr << "\nERROR: " << ex.what();
        return 1;
    }
}
