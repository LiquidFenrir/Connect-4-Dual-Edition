#include "draw.h"
#include "board.h"

extern "C" {
#include <SDL2/SDL_image.h>
}

Interface::Interface()
{
    if(!(this->window = SDL_CreateWindow("Connect 4 Dual Edition", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN)))
    {
        DEBUG("window could not be created: %s\n", SDL_GetError());
        return;
    }

    // if(!(this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)))
    if(!(this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_SOFTWARE)))
    {
        DEBUG("renderer could not be created: %s\n", SDL_GetError());
        return;
    }

    this->create_compound_images();

    #define LOAD_IMG_TO_IMAGES(name, path) do { \
        SDL_Surface* surface = IMG_Load(path); \
        this->images[name] = {surface->w, surface->h, SDL_CreateTextureFromSurface(this->renderer, surface)}; \
        SDL_FreeSurface(surface); \
    } while(false)

    LOAD_IMG_TO_IMAGES("yellow_piece", "romfs:/images/yellow_piece.png");
    LOAD_IMG_TO_IMAGES("red_piece", "romfs:/images/red_piece.png");
    LOAD_IMG_TO_IMAGES("background", "romfs:/images/background.png");
    LOAD_IMG_TO_IMAGES("thumb", "romfs:/images/thumb.png");
    LOAD_IMG_TO_IMAGES("index", "romfs:/images/index.png");
    LOAD_IMG_TO_IMAGES("fingers", "romfs:/images/fingers.png");
    LOAD_IMG_TO_IMAGES("hand", "romfs:/images/hand.png");
 
    #undef LOAD_IMG_TO_IMAGES

    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);

    this->ready = true;
}

Interface::~Interface()
{
    auto begin = this->images.begin();
    auto end = this->images.end();
    for(auto it = begin; it != end; ++it)
    {
        SDL_DestroyTexture(it->second.texture);
    }

    if(this->renderer)
        SDL_DestroyRenderer(this->renderer);
    if(this->window)
        SDL_DestroyWindow(this->window);
}

static void compound_image_draw_on(SDL_Renderer* dest_renderer, SDL_Texture* source_texture, int width, int height, const std::vector<Point>& positions, bool horizontal_flip)
{
    for(const auto& position : positions)
    {
        SDL_Rect rect = {position.x, position.y, width, height};
        SDL_RenderCopyEx(
            dest_renderer,
            source_texture,
            NULL,
            &rect,
            0.0,
            NULL,
            horizontal_flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
        );
    }
}

