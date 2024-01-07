#include "wfl_file.hpp"


bool wflWriterData(void* data, const char* fileName, size_t size) {
    SDL_RWops* file = SDL_RWFromFile(fileName, "wb");
    
    if(!SDL_RWwrite(file, data, size, 1)) {
        printf("Could not write savestate dump to '%s'", SDL_GetError());
        return false;
    }

    SDL_RWclose(file);

    return true;
}

void* wflLoadFileFromSpecificSize(const char* fileName, size_t size) {
    SDL_RWops* file = SDL_RWFromFile(fileName, "rb");

    void* data = SDL_malloc(size);

    if (!data) {
        printf("Failed to allocate memory for the content");
        SDL_RWclose(file);
        SDL_free(data);
        return NULL;
    }

    if (!SDL_RWread(file, (void*)data, size, 1)) {
        printf("Failed to read file data: %s", SDL_GetError());
        SDL_RWclose(file);
        SDL_free(data);
        return NULL;
    }

    SDL_RWclose(file);
    
    return data;
}

void* wflLoadFile(const char* fileName, size_t* size) {
    SDL_RWops* file = SDL_RWFromFile(fileName, "rb");

    if (!file) {
        printf("Failed to load %s: %s", fileName, SDL_GetError());
        SDL_RWclose(file);
        return NULL;
    }

    *size = SDL_RWsize(file);
    void* data = SDL_malloc(*size);

    if (*size  < 0) {
        printf("Failed to query game file size: %s", SDL_GetError());
        SDL_RWclose(file);
        return NULL;
    }

    if (!data) {
        printf("Failed to allocate memory for the content");
        SDL_RWclose(file);
        return NULL;
    }

    if (!SDL_RWread(file, (void*)data, *size, 1)) {
        printf("Failed to read file data: %s", SDL_GetError());
        SDL_RWclose(file);
        return NULL;
    }

    SDL_RWclose(file);

    return data;
}