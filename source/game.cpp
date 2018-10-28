#include "game.h"

ActionToKeyMap Game::red_keys = {
    {ACTION_A, JKEY_LSTICK},
    {ACTION_B, JKEY_L},
    {ACTION_C, JKEY_ZL},

    {ACTION_LEFT, JKEY_DUP},
    {ACTION_RIGHT, JKEY_DDOWN},
    {ACTION_UP, JKEY_DRIGHT},
    {ACTION_DOWN, JKEY_DLEFT},
};

ActionToKeyMap Game::yellow_keys = {
    {ACTION_A, JKEY_RSTICK},
    {ACTION_B, JKEY_R},
    {ACTION_C, JKEY_ZR},

    {ACTION_LEFT, JKEY_B},
    {ACTION_RIGHT, JKEY_X},
    {ACTION_UP, JKEY_Y},
    {ACTION_DOWN, JKEY_A},
};

Game::Game(int argc, char** argv)
{
    romfsInit();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) < 0)
    {
        DEBUG("SDL init failed: %s\n", SDL_GetError());
        return;
    }
    DEBUG("initialized SDL\n");

    this->interface = new Interface;
    DEBUG("Got interface\n");
    this->input = new Input;
    DEBUG("Got input\n");

    this->player = GRID_YELLOW;  // Gets reversed in the next_player_turn inside start_game
    this->winner = GRID_YELLOW;  // If the first game results in a tie, make the red start again
    this->start_game();
}

Game::~Game()
{
    DEBUG("Deleting input...\n");
    delete this->input;
    DEBUG("Deleting interface...\n");
    delete this->interface;

    DEBUG("Quitting SDL...\n");
    SDL_Quit();
    DEBUG("Quit SDL.\n");

    romfsExit();
}

bool Game::get_action(GameAction action)
{
    auto& actions_to_keys_map = this->player == GRID_RED ? this->red_keys : this->yellow_keys;
    return this->input->keys[actions_to_keys_map[action]];
}

void Game::start_game()
{
    DEBUG("Starting new game\n");
    this->tie = false;
    this->last_winner = this->winner;
    this->winner = GRID_EMPTY;
    this->game_state = GAME_STATE_CHOOSING_MODE;
    game_board.empty();
    this->next_player_turn();
}

void Game::next_player_turn()
{
    this->player = this->player == GRID_RED ? GRID_YELLOW : GRID_RED;
    this->selected_column = ceil(BOARD_WIDTH / 2.0);
    this->choosing_pop_out = false;
}

void Game::draw()
{
    this->interface->frame_start();

    SDL_RenderCopy(this->interface->renderer, this->interface->images["background"].texture, NULL, NULL);

    auto& grid = this->interface->images["grid"];
    static const int grid_base_x = (SCREEN_WIDTH - grid.width)/2;  // Won't ever change

    auto& red_piece = this->interface->images["red_piece"];
    auto& yellow_piece = this->interface->images["yellow_piece"];

    int square_x = grid_base_x + GRID_BORDER_SIZE;
    for(int x = BORDER_SIZE; x <= BOARD_WIDTH; x++)
    {
        int square_y = SCREEN_HEIGHT - GRID_SQUARE_SIZE*2;
        for(int y = BORDER_SIZE; y <= BOARD_HEIGHT; y++)
        {
            GridSquareColor square = game_board.board[x][y];
            if(square == GRID_EMPTY)
            {
                break;
            }
            else
            {
                this->interface->draw_image_at(square == GRID_RED ? red_piece : yellow_piece, square_x, square_y);
            }
            square_y -= GRID_SQUARE_SIZE;
        }
        square_x += GRID_SQUARE_SIZE;
    }

    static const SDL_Rect grid_dest_rect = {grid_base_x, SCREEN_HEIGHT - grid.height - GRID_SQUARE_SIZE, grid.width, grid.height}; // Won't ever change
    this->interface->draw_image_at(grid, &grid_dest_rect);

    int moving_part_x = (this->pop_out_mode ? GRID_SQUARE_SIZE/2 : 0);
    this->interface->draw_image_at("moving_part", grid_base_x + GRID_BORDER_SIZE + moving_part_x, SCREEN_HEIGHT - GRID_SQUARE_SIZE*2);

    this->interface->draw_image_at("stand", grid_base_x + GRID_BORDER_SIZE - GRID_SQUARE_SIZE, SCREEN_HEIGHT - GRID_SQUARE_SIZE*2);

    if(this->game_state == GAME_STATE_PLAYING && !(this->tie || this->winner != GRID_EMPTY))
    {
        auto& hand = this->interface->images["hand"];
        static constexpr int hand_offset_from_left_border = 64;
        SDL_Rect position_indicator_dest_rect = {grid_base_x + GRID_BORDER_SIZE + (this->selected_column - BORDER_SIZE)*GRID_SQUARE_SIZE - hand_offset_from_left_border, 0, hand.width, hand.height};
        if(this->choosing_pop_out)
        {
            position_indicator_dest_rect.x += GRID_SQUARE_SIZE/4;
            position_indicator_dest_rect.y = SCREEN_HEIGHT - GRID_SQUARE_SIZE*2 + GRID_SQUARE_SIZE/4;
            this->interface->draw_image_at("index", &position_indicator_dest_rect);
            this->interface->draw_image_at(hand, &position_indicator_dest_rect);
        }
        else
        {
            this->interface->draw_image_at("fingers", &position_indicator_dest_rect);
            this->interface->draw_image_at(hand, &position_indicator_dest_rect);
            this->interface->draw_image_at(this->player == GRID_RED ? red_piece : yellow_piece, position_indicator_dest_rect.x + hand_offset_from_left_border, position_indicator_dest_rect.y + GRID_SQUARE_SIZE*2 + GRID_SQUARE_SIZE/2);
            this->interface->draw_image_at("thumb", &position_indicator_dest_rect);
        }
    }

    this->interface->frame_end();
}

