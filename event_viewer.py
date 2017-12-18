# Very rough code to get event codes/info

import sys
import ctypes
import pprint
from sdl2 import *
import sdl2.ext

def main():
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)
    window = SDL_CreateWindow(b"Hello World",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              592, 460, SDL_WINDOW_SHOWN)
    windowsurface = SDL_GetWindowSurface(window)

    pads = []

    for n in range(SDL_NumJoysticks()):
        ispad = SDL_IsGameController(n)
        print "joystick", n, "is pad:", ispad
        if ispad:
            p = SDL_GameControllerOpen(n)
            pads.append(p)
            print "opened:", SDL_GameControllerName(p)

    running = True
    event = SDL_Event()
    while running:
        events = sdl2.ext.get_events()
        for ev in events:
            if ev.type == SDL_QUIT:
                running = False

            print "ev type:", ev.type
            if ev.type == SDL_JOYBUTTONDOWN:
                print "joy button down:", ev.jbutton.which, "btn:", ev.jbutton.button, "state:", ev.jbutton.state
            if ev.type == SDL_CONTROLLERBUTTONDOWN:
                print "ctrl button down:", ev.cbutton.which, "btn:", ev.cbutton.button, "state:", ev.cbutton.state
        
        SDL_Delay(10)
        #window.update()

    SDL_DestroyWindow(window)
    SDL_Quit()
    return 0

if __name__ == "__main__":
    sys.exit(main())
