#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <thread>

int ram_size;
uint16_t reg[20] = {0};
const char *program;
char *ram;
bool halted = false;
bool arrowsPressed[5] = {0};
uint64_t clocks = 0;
uint64_t old_clocks = 0;
uint64_t hz = 0;
SDL_Surface *icon=IMG_Load("icon.png");

const char *regnames[20] = {"PC",    "AR",      "REG_I", "REG_J", "REG_D",
                            "REG_E", "REG_F",   "REG_K", "REG_L", "STDOUT",
                            "REG_A", "REG_B",   "REG_C", "IPORT", "OPORT",
                            "FLAG",  "RET_POS", "REG_G", "REG_H", "RAMSIZE"};
SDL_Renderer *renderer;
TTF_Font *font;

enum regs {
  PC = 0,
  AR = 1,
  REG_I = 2,
  REG_J = 3,
  REG_D = 4,
  REG_E = 5,
  REG_F = 6,
  REG_K = 7,
  REG_L = 8,
  STDOUT = 9,
  REG_A = 10,
  REG_B = 11,
  REG_C = 12,
  IPORT = 13,
  OPORT = 14,
  FLAG = 15,
  RET_POS = 16,
  REG_G = 17,
  REG_H = 18,
  RAMSIZE = 19
};
enum ops {
  ADD = 0,
  SUB = 1,
  REGWRITE = 2,
  REGMOV = 3,
  RAMW = 4,
  RAMR = 5,
  JMP = 6,
  JPIE = 7,
  JPIG = 8,
  JPIS = 9,
  INC = 10,
  DEC = 11,
  NOP = 12,
  JPINE = 13,
  BITSL = 14,
  HALT = 15,
  BITSR = 16,
  CALL = 17,
  RET = 18,
  REGSWAP = 19,
  MUL = 20,
  DIV = 21
};

void updateHz() {
  while (true) {
    hz = clocks - old_clocks;
    old_clocks = clocks;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

std::tuple<uint8_t, uint8_t, uint8_t> rgb323_to_rgb888(uint8_t rgb323) {
  // Vytáhneme jednotlivé komponenty
  uint8_t r3 = (rgb323 >> 5) & 0x07; // 3 bity (R)
  uint8_t g2 = (rgb323 >> 3) & 0x03; // 2 bity (G)
  uint8_t b3 = rgb323 & 0x07;        // 3 bity (B)

  // Převod na 8bit barvy (škálování)
  uint8_t r8 = (r3 * 255) / 7;
  uint8_t g8 = (g2 * 255) / 3;
  uint8_t b8 = (b3 * 255) / 7;

  return {r8, g8, b8};
}

void renderFont(int x, int y, TTF_Font *font, SDL_Color clr,
                SDL_Renderer *renderer, const char *text) {

  SDL_Surface *surface = TTF_RenderUTF8_Solid(font, text, clr);
  if (!surface) {
    SDL_Log("Chyba renderování textu: %s", TTF_GetError());
    return;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    SDL_Log("Chyba vytváření textury: %s", SDL_GetError());
    SDL_FreeSurface(surface);
    return;
  }

  SDL_Rect dst = {x, y, surface->w, surface->h};
  SDL_FreeSurface(surface);

  SDL_RenderCopy(renderer, texture, NULL, &dst);
  SDL_DestroyTexture(texture);
}

void draw() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  char *reg_text = new char[64]();
  for (int i = 0; i < sizeof(reg) / 2; i++) {
    sprintf(reg_text, "%s %04X", regnames[i], reg[i]);
    renderFont(0, i * 24, font, {255, 255, 255, 255}, renderer, reg_text);
  }
  renderFont(200, 570, font, {255, 255, 255, 255}, renderer,
             (std::to_string(hz / 1000) + "kHz").c_str());
  renderFont(400, 540, font, {255, 255, 255, 255}, renderer,
             ("arrowkeys: " + std::to_string(arrowsPressed[0]) +
              std::to_string(arrowsPressed[1]) +
              std::to_string(arrowsPressed[2]) +
              std::to_string(arrowsPressed[3]))
                 .c_str());
  SDL_Rect r = {267, 19, 514, 514};
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
  SDL_RenderDrawRect(renderer, &r);
  delete[] reg_text;
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      auto [r, g, b] = rgb323_to_rgb888(ram[(y * 64 + x) + ram_size]);
      SDL_SetRenderDrawColor(renderer, r, g, b, 255);
      SDL_Rect rect = {x * 8 + 268, y * 8 + 20, 8, 8};
      SDL_RenderFillRect(renderer, &rect);
    }
  }
  std::stringstream ss;
  for (int i = 0; i < 8; i++) {
    ss << std::to_string((uint16_t)ram[i]);
    ss << " ";
  }

  renderFont(400, 570, font, {255, 255, 255, 255}, renderer, ss.str().c_str());

  if (halted) {
    renderFont(0, 560, font, {255, 0, 0, 255}, renderer, "Halted: true");
  } else {
    renderFont(0, 560, font, {0, 255, 0, 255}, renderer, "Halted: false");
  }
  SDL_RenderPresent(renderer);
}

uint16_t combine_chars(uint8_t high, uint8_t low) {
  return (static_cast<uint16_t>(high) << 8) | low;
}

