#pragma once

#include "common.h"
#include "board.h"
#include "draw.h"
#include "input.h"

enum GameState {
    GAME_STATE_CHOOSING_MODE,
    GAME_STATE_PLAYING,
};

enum GameAction {
    ACTION_A,
    ACTION_B,
    ACTION_C,

    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_UP,
    ACTION_DOWN,
};

using ActionToKeyMap = std::map<GameAction, JoystickKeys>;

class Game {
    public:
        Game(int argc, char** argv);
        ~Game();

        bool running = true;

        void update();

        static ActionToKeyMap red_keys, yellow_keys;

    private:
        GameState game_state;
        GridSquareColor last_winner, winner;
        bool tie;
        GridSquareColor player;
        Interface* interface = nullptr;
        Input* input = nullptr;

        int selected_column;

        bool pop_out_mode = false;
        bool choosing_pop_out;

        void draw();

        bool get_action(GameAction action);
        void start_game();
        void next_player_turn();
};
