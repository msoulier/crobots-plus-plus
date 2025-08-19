#!/bin/sh

(git submodule update --init)
(cd lib/SDLx_gpu && git submodule update --init --recursive -- lib/SDL_ttf)
(cd lib/SDLx_gpu && git submodule update --init -- lib/SDLx_model)