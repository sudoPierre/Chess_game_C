#include <stdio.h>
#include <string.h>
#include <SDL.h>

#include "game_logic.h"
#include "ui.h"
#include "ai.h"
#include "chat.h"
#include "file_io.h"

static void start_new_game(GameState *game, ChatLog *chat, UiState *ui, GameMode mode, const char *statusMessage) {
    game_reset(game, mode);
    chat_init(chat);
    ui_reset_game_interaction(ui);
    ui_set_view(ui, UI_VIEW_GAME);
    ui_set_status_message(ui, statusMessage);
    game->isPaused = false;
    ui->chatSpeaker = CHAT_SPEAKER_WHITE;
    ui->chatInputLength = 0;
    ui->chatInput[0] = '\0';
    chat_add(chat, CHAT_SPEAKER_SYSTEM, statusMessage);
    if (mode == MODE_PVE) {
        chat_add(chat, CHAT_SPEAKER_SYSTEM, "You play as WHITE. Computer is BLACK.");
    } else if (mode == MODE_PVP) {
        chat_add(chat, CHAT_SPEAKER_SYSTEM, "Local two-player mode. WHITE moves first.");
    }
}

static void process_command(const UiCommand *command, UiState *ui, GameState *game, ChatLog *chat) {
    if (!command || !ui || !game || !chat) {
        return;
    }

    switch (command->type) {
        case UI_CMD_NONE:
            break;
        case UI_CMD_QUIT:
            ui->running = false;
            break;
        case UI_CMD_START_PVE:
            start_new_game(game, chat, ui, MODE_PVE, "New game vs Computer started.");
            break;
        case UI_CMD_START_PVP:
            start_new_game(game, chat, ui, MODE_PVP, "New two-player game started.");
            break;
        case UI_CMD_MAIN_MENU:
            ui_set_view(ui, UI_VIEW_MAIN_MENU);
            ui_reset_game_interaction(ui);
            game_reset(game, MODE_NONE);
            ui_set_status_message(ui, "Returned to main menu.");
            break;
        case UI_CMD_PAUSE:
            game->isPaused = true;
            ui_set_view(ui, UI_VIEW_PAUSE);
            ui_set_status_message(ui, "Game paused.");
            break;
        case UI_CMD_RESUME:
            game->isPaused = false;
            ui_set_view(ui, UI_VIEW_GAME);
            ui_set_status_message(ui, "Game resumed.");
            break;
        case UI_CMD_SAVE:
            if (save_game_to_path(game, chat, SAVE_FILE_PATH)) {
                ui_set_status_message(ui, "Game saved.");
            } else {
                ui_set_status_message(ui, "Failed to save game.");
            }
            break;
        case UI_CMD_LOAD:
            if (load_game_from_path(game, chat, SAVE_FILE_PATH)) {
                ui_reset_game_interaction(ui);
                ui_set_view(ui, UI_VIEW_GAME);
                ui_set_status_message(ui, "Game loaded.");
            } else {
                ui_set_status_message(ui, "Failed to load game.");
            }
            break;
        case UI_CMD_PLAYER_MOVE: {
            if (game_apply_move(game, command->move.from, command->move.to)) {
                ui_reset_game_interaction(ui);
                if (game->isGameOver) {
                    const char *winner = (game->score[PLAYER_WHITE] == game->score[PLAYER_BLACK]) ? "No one" : (game->score[PLAYER_WHITE] > game->score[PLAYER_BLACK] ? "WHITE" : "BLACK");
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Game over. %s wins!", winner);
                    ui_set_status_message(ui, msg);
                    chat_add(chat, CHAT_SPEAKER_SYSTEM, msg);
                } else {
                    ui_set_status_message(ui, "Move applied.");
                }
            } else {
                ui_set_status_message(ui, "Invalid move.");
            }
            break;
        }
        case UI_CMD_CHAT_MESSAGE: {
            ChatSpeaker speaker = ui_current_chat_speaker(ui);
            chat_add(chat, speaker, command->chatMessage);
            ui_set_status_message(ui, "Message sent.");
            break;
        }
    }
}

static void maybe_run_ai(GameState *game, ChatLog *chat, UiState *ui) {
    if (!game || !chat || !ui) {
        return;
    }
    if (game->mode != MODE_PVE) {
        return;
    }
    if (game->isPaused || game->isGameOver) {
        return;
    }
    if (game->currentPlayer != PLAYER_BLACK) {
        return;
    }

    Move aiMove;
    if (!ai_pick_move(game, &aiMove)) {
        game->isGameOver = true;
        ui_set_status_message(ui, "Computer cannot move. You win!");
        chat_add(chat, CHAT_SPEAKER_SYSTEM, "Computer cannot move. You win!");
        return;
    }

    if (game_apply_move(game, aiMove.from, aiMove.to)) {
        chat_add(chat, CHAT_SPEAKER_SYSTEM, "Computer played a move.");
        if (game->isGameOver) {
            const char *winner = (game->score[PLAYER_WHITE] == game->score[PLAYER_BLACK]) ? "No one" : (game->score[PLAYER_WHITE] > game->score[PLAYER_BLACK] ? "WHITE" : "BLACK");
            char msg[64];
            snprintf(msg, sizeof(msg), "Game over. %s wins!", winner);
            ui_set_status_message(ui, msg);
            chat_add(chat, CHAT_SPEAKER_SYSTEM, msg);
        }
    }
}

int main(void) {
    GameState game;
    ChatLog chat;
    UiState ui;

    ai_init();
    chat_init(&chat);
    game_init(&game, MODE_NONE);

    if (!ui_init(&ui, "Simplified Chess")) {
        fprintf(stderr, "Failed to initialize UI: %s\n", SDL_GetError());
        return 1;
    }

    Uint32 lastTicks = SDL_GetTicks();

    while (ui.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            UiCommand command;
            if (ui_handle_event(&ui, &event, &game, &command) && command.type != UI_CMD_NONE) {
                process_command(&command, &ui, &game, &chat);
            }
        }

        maybe_run_ai(&game, &chat, &ui);

        Uint32 currentTicks = SDL_GetTicks();
        Uint32 delta = currentTicks - lastTicks;
        ui_update(&ui, delta);
        lastTicks = currentTicks;

        ui_render(&ui, &game, &chat);
        SDL_Delay(16);
    }

    ui_cleanup(&ui);
    return 0;
}
