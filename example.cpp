#include "chord.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace chordreader;

namespace {

std::string ReadAll(std::istream& input) {
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

void PrintUsage(const char* exe) {
    std::cerr << "Usage: " << exe << " [chord-file]\n"
              << "       Without an argument, reads sample.txt.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 2) {
        PrintUsage(argv[0]);
        return 2;
    }

    const char* filename = argc == 2 ? argv[1] : "sample.txt";
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open: " << filename << '\n';
        return 1;
    }

    const std::string input = ReadAll(file);

    ChordManager manager;
    std::vector<std::string> errors;
    manager.AddText(input, &errors);

    std::cout << "Input progression:\n";
    std::cout << input << "\n";
    std::cout << "Parsed " << manager.Size() << " chord(s).\n";

    if (const auto key = manager.EstimateKey()) {
        std::cout << "Estimated key: "
                  << GetNoteName(*key, NoteNameStyle::Flats)
                  << " major\n";
        std::cout << "Transposed to C major:\n";
        manager.Transpose(-static_cast<int>(*key));
        manager.WriteTo(std::cout, NoteNameStyle::Flats);
        std::cout << '\n';
    } else {
        std::cout << "Estimated key: unavailable\n";
    }

    if (!errors.empty()) {
        std::cerr << "Skipped invalid chord symbol(s):\n";
        for (const auto& error : errors) {
            std::cerr << "  " << error << '\n';
        }
    }

    return errors.empty() ? 0 : 1;
}
