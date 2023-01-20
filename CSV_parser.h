#ifndef LAB4_CSV_PARSER_CSV_PARSER_H
#define LAB4_CSV_PARSER_CSV_PARSER_H

#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <typeinfo>

template<std::size_t>
struct str_pos {
};

template<typename ...Args>
class CSV_parser {
private:
    char separator;              // Data separator value.
    char end_line;               // End of line value.
    char shield;                 // Shielding value.
    int line_num;                // Number of current line.
    std::ifstream file;          // Current csv file.
    std::string cur_line;        // Current line .
    std::tuple<Args ...> res_tp; // Result tuple.

    template<typename Iter>
    class CSV_iterator {
    public:
        Iter* pointer;

        bool operator!=(CSV_iterator const &other) const {
            return pointer != other.pointer;
        }

        Iter& operator*() const {
            return *pointer;
        }

        CSV_iterator& operator++() {
            if (pointer->file.eof()) {
                *this = nullptr;
                return *this;
            }
            std::getline(pointer->file, pointer->cur_line, pointer->end_line);
            pointer->template str_to_tuple<Args...>(0, str_pos<0>());
            pointer->line_num++;
            return *this;
        }

        friend class CSV_parser;

        CSV_iterator(Iter* iter) : pointer(iter) {}; // initialization with point

        CSV_iterator(const CSV_iterator &it) : pointer(it.pointer) {} // copy construct
    };    // Iterator class.


    void arg(char **argv) {
        separator = *argv[1];
        end_line = *argv[2];
        if (end_line == '|')
            end_line = '\n';
        if (argv[3] != nullptr) {
            shield = *argv[3];
        }else
            shield = '"';
    }

    // Convert string to required tuple.
    template<typename Head, typename ...Params, std::size_t Pos>
    void str_to_tuple(int cur_pos, str_pos<Pos>) {
        try {
            Head head;
            int comma_pos;
            std::string data;
            if (cur_line[cur_pos] == shield) {
                data = get_substr(shield, cur_pos + 1, comma_pos);
                comma_pos++;
            } else
                data = get_substr(separator, cur_pos, comma_pos);

            str_to_head(cur_pos, comma_pos, data, head);

            std::get<Pos>(res_tp) = head;

            cur_pos = (comma_pos + 1) % (cur_line.size() + 1);
            str_to_tuple<Params...>(cur_pos, str_pos<Pos + 1>());
        }
        catch (const std::exception &ex) {
            throw std::runtime_error("Wrong length of " + std::to_string(line_num) + " string (too short)");
        }
    }

    // Recursion bottom.
    template<typename ...Params>
    void str_to_tuple(int cur_pos, str_pos<sizeof...(Args)>) {
        if (cur_pos != 0) {
            throw std::runtime_error("Wrong length of " + std::to_string(line_num) + " string ");
        }
    }

    // Convert substring to Head elem, which type is Head.
    template<typename Head>
    void str_to_head(int cur_pos, int comma_pos, const std::string &data, Head &head) {
        std::istringstream ist(data);
        ist >> head;
        if (!ist.eof())
            throw std::runtime_error("bad data at" + std::to_string(cur_pos) + "position");
    }

    // Convert substring to Head elem, which type is std::string.
    void str_to_head(int cur_pos, int comma_pos, const std::string &data, std::string &head) {
        head = data;
    }

    // Convert substring to Head elem, which type is char.
    void str_to_head(int cur_pos, int comma_pos, const std::string &data, char &head) {
        if (data.size() != 1)
            throw std::runtime_error("bad data at" + std::to_string(cur_pos) + "position");
        head = static_cast<char>(data[0]);
    }

    // Return a substring.
    std::string get_substr(char sep, int cur_pos, int &comma_pos) {
        comma_pos = static_cast<int>(cur_line.find(sep, cur_pos));
        if (comma_pos == std::string::npos && sep == shield) {                   // If open-shield symbol don't have close.
            std::string elem = get_substr(separator, cur_pos - 1, comma_pos);
            comma_pos--;
            return elem;
        }
        std::string elem = cur_line.substr(cur_pos, comma_pos - cur_pos);
        if (elem.empty())
            throw std::runtime_error("empty data at" + std::to_string(cur_pos) + "position");;
        return elem;
    }

    // Return position of bad interval.
    std::string bad_data(int a) {
        return "Bad data in " + std::to_string(line_num) + " row, in " + std::to_string(a % cur_line.size()) +
               " column";
    }

    // Return position of bad interval.
    std::string bad_data(int a, std::istringstream &ist) {
        return "Bad data in " + std::to_string(line_num) + " row, in " + std::to_string(a + ist.tellg() + 1) +
               " column";
    }

    // Return position of empty interval.
    std::string empty_data(int a) {
        return "Empty data in " + std::to_string(line_num) + " row, in " + std::to_string(a) + " column";
    }

public:

    std::tuple<Args ...> get_tuple() {
        return res_tp;
    }

    CSV_iterator<CSV_parser> begin() {
        CSV_parser<Args...> *csv_parser = this;
        return CSV_iterator<CSV_parser>(csv_parser);
    }

    CSV_iterator<CSV_parser> end() {
        return nullptr;
    }

    CSV_parser() = default;

    // Initial fields, skip lines and convert first string to tuple.
    CSV_parser(char **argv, std::basic_ios<char> &fin, int skip) {
        file.basic_ios<char>::rdbuf(fin.rdbuf());
        arg(argv);


        for (int i = 0; i < skip; i++)
            if (!std::getline(file, cur_line, end_line))
                throw std::runtime_error("error with skip lanes");

        line_num = skip + 1;
        std::getline(file, cur_line, end_line);
        str_to_tuple<Args...>(0, str_pos<0>());
        line_num++;
    }

    ~CSV_parser() = default;
};



template<class Tuple, typename CharT, typename Traits>
std::ostream &print_tuple(std::basic_ostream<CharT, Traits> &os, const Tuple &tp, str_pos<1>) {
    os << std::get<std::tuple_size<Tuple>::value - 1>(tp);
    return os;
}

template<class Tuple, typename CharT, typename Traits, std::size_t Pos>
std::ostream& print_tuple(std::basic_ostream<CharT, Traits> &os, const Tuple &tp, str_pos<Pos>) {
    os << std::get<std::tuple_size<Tuple>::value - Pos>(tp) << ", ";
    return print_tuple(os, tp, str_pos<Pos - 1>());
}

template<typename ...Args, typename CharT, typename Traits>
auto operator<<(std::basic_ostream<CharT, Traits>& os, std::tuple<Args...> const &tp) {
    print_tuple(os, tp, str_pos<sizeof...(Args)>());
    return;
}

template<typename ...Args, typename ChartT, typename Traits>
auto operator<<(std::basic_ostream<ChartT, Traits>& os, CSV_parser<Args...>& parser) {
    return os << parser.get_tuple();
}


#endif //LAB4_CSV_PARSER_CSV_PARSER_H