std::string hex_last_2(const char &c) {
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

void tick() {
  // std::cout << std::hex << int{program[reg[PC]]} << std::endl;
  reg[PC]++;

  if (arrowsPressed[0]) {
    reg[IPORT] = 0x1111;
  } else if (arrowsPressed[1]) {
    reg[IPORT] = 0x2222;
  } else if (arrowsPressed[2]) {
    reg[IPORT] = 0x3333;
  } else if (arrowsPressed[3]) {
    reg[IPORT] = 0x4444;
  } else if (arrowsPressed[4]) {
    reg[IPORT] = 0x5555;
  } else {
    reg[IPORT] = 0x0000;
  }

  switch (program[reg[PC]]) {
  case ADD: {
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    reg[AR] = reg[src_reg_a] + reg[src_reg_b];
    break;
  }
  case SUB: {
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    reg[AR] = reg[src_reg_a] - reg[src_reg_b];
    break;
  }
  case REGWRITE: {
    reg[PC]++;
    char dest_reg = program[reg[PC]];
    reg[PC]++;
    char src1 = program[reg[PC]];
    reg[PC]++;
    char src2 = program[reg[PC]];
    uint16_t to_write = combine_chars(src1, src2);
    reg[dest_reg] = to_write;
    break;
  }
  case REGMOV: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char dst_reg = program[reg[PC]];
    reg[dst_reg] = reg[src_reg];
    break;
  }
  case RAMW: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    ram[reg[src_reg]] = reg[src_reg_a];
    break;
  }
  case RAMR: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char dst_reg = program[reg[PC]];
    reg[dst_reg] = ram[reg[src_reg]];
    break;
  }
  case JMP: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC] = reg[src_reg];
    break;
  }
  case JPIE: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    if (reg[src_reg_a] == reg[src_reg_b]) {
      reg[PC] = reg[src_reg];
    }
    break;
  }
  case JPIG: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    if (reg[src_reg_a] > reg[src_reg_b]) {
      reg[PC] = reg[src_reg];
    }
    break;
  }
  case JPIS: {
    // std::cout << "jmpis" << std::endl;
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    if (reg[src_reg_a] < reg[src_reg_b]) {
      reg[PC] = reg[src_reg];
    }
    break;
  }
  case INC: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[src_reg]++;
    break;
  }
  case DEC: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[src_reg]--;
    break;
  }
  case NOP:
    break;
  case JPINE: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    if (reg[src_reg_a] != reg[src_reg_b]) {
      reg[PC] = reg[src_reg];
    }
    break;
  }
  case BITSL: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    uint16_t shifted = reg[src_reg] >> 1;
    reg[src_reg] = shifted;
    break;
  }
  case HALT: {
    // std::cout << "halting" << std::endl;
    halted = true;
    break;
  }
  case BITSR: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    uint16_t shifted = reg[src_reg] << 1;
    reg[src_reg] = shifted;
    break;
  }
  case CALL: {
    reg[PC]++;
    char src_reg = program[reg[PC]];
    reg[RET_POS] = reg[PC];
    reg[PC] = reg[src_reg];
    break;
  }
  case RET:
    reg[PC] = reg[RET_POS];
    break;
  case REGSWAP: {
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    uint16_t reg_a_prev = reg[src_reg_a];
    reg[src_reg_a] = reg[src_reg_b];
    reg[src_reg_b] = reg_a_prev;
    std::cout << "a prev: " << reg_a_prev << " reg b: " << reg[src_reg_b]
              << "reb a: " << reg[src_reg_a] << std::endl;
    break;
  }
  case MUL: {
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    reg[AR] = reg[src_reg_a] * reg[src_reg_b];
    break;
  }
  case DIV: {
    reg[PC]++;
    char src_reg_a = program[reg[PC]];
    reg[PC]++;
    char src_reg_b = program[reg[PC]];
    reg[AR] = reg[src_reg_a] / reg[src_reg_b];
    break;
  }
  }

  if (reg[STDOUT] != 0) {
    char to_write = reg[STDOUT] & 0xFF;
    std::cout << to_write << std::flush;
    reg[STDOUT] = 0;
  }
}

int main() {
  ram_size = 1024;
  ram = new char[ram_size + 4096]();

  std::string programstr = readFileToString("flash.bin");
  program = programstr.c_str();

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return -1;
  }

  if (TTF_Init() < 0) {
    std::cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError()
              << std::endl;
    return -1;
  }

  SDL_Window *window =
      SDL_CreateWindow("CPU Emulator", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

  if (!window) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    return -1;
  }
  SDL_SetWindowIcon(window, icon);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  font = TTF_OpenFont("IntelOneMono.ttf", 18);
  if (!font) {
    SDL_Log("Chyba načtení fontu: %s", TTF_GetError());
    return 1;
  }

  // SDL_Delay(100);

  SDL_Event event;

  std::thread updateHzThread(updateHz);

  reg[19] = ram_size;

  while (!halted) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        halted = true;
      }
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_UP) {
          arrowsPressed[0] = true;
        }
        if (event.key.keysym.sym == SDLK_LEFT) {
          arrowsPressed[1] = true;
        }
        if (event.key.keysym.sym == SDLK_DOWN) {
          arrowsPressed[2] = true;
        }
        if (event.key.keysym.sym == SDLK_RIGHT) {
          arrowsPressed[3] = true;
        }
        if (event.key.keysym.sym == SDLK_SPACE) {
          arrowsPressed[4] = true;
        }
      }
      if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_UP) {
          arrowsPressed[0] = false;
        }
        if (event.key.keysym.sym == SDLK_LEFT) {
          arrowsPressed[1] = false;
        }
        if (event.key.keysym.sym == SDLK_DOWN) {
          arrowsPressed[2] = false;
        }
        if (event.key.keysym.sym == SDLK_RIGHT) {
          arrowsPressed[3] = false;
        }
        if (event.key.keysym.sym == SDLK_SPACE) {
          arrowsPressed[4] = false;
        }
      }
    }
    for (int i = 0; i < 1000; i++) {
      tick();
      clocks++;
      //SDL_Delay(10);
    }
    draw();
    SDL_Delay(10);
  }

  draw();

  SDL_Delay(5000);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}
