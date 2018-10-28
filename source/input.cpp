#include "input.h"

Input::Input()
{
    for(int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if(!SDL_JoystickOpen(i))
        {
            DEBUG("SDL_JoystickOpen: %s\n", SDL_GetError());
            return;
        }
    }
}

Input::~Input()
{

}

void Input::get()
{
    std::fill(this->keys.begin(), this->keys.end(), false);
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_JOYBUTTONDOWN)
        {
            int button = event.jbutton.button;
            if(button < JOYSTICK_KEYS_AMOUNT)
                this->keys[button] = true;
        }
    }
}
