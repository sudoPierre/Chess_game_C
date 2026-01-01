#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include <stddef.h>

#define BOARD_SIZE 8
#define MAX_MOVES_PER_TURN 64

typedef enum {
    PLAYER_WHITE = 0,
    PLAYER_BLACK = 1,
    PLAYER_COUNT
} Player;

typedef enum {
    PIECE_ROOK = 0,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_QUEEN,
    PIECE_TYPE_COUNT
} PieceType;

typedef enum {
    MODE_NONE = 0,
    MODE_PVE,
    MODE_PVP
} GameMode;

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    Position from;
    Position to;
} Move;

typedef struct {
    bool occupied;
    PieceType type;
    Player owner;
} Square;

typedef struct {
    Square board[BOARD_SIZE][BOARD_SIZE];
    Player currentPlayer;
    int score[PLAYER_COUNT];
    GameMode mode;
    bool isPaused;
    bool isGameOver;
} GameState;

void game_init(GameState *state, GameMode mode);
void game_reset(GameState *state, GameMode mode);
Player game_get_opponent(Player player);
bool game_is_valid_position(Position pos);
bool game_is_valid_move(const GameState *state, Position from, Position to);
bool game_apply_move(GameState *state, Position from, Position to);
size_t game_list_moves(const GameState *state, Player player, Move *moves, size_t maxMoves);
bool game_has_any_moves(const GameState *state, Player player);

#endif // GAME_LOGIC_H
