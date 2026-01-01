#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define SAVE_DIR "saves"
#define SAVE_EXT ".sav"
#define SAVE_HEADER "CHESS_SAVE_V1"

/* =========================
   UTILS
   ========================= */

static char piece_to_char(PieceType t) {
    switch (t) {
    case PIECE_ROOK:   return 'R';
    case PIECE_KNIGHT: return 'N';
    case PIECE_BISHOP: return 'B';
    case PIECE_QUEEN:  return 'Q';
    default: return '?';
    }
}

static int char_to_piece(char c, PieceType* out) {
    if (!out) return 0;
    switch (c) {
    case 'R': *out = PIECE_ROOK;   return 1;
    case 'N': *out = PIECE_KNIGHT; return 1;
    case 'B': *out = PIECE_BISHOP; return 1;
    case 'Q': *out = PIECE_QUEEN;  return 1;
    default: return 0;
    }
}

/* =========================
   SAVE
   ========================= */

bool save_game_to_path(const GameState* state, const ChatLog* log, const char* path) {
    if (!state || !path) return false;

    FILE* f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "%s\n", SAVE_HEADER);
    fprintf(f, "%d %d %d %d %d %d\n",
        (int)state->mode,
        (int)state->currentPlayer,
        state->score[0],
        state->score[1],
        (int)state->isPaused,
        (int)state->isGameOver);

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            const Square* sq = &state->board[r][c];
            if (!sq->occupied) {
                fprintf(f, "__ ");
            }
            else {
                fprintf(f, "%c%c ",
                    piece_to_char(sq->type),
                    sq->owner == PLAYER_WHITE ? 'W' : 'B');
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return true;
}

/* =========================
   LIST SAVES
   ========================= */

int list_save_files(char files[][256], int max) {
    DIR* dir = opendir(SAVE_DIR);
    if (!dir) return 0;

    struct dirent* entry;
    int count = 0;

    while ((entry = readdir(dir)) && count < max) {
        const char* name = entry->d_name;
        size_t len = strlen(name);

        if (len > 4 && strcmp(name + len - 4, SAVE_EXT) == 0) {
            strncpy(files[count], name, 255);
            files[count][255] = '\0';
            count++;
        }
    }

    closedir(dir);
    return count;
}

/* =========================
   LOAD
   ========================= */

bool load_game_from_path(GameState* state, ChatLog* log, const char* path) {
    if (!state || !path) return false;

    FILE* f = fopen(path, "r");
    if (!f) return false;

    char header[64];
    if (!fgets(header, sizeof(header), f)) { fclose(f); return false; }
    header[strcspn(header, "\r\n")] = 0;

    if (strcmp(header, SAVE_HEADER) != 0) {
        fclose(f);
        return false;
    }

    int paused, gameOver;
    fscanf(f, "%d %d %d %d %d %d\n",
        (int*)&state->mode,
        (int*)&state->currentPlayer,
        &state->score[0],
        &state->score[1],
        &paused,
        &gameOver);

    state->isPaused = paused != 0;
    state->isGameOver = gameOver != 0;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            char tok[4];
            fscanf(f, "%3s", tok);

            Square* sq = &state->board[r][c];
            if (strcmp(tok, "__") == 0) {
                sq->occupied = false;
            }
            else {
                PieceType pt;
                if (!char_to_piece(tok[0], &pt)) {
                    fclose(f);
                    return false;
                }
                sq->occupied = true;
                sq->type = pt;
                sq->owner = (tok[1] == 'W') ? PLAYER_WHITE : PLAYER_BLACK;
            }
        }
    }

    fclose(f);
    return true;
}
