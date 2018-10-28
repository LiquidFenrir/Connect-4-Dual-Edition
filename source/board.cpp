#include "board.h"

GameBoard game_board;

GameBoard::GameBoard()
{
    for(auto& column : this->board)
    {
        std::fill(column.begin(), column.end(), GRID_EMPTY);
    }
}

bool GameBoard::is_column_full(int column_id)
{
    return this->board[column_id][BOARD_HEIGHT] != GRID_EMPTY;
}

bool GameBoard::column_has_bottom_filled(int column_id)
{
    return this->board[column_id][BORDER_SIZE] != GRID_EMPTY;
}

void GameBoard::pop_column_bottom(int column_id)
{
    auto& column = this->board[column_id];
    auto begin = column.rbegin() + BORDER_SIZE;
    auto end = column.rend() - BORDER_SIZE;
    std::rotate(begin, begin+1, end);
    column[BOARD_HEIGHT] = GRID_EMPTY;
}

int GameBoard::add_column_top(int column_id, GridSquareColor color)
{
    int pos = BOARD_HEIGHT;
    while(pos >= BORDER_SIZE && this->board[column_id][pos] == GRID_EMPTY)
    {
        pos--;
    }
    pos++;
    this->board[column_id][pos] = color;
    return pos;
}

std::pair<GridSquareColor, int> GameBoard::check_board_around(int x, int y)
{
    GridSquareColor color = this->board[x][y];

    std::array<bool, GRID_DIRECTIONS_AMOUNT> keep_going;  // Set to false when we hit a GRID_EMPTY square (why the border is there)
    std::fill(keep_going.begin(), keep_going.end(), true);

    static const std::array<std::pair<int, int>, GRID_DIRECTIONS_AMOUNT> directions_change{{  // dx, dy
        {0, 1},  // GRID_DIRECTION_UP
        {1, 1},  // GRID_DIRECTION_RIGHT_UP
        {1, 0},  // GRID_DIRECTION_RIGHT
        {1, -1},  // GRID_DIRECTION_RIGHT_DOWN
        {0, -1},  // GRID_DIRECTION_DOWN
        {-1, -1},  // GRID_DIRECTION_LEFT_DOWN
        {-1, 0},  // GRID_DIRECTION_LEFT
        {-1, 1}  // GRID_DIRECTION_LEFT_UP
    }};

    std::array<int, GRID_DIRECTIONS_AMOUNT> connection_length;
    std::fill(connection_length.begin(), connection_length.end(), 1);  // Starts at 1: the 

    int change_value = 1;
    while(std::any_of(keep_going.cbegin(), keep_going.cend(), [](bool val){ return val; }))
    {
        for(int direction = GRID_DIRECTION_UP; direction < GRID_DIRECTIONS_AMOUNT; direction++)
        {
            if(keep_going[direction])
            {
                int new_x = x + directions_change[direction].first * change_value;
                int new_y = y + directions_change[direction].second * change_value;

                if(this->board[new_x][new_y] != color)
                {
                    keep_going[direction] = false;
                }
                else
                {
                    connection_length[direction]++;
                }
            }
        }

        change_value++;
    }

    int longest = *std::max_element(connection_length.cbegin(), connection_length.cend());
    return std::make_pair(color, longest);
}

GridSquareColor GameBoard::check_winner(int x, int y)
{
    std::pair<GridSquareColor, int> color_and_longest = this->check_board_around(x, y);
    return color_and_longest.second >= 4 ? color_and_longest.first : GRID_EMPTY;
}

void GameBoard::empty()
{
    auto begin = this->board.begin() + BORDER_SIZE;
    auto end = this->board.end() - BORDER_SIZE;
    for(auto& column = begin; column != end; ++column)
    {
        std::fill(column->begin() + BORDER_SIZE, column->end() - BORDER_SIZE, GRID_EMPTY);
    }
}
