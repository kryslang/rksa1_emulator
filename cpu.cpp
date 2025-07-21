#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <thread>

#define ADD 0x00

uint16_t combine_chars(uint8_t high, uint8_t low) {
  return (static_cast<uint16_t>(high) << 8) | low;
}

inline std::string hex_last_2(const char &c) {
  std::stringstream ss;
  ss << std::setw(2) << std::setfill('0') << std::hex << int{c};
  return ss.str().substr(ss.str().size() - 2, ss.str().size());
}

std::string readFileToString(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    return {};

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0);

  std::string contents(size, '\0');
  file.read(&contents[0], size);
  return contents;
}

int ram_size;
uint16_t reg[19] = {0};
const char *program;
char *ram;
bool halted = false;

void tick() {
  // std::cout << std::hex << int{program[reg[0]]} << std::endl;
  reg[0]++;

  switch (program[reg[0]]) {
  case 0x00: {
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    reg[0x01] = reg[src_reg_a] + reg[src_reg_b];
    break;
  }
  case 0x01: {
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    reg[0x01] = reg[src_reg_a] - reg[src_reg_b];
    break;
  }
  case 0x02: {
    reg[0]++;
    char dest_reg = program[reg[0]];
    reg[0]++;
    char src1 = program[reg[0]];
    reg[0]++;
    char src2 = program[reg[0]];
    uint16_t to_write = combine_chars(src1, src2);
    reg[dest_reg] = program[to_write];  
    break;
  }
  case 0x03: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0]++;
    char dst_reg = program[reg[0]];
    reg[dst_reg] = reg[src_reg];
    break;
  }
  case 0x04: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0]++;
    char src_reg_a = program[reg[0]];
    ram[reg[src_reg]] = reg[src_reg_a];
    break;
  }
  case 0x05: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0]++;
    char dst_reg = program[reg[0]];
    reg[dst_reg] = ram[reg[src_reg]];
    break;
  }
  case 0x06: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0] = reg[src_reg];
    break;
  }
  case 0x07: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    if (reg[src_reg_a] == reg[src_reg_b]) {
      reg[0] = reg[src_reg];
    }
    break;
  }
  case 0x08: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    if (reg[src_reg_a] > reg[src_reg_b]) {
      reg[0] = reg[src_reg];
    }
    break;
  }
  case 0x09: {
    // std::cout << "jmpis" << std::endl;
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    if (reg[src_reg_a] < reg[src_reg_b]) {
      reg[0] = reg[src_reg];
    }
    break;
  }
  case 0x0A: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[src_reg]++;
    break;
  }
  case 0x0B: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[src_reg]--;
    break;
  }
  case 0x0C:
    break;
  case 0x0D:
    break;
  case 0x0E: {
    reg[0]++;
    char src_reg = program[reg[0]];
    uint16_t shifted = reg[src_reg] >> 1;
    reg[src_reg] = shifted;
    break;
  }
  case 0x0F: {
    // std::cout << "halting" << std::endl;
    halted = true;
    break;
  }
  case 0x10: {
    reg[0]++;
    char src_reg = program[reg[0]];
    uint16_t shifted = reg[src_reg] << 1;
    reg[src_reg] = shifted;
    break;
  }
  case 0x11: {
    reg[0]++;
    char src_reg = program[reg[0]];
    reg[0x10] = reg[0];
    reg[0] = reg[src_reg];
    break;
  }
  case 0x12:
    reg[0] = reg[0x10];
    break;
  }

  if (reg[0x09] != 0) {
    char to_write = reg[0x09] & 0xFF;
    std::cout << to_write << std::flush;
    reg[0x09] = 0;
  }
}


int main() {
  ram_size = 512;
  ram = new char[ram_size + 4096]();

  std::string programstr = readFileToString("flash.bin");
  program = programstr.c_str();

  while (!halted) {
    tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}