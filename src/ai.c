#include "ai.h"

#include <stdlib.h>
#include <time.h>

void ai_init(void) {
    srand((unsigned int)time(NULL));
}

bool ai_pick_move(const GameState *state, Move *outMove) {
    if (!state || !outMove) {
        return false;
    }

    Move moves[MAX_MOVES_PER_TURN];
    size_t count = game_list_moves(state, state->currentPlayer, moves, MAX_MOVES_PER_TURN);
    if (count == 0) {
        return false;
    }

    size_t index = (size_t)(rand() % count);
    *outMove = moves[index];
    return true;
}
