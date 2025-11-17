#include "file_io.h"

#include <stdio.h>
#include <string.h>

#define SAVE_HEADER "CHESS_SAVE_V1"

static char piece_type_to_char(PieceType type) {
    switch (type) {
        case PIECE_ROOK:
            return 'R';
        case PIECE_KNIGHT:
            return 'N';
        case PIECE_BISHOP:
            return 'B';
        case PIECE_QUEEN:
            return 'Q';
        default:
            return '?';
    }
}

static bool char_to_piece_type(char c, PieceType *outType) {
    if (!outType) {
        return false;
    }
    switch (c) {
        case 'R':
            *outType = PIECE_ROOK;
            return true;
        case 'N':
            *outType = PIECE_KNIGHT;
            return true;
        case 'B':
            *outType = PIECE_BISHOP;
            return true;
        case 'Q':
            *outType = PIECE_QUEEN;
            return true;
        default:
            return false;
    }
}

static void sanitize_message(const char *input, char *output, size_t outputSize) {
    if (!input || !output || outputSize == 0) {
        return;
    }
    size_t len = strnlen(input, outputSize - 1);
    for (size_t i = 0; i < len; ++i) {
        char c = input[i];
        if (c == '\n' || c == '\r') {
            c = ' ';
        }
        output[i] = c;
    }
    output[len] = '\0';
}

bool save_game_to_path(const GameState *state, const ChatLog *log, const char *path) {
    if (!state || !path) {
        return false;
    }

    FILE *fp = fopen(path, "w");
    if (!fp) {
        return false;
    }

    fprintf(fp, "%s\n", SAVE_HEADER);
    fprintf(fp, "MODE %d\n", (int)state->mode);
    fprintf(fp, "CURRENT_PLAYER %d\n", (int)state->currentPlayer);
    fprintf(fp, "SCORES %d %d\n", state->score[PLAYER_WHITE], state->score[PLAYER_BLACK]);
    fprintf(fp, "PAUSED %d\n", state->isPaused ? 1 : 0);
    fprintf(fp, "GAME_OVER %d\n", state->isGameOver ? 1 : 0);
    fprintf(fp, "BOARD\n");

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            const Square *sq = &state->board[row][col];
            if (!sq->occupied) {
                fputs("__", fp);
            } else {
                char typeChar = piece_type_to_char(sq->type);
                char ownerChar = (sq->owner == PLAYER_WHITE) ? 'W' : 'B';
                fputc(typeChar, fp);
                fputc(ownerChar, fp);
            }
            if (col < BOARD_SIZE - 1) {
                fputc(' ', fp);
            }
        }
        fputc('\n', fp);
    }

    size_t chatCount = log ? log->count : 0;
    fprintf(fp, "CHAT %zu\n", chatCount);
    if (log) {
        for (size_t i = 0; i < chatCount; ++i) {
            const ChatEntry *entry = &log->entries[i];
            char sanitized[CHAT_MESSAGE_LENGTH];
            sanitize_message(entry->message, sanitized, sizeof(sanitized));
            fprintf(fp, "ENTRY %d\n", (int)entry->speaker);
            fprintf(fp, "TEXT %s\n", sanitized);
        }
    }

    fclose(fp);
    return true;
}

