#pragma once

#include "common.h"

enum GridSquareColor {
    GRID_EMPTY,

    GRID_RED,
    GRID_YELLOW,
};

enum GridConnectionDirection {
    GRID_DIRECTION_UP = 0,
    GRID_DIRECTION_RIGHT_UP,
    GRID_DIRECTION_RIGHT,
    GRID_DIRECTION_RIGHT_DOWN,
    GRID_DIRECTION_DOWN,
    GRID_DIRECTION_LEFT_DOWN,
    GRID_DIRECTION_LEFT,
    GRID_DIRECTION_LEFT_UP,
    
    GRID_DIRECTIONS_AMOUNT
};

constexpr int BOARD_WIDTH = 7;
constexpr int BOARD_HEIGHT = 6;
constexpr int BORDER_SIZE = 1;
constexpr int ACTUAL_WIDTH = BOARD_WIDTH + 2*BORDER_SIZE;
constexpr int ACTUAL_HEIGHT = BOARD_HEIGHT + 2*BORDER_SIZE;

class GameBoard {
    public:
        GameBoard();

        std::array<std::array<GridSquareColor, ACTUAL_HEIGHT>, ACTUAL_WIDTH> board;

        bool is_column_full(int column_id);
        bool column_has_bottom_filled(int column_id);

        void pop_column_bottom(int column_id);
        int add_column_top(int column_id, GridSquareColor color);

        GridSquareColor check_winner(int x, int y);

        void empty();
    
    private:
        std::pair<GridSquareColor, int> check_board_around(int x, int y);
};

extern GameBoard game_board;