void Interface::create_compound_images()
{
    std::vector<Point> positions;
    positions.reserve(BOARD_WIDTH * BOARD_HEIGHT);  // never need that much space but save time by not needing to reallocate

    SDL_Surface* dest_surface;
    SDL_Renderer* dest_renderer;
    SDL_Surface* source_surface; // never accessed directly, only in the macros
    SDL_Texture* source_texture;
    int dest_surface_width, dest_surface_height;
    int width, height;

    #define INIT_COMPOUND_IMAGE(width, height) do { \
        dest_surface_width = width; \
        dest_surface_height = height; \
        dest_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888); \
        dest_renderer = SDL_CreateSoftwareRenderer(dest_surface); \
    } while(false)

    #define LOAD_IMG_TO_TEXTURE(path) do { \
        source_surface = IMG_Load(path); \
        width = source_surface->w; \
        height = source_surface->h; \
        source_texture = SDL_CreateTextureFromSurface(dest_renderer, source_surface); \
        SDL_FreeSurface(source_surface); \
        source_surface = nullptr;  \
    } while(false)

    #define DRAW_CURRENT_TEXTURE_TO_POS() compound_image_draw_on(dest_renderer, source_texture, width, height, positions, false)
    #define DRAW_CURRENT_TEXTURE_TO_POS_FLIPPED() compound_image_draw_on(dest_renderer, source_texture, width, height, positions, true)

    #define FREE_CURRENT_TEXTURE() do { \
        SDL_DestroyTexture(source_texture); \
        source_texture = nullptr; \
    } while(false)

    #define SAVE_COMPOUND_IMAGE(name) do { \
        SDL_DestroyRenderer(dest_renderer); \
        dest_renderer = nullptr; \
        this->images[name] = {dest_surface_width, dest_surface_height, SDL_CreateTextureFromSurface(this->renderer, dest_surface)}; \
        SDL_FreeSurface(dest_surface); \
        dest_surface = nullptr; \
    } while(false)

    {  // Grid to contain the pieces
        INIT_COMPOUND_IMAGE(BOARD_WIDTH*GRID_SQUARE_SIZE + GRID_BORDER_SIZE*2, BOARD_HEIGHT*GRID_SQUARE_SIZE + GRID_BORDER_SIZE);

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_outer_top.png");

            positions.clear();
            for(size_t i = 0; i < BOARD_WIDTH; i++)
            {
                positions.emplace_back(GRID_BORDER_SIZE + GRID_SQUARE_SIZE*i, 0);
            }
            DRAW_CURRENT_TEXTURE_TO_POS();

            FREE_CURRENT_TEXTURE();
        }

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_outer_side.png");

            positions.clear();
            for(size_t i = 0; i < BOARD_HEIGHT-1; i++) // -1 because bottom row is handled by the stand
            {
                positions.emplace_back(0, GRID_BORDER_SIZE + GRID_SQUARE_SIZE*i);
            }
            DRAW_CURRENT_TEXTURE_TO_POS();

            positions.clear();
            for(size_t i = 0; i < BOARD_HEIGHT-1; i++) // -1 because bottom row is handled by the stand
            {
                positions.emplace_back(dest_surface_width - GRID_BORDER_SIZE, GRID_BORDER_SIZE + GRID_SQUARE_SIZE*i);
            }
            DRAW_CURRENT_TEXTURE_TO_POS_FLIPPED();

            FREE_CURRENT_TEXTURE();
        }

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_outer_corner.png");

            positions.clear();
            positions.emplace_back(0, 0);
            DRAW_CURRENT_TEXTURE_TO_POS();

            positions.clear();
            positions.emplace_back(dest_surface_width - GRID_BORDER_SIZE, 0);
            DRAW_CURRENT_TEXTURE_TO_POS_FLIPPED();

            FREE_CURRENT_TEXTURE();
        }

        int square_y = dest_surface_height - GRID_SQUARE_SIZE;

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_bottom_square.png");

            positions.clear();
            for(size_t j = 0; j < BOARD_WIDTH; j++)
            {
                positions.emplace_back(GRID_BORDER_SIZE + GRID_SQUARE_SIZE*j, square_y);
            }
            DRAW_CURRENT_TEXTURE_TO_POS();

            FREE_CURRENT_TEXTURE();
        }

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_square.png");

            positions.clear();
            for(size_t i = 0; i < BOARD_HEIGHT-1; i++) // -1 because bottom row is handled above
            {
                square_y -= GRID_SQUARE_SIZE;
                for(size_t j = 0; j < BOARD_WIDTH; j++)
                {
                    positions.emplace_back(GRID_BORDER_SIZE + GRID_SQUARE_SIZE*j, square_y);
                }
            }
            DRAW_CURRENT_TEXTURE_TO_POS();

            FREE_CURRENT_TEXTURE();
        }

        SAVE_COMPOUND_IMAGE("grid");
    }

    {  // Moving part/pop out mode indicator
        INIT_COMPOUND_IMAGE(BOARD_WIDTH*GRID_SQUARE_SIZE, GRID_SQUARE_SIZE);

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_bottom_moving_part.png");

            positions.clear();
            for(size_t i = 0; i < BOARD_WIDTH-2; i++)
            {
                positions.emplace_back(GRID_SQUARE_SIZE + GRID_SQUARE_SIZE*i, 0);
            }
            DRAW_CURRENT_TEXTURE_TO_POS();

            FREE_CURRENT_TEXTURE();
        }

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_bottom_moving_part_end.png");

            positions.clear();
            positions.emplace_back(0, 0);
            DRAW_CURRENT_TEXTURE_TO_POS();

            positions.clear();
            positions.emplace_back((BOARD_WIDTH-1)*GRID_SQUARE_SIZE, 0);
            DRAW_CURRENT_TEXTURE_TO_POS_FLIPPED();

            FREE_CURRENT_TEXTURE();
        }

        SAVE_COMPOUND_IMAGE("moving_part");
    }

    {  // Stand holding the grid
        INIT_COMPOUND_IMAGE((BOARD_WIDTH+2)*GRID_SQUARE_SIZE, GRID_SQUARE_SIZE*2);

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_stand_body.png");

            positions.clear();
            for(size_t i = 1; i < BOARD_WIDTH-1; i++)
            {
                positions.emplace_back(GRID_SQUARE_SIZE + GRID_SQUARE_SIZE*i, 0);
            }
            DRAW_CURRENT_TEXTURE_TO_POS();

            FREE_CURRENT_TEXTURE();
        }

        {
            LOAD_IMG_TO_TEXTURE("romfs:/images/grid_stand_end.png");

            positions.clear();
            positions.emplace_back(0, 0);
            DRAW_CURRENT_TEXTURE_TO_POS();

            positions.clear();
            positions.emplace_back(BOARD_WIDTH*GRID_SQUARE_SIZE, 0);
            DRAW_CURRENT_TEXTURE_TO_POS_FLIPPED();

            FREE_CURRENT_TEXTURE();
        }

        SAVE_COMPOUND_IMAGE("stand");
    }

    #undef INIT_COMPOUND_IMAGE
    #undef LOAD_IMG_TO_TEXTURE
    #undef DRAW_CURRENT_TEXTURE_TO_POS
    #undef DRAW_CURRENT_TEXTURE_TO_POS_FLIPPED
    #undef FREE_CURRENT_TEXTURE
    #undef SAVE_COMPOUND_IMAGE
}

void Interface::frame_start()
{
    SDL_RenderClear(this->renderer);
}

void Interface::frame_end()
{
    SDL_RenderPresent(this->renderer);
}
