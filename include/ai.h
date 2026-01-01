#ifndef AI_H
#define AI_H

#include <stdbool.h>
#include "game_logic.h"

void ai_init(void);
bool ai_pick_move(const GameState *state, Move *outMove);

#endif // AI_H
