#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <fstream>
#include <iomanip>
#include <iostream>

uint8_t pixel_to_byte(Uint8 r, Uint8 g, Uint8 b) {
  uint8_t red = (r * 7) / 255;   // 3 bity → 0–7
  uint8_t green = (g * 3) / 255; // 2 bity → 0–3
  uint8_t blue = (b * 7) / 255;  // 3 bity → 0–7

  return (red << 5) | (green << 3) | blue;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Použití: " << argv[0] << " vstup.png vystup.asm\n";
    return 1;
  }

  const char *input_path = argv[1];
  const char *output_path = argv[2];

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init selhalo: " << SDL_GetError() << "\n";
    return 1;
  }

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    std::cerr << "IMG_Init selhalo: " << IMG_GetError() << "\n";
    SDL_Quit();
    return 1;
  }

  SDL_Surface *image = IMG_Load(input_path);
  if (!image) {
    std::cerr << "Nepodařilo se načíst obrázek: " << IMG_GetError() << "\n";
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  if (image->w != 64 || image->h != 64) {
    std::cerr << "Obrázek musí mít velikost 64x64 pixelů!\n";
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_LockSurface(image);
  Uint32 *pixels = (Uint32 *)image->pixels;

  std::ofstream asm_out(output_path);
  if (!asm_out.is_open()) {
    std::cerr << "Nelze otevřít výstupní soubor!\n";
    SDL_UnlockSurface(image);
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  asm_out << "regmov ramsize reg_b\n";

  SDL_PixelFormat *fmt = image->format;
  for (int i = 0; i < 64 * 64; ++i) {
    Uint8 r, g, b;
    SDL_GetRGB(pixels[i], fmt, &r, &g, &b);
    uint8_t val = pixel_to_byte(r, g, b);

    asm_out << "regwrite reg_a 0x00 0x" << std::hex << std::uppercase
            << std::setw(2) << std::setfill('0') << static_cast<int>(val)
            << std::dec << "\n";

    asm_out << "inc reg_b\n";
    asm_out << "ramw reg_b reg_a\n";
  }
  asm_out << "jmp reg_e\n";

  asm_out.close();
  SDL_UnlockSurface(image);
  SDL_FreeSurface(image);
  IMG_Quit();
  SDL_Quit();

  std::cout << "Generování hotovo: " << output_path << "\n";
  return 0;
}
