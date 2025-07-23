#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <thread>

#define ADD 0x00
int ram_size;
uint16_t reg[20] = {0};
const char *program;
char *ram;
bool halted = false;
bool arrowsPressed[4] = {0};
const char *regnames[20] = {"PC",    "AR",      "res",   "res",   "REG_D",
                            "REG_E", "REG_F",   "res",   "res",   "STDOUT",
                            "REG_A", "REG_B",   "REG_C", "IPORT", "OPORT",
                            "FLAG",  "RET_POS", "REG_G", "REG_H", "RAMSIZE"};
SDL_Renderer *renderer;
TTF_Font *font;

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

void draw_thread_func() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  char *reg_text = new char[64]();
  for (int i = 0; i < sizeof(reg) / 2; i++) {
    sprintf(reg_text, "%s %04X", regnames[i], reg[i]);
    renderFont(0, i * 24, font, {255, 255, 255, 255}, renderer, reg_text);
  }
  renderFont(
      450, 300, font, {255, 255, 255, 255}, renderer,
      ("arrowkeys: " + std::to_string(arrowsPressed[0]) + std::to_string(arrowsPressed[1]) +
       std::to_string(arrowsPressed[2]) + std::to_string(arrowsPressed[3]))
          .c_str());
  SDL_Rect r = {449, 49, 258, 258};
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
  SDL_RenderDrawRect(renderer, &r);
  delete[] reg_text;
  for (int y = 0; y < 256; y += 4) {
    for (int x = 0; x < 256; x += 4) {
      auto [r, g, b] = rgb323_to_rgb888(ram[(y / 4 * 64 + x / 4) + ram_size]);
      SDL_SetRenderDrawColor(renderer, r, g, b, 255);
      SDL_Rect rect = {x + 450, y + 50, 4, 4};
      SDL_RenderFillRect(renderer, &rect);
    }
  }
  if (halted) {
    renderFont(300, 10, font, {255, 0, 0, 255}, renderer, "Halted");
  }
  SDL_RenderPresent(renderer);
}

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

void tick() {
  // std::cout << std::hex << int{program[reg[0]]} << std::endl;
  reg[0]++;

  if (arrowsPressed[0]) {
    reg[0x0D] = 0x1111;
  }
  if (arrowsPressed[1]) {
    reg[0x0D] = 0x2222;
  }
  if (arrowsPressed[2]) {
    reg[0x0D] = 0x3333;
  }
  if (arrowsPressed[3]) {
    reg[0x0D] = 0x4444;
  }
  if (!arrowsPressed[0] && !arrowsPressed[1] && !arrowsPressed[2] &&
      !arrowsPressed[3]) {
    reg[0x0D] = 0x00;
  }

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
    reg[dest_reg] = to_write;
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
  case 0x0D: {
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
  case 0x13: {
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    uint16_t reg_a_prev = reg[src_reg_a];
    reg[src_reg_a] = reg[src_reg_b];
    reg[src_reg_b] = reg_a_prev;
    std::cout << "a prev: " << reg_a_prev << " reg b: " << reg[src_reg_b]
              << "reb a: " << reg[src_reg_a] << std::endl;
    break;
  }
  case 0x14: {
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    reg[0x01] = reg[src_reg_a] * reg[src_reg_b];
    break;
  }
  case 0x15: {
    reg[0]++;
    char src_reg_a = program[reg[0]];
    reg[0]++;
    char src_reg_b = program[reg[0]];
    reg[0x01] = reg[src_reg_a] / reg[src_reg_b];
    break;
  }
  }

  if (reg[0x09] != 0) {
    char to_write = reg[0x09] & 0xFF;
    std::cout << to_write << std::flush;
    reg[0x09] = 0;
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

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  font = TTF_OpenFont("IntelOneMono.ttf", 24);
  if (!font) {
    SDL_Log("Chyba načtení fontu: %s", TTF_GetError());
    return 1;
  }

  // SDL_Delay(100);

  SDL_Event event;
  uint16_t framecnt;

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
      }
    }
    tick();
    if (framecnt % 10 == 0) {
      draw_thread_func();
      SDL_Delay(16);
      framecnt = 0;
    }
    framecnt++;
  }

  draw_thread_func();

  SDL_Delay(5000);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}