bool load_game_from_path(GameState *state, ChatLog *log, const char *path) {
    if (!state || !path) {
        return false;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return false;
    }

    char line[256];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return false;
    }
    line[strcspn(line, "\r\n")] = '\0';
    if (strcmp(line, SAVE_HEADER) != 0) {
        fclose(fp);
        return false;
    }

    int modeValue = 0;
    int currentPlayerValue = 0;
    int scoreWhite = 0;
    int scoreBlack = 0;
    int paused = 0;
    int gameOver = 0;

    if (!fgets(line, sizeof(line), fp) || sscanf(line, "MODE %d", &modeValue) != 1) {
        fclose(fp);
        return false;
    }
    if (!fgets(line, sizeof(line), fp) || sscanf(line, "CURRENT_PLAYER %d", &currentPlayerValue) != 1) {
        fclose(fp);
        return false;
    }
    if (!fgets(line, sizeof(line), fp) || sscanf(line, "SCORES %d %d", &scoreWhite, &scoreBlack) != 2) {
        fclose(fp);
        return false;
    }
    if (!fgets(line, sizeof(line), fp) || sscanf(line, "PAUSED %d", &paused) != 1) {
        fclose(fp);
        return false;
    }
    if (!fgets(line, sizeof(line), fp) || sscanf(line, "GAME_OVER %d", &gameOver) != 1) {
        fclose(fp);
        return false;
    }

    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return false;
    }
    line[strcspn(line, "\r\n")] = '\0';
    if (strcmp(line, "BOARD") != 0) {
        fclose(fp);
        return false;
    }

    GameState temp;
    memset(&temp, 0, sizeof(temp));
    temp.mode = (GameMode)modeValue;
    temp.currentPlayer = (currentPlayerValue == 0) ? PLAYER_WHITE : PLAYER_BLACK;
    temp.score[PLAYER_WHITE] = scoreWhite;
    temp.score[PLAYER_BLACK] = scoreBlack;
    temp.isPaused = paused != 0;
    temp.isGameOver = gameOver != 0;

    for (int row = 0; row < BOARD_SIZE; ++row) {
        if (!fgets(line, sizeof(line), fp)) {
            fclose(fp);
            return false;
        }
        char *token = strtok(line, " \t\r\n");
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (!token) {
                fclose(fp);
                return false;
            }
            Square *sq = &temp.board[row][col];
            if (strcmp(token, "__") == 0) {
                sq->occupied = false;
                sq->type = PIECE_ROOK;
                sq->owner = PLAYER_WHITE;
            } else {
                size_t tokenLen = strnlen(token, 3);
                if (tokenLen != 2) {
                    fclose(fp);
                    return false;
                }
                PieceType type;
                if (!char_to_piece_type(token[0], &type)) {
                    fclose(fp);
                    return false;
                }
                Player owner;
                if (token[1] == 'W') {
                    owner = PLAYER_WHITE;
                } else if (token[1] == 'B') {
                    owner = PLAYER_BLACK;
                } else {
                    fclose(fp);
                    return false;
                }
                sq->occupied = true;
                sq->type = type;
                sq->owner = owner;
            }
            token = strtok(NULL, " \t\r\n");
        }
    }

    size_t chatCount = 0;
    if (!fgets(line, sizeof(line), fp) || sscanf(line, "CHAT %zu", &chatCount) != 1) {
        fclose(fp);
        return false;
    }

    if (log) {
        chat_init(log);
    }

    for (size_t i = 0; i < chatCount; ++i) {
        int speakerValue = 0;
        if (!fgets(line, sizeof(line), fp) || sscanf(line, "ENTRY %d", &speakerValue) != 1) {
            fclose(fp);
            return false;
        }
        if (!fgets(line, sizeof(line), fp)) {
            fclose(fp);
            return false;
        }
        if (strncmp(line, "TEXT ", 5) != 0) {
            fclose(fp);
            return false;
        }
        char *text = line + 5;
        text[strcspn(text, "\r\n")] = '\0';
        if (log && i < CHAT_MAX_MESSAGES) {
            ChatSpeaker speaker = CHAT_SPEAKER_SYSTEM;
            if (speakerValue == (int)CHAT_SPEAKER_WHITE) {
                speaker = CHAT_SPEAKER_WHITE;
            } else if (speakerValue == (int)CHAT_SPEAKER_BLACK) {
                speaker = CHAT_SPEAKER_BLACK;
            }
            chat_add(log, speaker, text);
        }
    }

    *state = temp;

    fclose(fp);
    return true;
}
