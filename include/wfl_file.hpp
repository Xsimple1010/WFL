#ifndef WFL_FILE_H
#define WFL_FILE_H

#include <SDL.h>
#include <SDL_stdinc.h>
#include <iostream>

bool wflWriterData(void* data, const char* fileName, size_t size);
void* wflLoadFileFromSpecificSize(const char* fileName, size_t size);
void* wflLoadFile(const char* path, size_t* size);

#endif