void Game::update()
{
    if(this->interface && this->interface->ready)
        this->draw();
    this->input->get();

    if(this->input->keys[JKEY_PLUS])
    {
        this->running = false;
    }
    else if(this->tie || this->winner != GRID_EMPTY)  // Dont need a state for game done if you always check if someone won
    {
        if(this->input->keys[JKEY_MINUS])
        {
            if(this->tie)
                this->winner = this->last_winner;
            this->player = this->winner;  // Gets reversed in the next_player_turn inside start_game
            this->start_game();
        }
    }
    else if(this->get_action(ACTION_A))  // Stick press
    {
        if(this->game_state == GAME_STATE_CHOOSING_MODE)
        {
            this->game_state = GAME_STATE_PLAYING;
        }
        else if(this->game_state == GAME_STATE_PLAYING)
        {
            if(this->choosing_pop_out)
            {
                if(game_board.column_has_bottom_filled(this->selected_column))
                {
                    game_board.pop_column_bottom(this->selected_column);

                    std::vector<GridSquareColor> winners(BOARD_HEIGHT, GRID_EMPTY);

                    for(int i = BORDER_SIZE; i <= BOARD_HEIGHT; i++)
                    {
                        if(game_board.board[this->selected_column][i] == GRID_EMPTY)  // Stop checking once you reach an empty square
                        {
                            break;
                        }
                        else 
                        {
                            winners.push_back(game_board.check_winner(this->selected_column, i));
                        }
                    }

                    bool red_won = std::count(winners.cbegin(), winners.cend(), GRID_RED) >= 1;
                    bool yellow_won = std::count(winners.cbegin(), winners.cend(), GRID_YELLOW) >= 1;
                    if(red_won && yellow_won)
                    {
                        this->tie = true;
                    }
                    else if(red_won)
                    {
                        this->winner = GRID_RED;
                    }
                    else if(yellow_won)
                    {
                        this->winner = GRID_YELLOW;
                    }
                    else
                    {
                        this->next_player_turn();
                    }
                }
            }
            else
            {
                if(!game_board.is_column_full(this->selected_column))
                {
                    int y = game_board.add_column_top(this->selected_column, this->player);
                    if((this->winner = game_board.check_winner(this->selected_column, y)) == GRID_EMPTY)
                    {
                        if(!this->pop_out_mode)
                        {
                            bool all_full = true;
                            for(int column = BORDER_SIZE; column <= BOARD_WIDTH; column++)
                            {
                                if(!game_board.is_column_full(column))
                                {
                                    all_full = false;
                                    break;
                                }
                            }
                            
                            if(all_full)
                            {
                                this->tie = true;
                                return;  // Skip the next_player_turn
                            }
                        }
                        this->next_player_turn();
                    }
                }
            }
        }
    }
    else if(this->get_action(ACTION_B))  // Shoulder press
    {
        if(this->game_state == GAME_STATE_CHOOSING_MODE)
        {
            this->pop_out_mode = !this->pop_out_mode;
        }
    }
    else if(this->get_action(ACTION_LEFT))
    {
        if(this->game_state == GAME_STATE_PLAYING)
        {
            this->selected_column--;
            if(this->selected_column < BORDER_SIZE)
                this->selected_column = BOARD_WIDTH;
        }
    }
    else if(this->get_action(ACTION_RIGHT))
    {
        if(this->game_state == GAME_STATE_PLAYING)
        {
            this->selected_column++;
            if(this->selected_column > BORDER_SIZE)
                this->selected_column = BORDER_SIZE;
        }
    }
    else if(this->get_action(ACTION_UP))
    {
        if(this->game_state == GAME_STATE_PLAYING)
        {
            if(this->pop_out_mode)
            {
                this->choosing_pop_out = false;
            }
        }
    }
    else if(this->get_action(ACTION_DOWN))
    {
        if(this->game_state == GAME_STATE_PLAYING)
        {
            if(this->pop_out_mode)
            {
                this->choosing_pop_out = true;
            }
        }
    }
}
