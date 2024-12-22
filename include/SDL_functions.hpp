//A few functions, for doing various SDL procedures

#include<iostream>
#include<string>
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

SDL_Texture* loadTexture(const std::string& path,SDL_Renderer* r, SDL_Rect& background_tile_size);
