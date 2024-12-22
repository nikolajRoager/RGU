#include"SDL_functions.hpp"



SDL_Texture* loadTexture(const std::string& path,SDL_Renderer* renderer, SDL_Rect& background_tile_size)
{
    SDL_Texture* Out = nullptr;

    //Load image
    SDL_Surface* surf = IMG_Load(path.c_str());
    if( surf == nullptr)
    {
        std::cout<<"Unable to load file: "<<SDL_GetError()<<std::endl;
        return nullptr;
    }
    Out = SDL_CreateTextureFromSurface(renderer, surf);

    background_tile_size.w = surf->w;
    background_tile_size.h = surf->h;

    SDL_FreeSurface(surf);

    if(Out == nullptr)
    {
        std::cout<<"Unable to create texture: "<<SDL_GetError()<<std::endl;
        return nullptr;
    }
    return Out;


}
