#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>
#include "game_logic.h"
#include "chat.h"

#define SAVE_FILE_PATH "saves/save_slot1.dat"

bool save_game_to_path(const GameState *state, const ChatLog *log, const char *path);
bool load_game_from_path(GameState *state, ChatLog *log, const char *path);

#endif // FILE_IO_H
