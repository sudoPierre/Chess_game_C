#include "game_logic.h"

#include <string.h>

typedef struct {
    PieceType type;
    Position pos;
} StartPiece;

static const StartPiece START_PIECES[PLAYER_COUNT][PIECE_TYPE_COUNT] = {
    {
        { PIECE_ROOK,   { 7, 1 } },
        { PIECE_KNIGHT, { 7, 3 } },
        { PIECE_BISHOP, { 7, 5 } },
        { PIECE_QUEEN,  { 7, 7 } }
    },
    {
        { PIECE_ROOK,   { 0, 1 } },
        { PIECE_KNIGHT, { 0, 3 } },
        { PIECE_BISHOP, { 0, 5 } },
        { PIECE_QUEEN,  { 0, 7 } }
    }
};

static inline bool is_inside(Position pos) {
    return pos.row >= 0 && pos.row < BOARD_SIZE && pos.col >= 0 && pos.col < BOARD_SIZE;
}

static inline int forward_direction(Player player) {
    return (player == PLAYER_WHITE) ? -1 : 1;
}

Player game_get_opponent(Player player) {
    return (player == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;
}

bool game_is_valid_position(Position pos) {
    return is_inside(pos);
}

static void clear_board(GameState *state) {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            state->board[row][col].occupied = false;
            state->board[row][col].type = PIECE_ROOK;
            state->board[row][col].owner = PLAYER_WHITE;
        }
    }
}

static void place_initial_pieces(GameState *state) {
    for (int player = 0; player < PLAYER_COUNT; ++player) {
        for (int idx = 0; idx < PIECE_TYPE_COUNT; ++idx) {
            StartPiece piece = START_PIECES[player][idx];
            if (!game_is_valid_position(piece.pos)) {
                continue;
            }
            Square *sq = &state->board[piece.pos.row][piece.pos.col];
            sq->occupied = true;
            sq->type = piece.type;
            sq->owner = (Player)player;
        }
    }
}

void game_init(GameState *state, GameMode mode) {
    game_reset(state, mode);
}

void game_reset(GameState *state, GameMode mode) {
    if (!state) {
        return;
    }

    clear_board(state);

    state->mode = mode;
    state->currentPlayer = PLAYER_WHITE;
    state->score[PLAYER_WHITE] = 0;
    state->score[PLAYER_BLACK] = 0;
    state->isPaused = false;
    state->isGameOver = false;

    place_initial_pieces(state);
}

static bool square_contains_player_piece(const GameState *state, Position pos, Player player) {
    if (!is_inside(pos)) {
        return false;
    }
    const Square *sq = &state->board[pos.row][pos.col];
    return sq->occupied && sq->owner == player;
}

static bool is_valid_destination(const Square *fromSquare, const Square *toSquare, int colDiff) {
    if (!toSquare->occupied) {
        return true;
    }
    if (toSquare->owner == fromSquare->owner) {
        return false;
    }
    return (colDiff != 0);
}

bool game_is_valid_move(const GameState *state, Position from, Position to) {
    if (!state || !is_inside(from) || !is_inside(to)) {
        return false;
    }
    if (from.row == to.row && from.col == to.col) {
        return false;
    }

    const Square *fromSquare = &state->board[from.row][from.col];
    if (!fromSquare->occupied) {
        return false;
    }

    int dir = forward_direction(fromSquare->owner);
    if (to.row != from.row + dir) {
        return false;
    }

    int colDiff = to.col - from.col;
    if (colDiff < -1 || colDiff > 1) {
        return false;
    }

    const Square *toSquare = &state->board[to.row][to.col];
    if (colDiff == 0) {
        return !toSquare->occupied;
    }

    return is_valid_destination(fromSquare, toSquare, colDiff);
}

static bool apply_move_internal(GameState *state, Position from, Position to, bool dryRun) {
    if (!game_is_valid_move(state, from, to)) {
        return false;
    }

    Square *fromSquare = &state->board[from.row][from.col];
    Square *toSquare = &state->board[to.row][to.col];

    Player mover = fromSquare->owner;

    if (!dryRun && toSquare->occupied && toSquare->owner != mover) {
        state->score[mover] += 1;
    }

    if (!dryRun) {
        *toSquare = *fromSquare;
        fromSquare->occupied = false;
    }

    return true;
}

bool game_apply_move(GameState *state, Position from, Position to) {
    if (!state || state->isGameOver || state->isPaused) {
        return false;
    }

    if (!is_inside(from) || !is_inside(to)) {
        return false;
    }

    Square *fromSquare = &state->board[from.row][from.col];
    if (!fromSquare->occupied || fromSquare->owner != state->currentPlayer) {
        return false;
    }

    if (!apply_move_internal(state, from, to, false)) {
        return false;
    }

    state->currentPlayer = game_get_opponent(state->currentPlayer);

    if (!game_has_any_moves(state, state->currentPlayer)) {
        state->isGameOver = true;
    }

    return true;
}

size_t game_list_moves(const GameState *state, Player player, Move *moves, size_t maxMoves) {
    if (!state) {
        return 0;
    }

    size_t count = 0;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            Position from = { row, col };
            if (!square_contains_player_piece(state, from, player)) {
                continue;
            }

            int dir = forward_direction(player);
            for (int deltaCol = -1; deltaCol <= 1; ++deltaCol) {
                Position to = { row + dir, col + deltaCol };
                if (!is_inside(to)) {
                    continue;
                }
                GameState temp = *state;
                if (apply_move_internal(&temp, from, to, true)) {
                    if (moves && count < maxMoves) {
                        moves[count].from = from;
                        moves[count].to = to;
                    }
                    ++count;
                    if (!moves && count > 0) {
                        return count;
                    }
                    if (moves && count >= maxMoves) {
                        return count;
                    }
                }
            }
        }
    }
    return count;
}

bool game_has_any_moves(const GameState *state, Player player) {
    Move moves[1];
    return game_list_moves(state, player, moves, 1) > 0;
}
