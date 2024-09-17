#include "pch.hpp"

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();

    SDL_Window* window;
#ifdef TL_ANDROID
    window = SDL_CreateWindow("Treasure Looter", 0, 0, 0, 0, SDL_WINDOW_SHOWN);
#else
    window = SDL_CreateWindow("Treasure Looter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 720, SDL_WINDOW_SHOWN);
#endif

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Event event;
    bool shouldExit = false;
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    while(!shouldExit) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldExit = true;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
