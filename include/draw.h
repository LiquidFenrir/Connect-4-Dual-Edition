#pragma once

#include "common.h"

constexpr int SCREEN_HEIGHT = 720;
constexpr int SCREEN_WIDTH = 1280;
constexpr int GRID_SQUARE_SIZE = 64;
constexpr int GRID_BORDER_SIZE = 16;

struct Point {
    int x, y;

    Point(int x, int y) : x(x), y(y) { };
};

struct Texture {
    int width, height;
    SDL_Texture* texture;
};

class Interface {
    public:
        Interface();
        ~Interface();

        void frame_start();
        void frame_end();

        inline void draw_image_at(SDL_Texture* image, const SDL_Rect* dest_rect)
        {
            SDL_RenderCopy(this->renderer, image, NULL, dest_rect);
        }

        inline void draw_image_at(Texture& image, const SDL_Rect* dest_rect)
        {
            this->draw_image_at(image.texture, dest_rect);
        }
        inline void draw_image_at(Texture& image, int x, int y)
        {
            SDL_Rect dest_rect = {x, y, image.width, image.height};
            this->draw_image_at(image.texture, &dest_rect);
        }

        inline void draw_image_at(const std::string& image, const SDL_Rect* dest_rect)
        {
            this->draw_image_at(this->images[image], dest_rect);
        }
        inline void draw_image_at(const std::string& image, int x, int y)
        {
            this->draw_image_at(this->images[image], x, y);
        }

        SDL_Renderer* renderer = nullptr;
        std::map<std::string, Texture> images;

        bool ready = false;

    private:
        void create_compound_images();

        SDL_Window* window = nullptr;
};
