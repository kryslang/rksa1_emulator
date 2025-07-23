#include <fstream>
#include <iostream>
#include <vector>

std::vector<std::string> read_lines_from_file(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}

int main(){
    std::vector<std::string> lines = read_lines_from_file("main.c");
    for(std::string line : lines){
        
    }
    return 0;
}