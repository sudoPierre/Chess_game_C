#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game_logic.h"
#include "ui.h"
#include "ai.h"
#include "chat.h"
#include "file_io.h"

// Structure pour stocker les couleurs personnalisées des pions
typedef struct {
    SDL_Color whiteColor;
    SDL_Color blackColor;
} PieceColors;

// Couleurs par défaut
static PieceColors pieceColors = {
    .whiteColor = {255, 255, 255, 255},  // Blanc
    .blackColor = {0, 0, 0, 255}          // Noir
};

// Couleurs prédéfinies disponibles
static const SDL_Color PRESET_COLORS[] = {
    {255, 255, 255, 255},  // Blanc
    {0, 0, 0, 255},        // Noir
    {255, 0, 0, 255},      // Rouge
    {0, 255, 0, 255},      // Vert
    {0, 0, 255, 255},      // Bleu
    {255, 255, 0, 255},    // Jaune
    {255, 0, 255, 255},    // Magenta
    {0, 255, 255, 255},    // Cyan
    {255, 165, 0, 255},    // Orange
    {128, 0, 128, 255},    // Violet
    {165, 42, 42, 255},    // Marron
    {192, 192, 192, 255}   // Gris
};

static const char* COLOR_NAMES[] = {
    "Blanc", "Noir", "Rouge", "Vert", "Bleu", "Jaune",
    "Magenta", "Cyan", "Orange", "Violet", "Marron", "Gris"
};

#define NUM_PRESET_COLORS (sizeof(PRESET_COLORS) / sizeof(PRESET_COLORS[0]))

static void start_new_game(GameState* game, ChatLog* chat, UiState* ui, GameMode mode, const char* statusMessage) {
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
    }
    else if (mode == MODE_PVP) {
        chat_add(chat, CHAT_SPEAKER_SYSTEM, "Local two-player mode. WHITE moves first.");
    }
}

static void process_command(const UiCommand* command, UiState* ui, GameState* game, ChatLog* chat) {
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
        }
        else {
            ui_set_status_message(ui, "Failed to save game.");
        }
        break;
    case UI_CMD_LOAD:
        if (load_game_from_path(game, chat, SAVE_FILE_PATH)) {
            ui_reset_game_interaction(ui);
            ui_set_view(ui, UI_VIEW_GAME);
            ui_set_status_message(ui, "Game loaded.");
        }
        else {
            ui_set_status_message(ui, "Failed to load game.");
        }
        break;
    case UI_CMD_PLAYER_MOVE: {
        if (game_apply_move(game, command->move.from, command->move.to)) {
            ui_reset_game_interaction(ui);
            if (game->isGameOver) {
                const char* winner = (game->score[PLAYER_WHITE] == game->score[PLAYER_BLACK]) ? "No one" : (game->score[PLAYER_WHITE] > game->score[PLAYER_BLACK] ? "WHITE" : "BLACK");
                char msg[64];
                snprintf(msg, sizeof(msg), "Game over. %s wins!", winner);
                ui_set_status_message(ui, msg);
                chat_add(chat, CHAT_SPEAKER_SYSTEM, msg);
            }
            else {
                ui_set_status_message(ui, "Move applied.");
            }
        }
        else {
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

static void maybe_run_ai(GameState* game, ChatLog* chat, UiState* ui) {
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
            const char* winner = (game->score[PLAYER_WHITE] == game->score[PLAYER_BLACK]) ? "No one" : (game->score[PLAYER_WHITE] > game->score[PLAYER_BLACK] ? "WHITE" : "BLACK");
            char msg[64];
            snprintf(msg, sizeof(msg), "Game over. %s wins!", winner);
            ui_set_status_message(ui, msg);
            chat_add(chat, CHAT_SPEAKER_SYSTEM, msg);
        }
    }
}

// Fonction pour afficher le menu de sélection des couleurs
static void show_color_menu(SDL_Renderer* renderer) {
    // Cette fonction devrait être appelée dans votre boucle de rendu
    // Elle affiche un menu simple pour changer les couleurs

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect menuRect = { 100, 100, 600, 400 };
    SDL_RenderFillRect(renderer, &menuRect);

    // Afficher les options de couleurs
    // (Vous devrez implémenter l'affichage du texte avec SDL_ttf)
}

// Fonction pour changer la couleur des pions blancs
void set_white_piece_color(int colorIndex) {
    if (colorIndex >= 0 && colorIndex < NUM_PRESET_COLORS) {
        pieceColors.whiteColor = PRESET_COLORS[colorIndex];
        printf("Couleur des pions blancs changée en: %s\n", COLOR_NAMES[colorIndex]);
    }
}

// Fonction pour changer la couleur des pions noirs
void set_black_piece_color(int colorIndex) {
    if (colorIndex >= 0 && colorIndex < NUM_PRESET_COLORS) {
        pieceColors.blackColor = PRESET_COLORS[colorIndex];
        printf("Couleur des pions noirs changée en: %s\n", COLOR_NAMES[colorIndex]);
    }
}

// Fonction pour obtenir la couleur actuelle d'un pion
SDL_Color get_piece_color(Player player) {
    return (player == PLAYER_WHITE) ? pieceColors.whiteColor : pieceColors.blackColor;
}

// Gestion des touches pour changer les couleurs
static void handle_color_change_keys(SDL_Keycode key) {
    static int whiteColorIndex = 0;
    static int blackColorIndex = 1;

    switch (key) {
    case SDLK_1: // Changer couleur pions blancs (précédent)
        whiteColorIndex = (whiteColorIndex - 1 + NUM_PRESET_COLORS) % NUM_PRESET_COLORS;
        set_white_piece_color(whiteColorIndex);
        break;
    case SDLK_2: // Changer couleur pions blancs (suivant)
        whiteColorIndex = (whiteColorIndex + 1) % NUM_PRESET_COLORS;
        set_white_piece_color(whiteColorIndex);
        break;
    case SDLK_3: // Changer couleur pions noirs (précédent)
        blackColorIndex = (blackColorIndex - 1 + NUM_PRESET_COLORS) % NUM_PRESET_COLORS;
        set_black_piece_color(blackColorIndex);
        break;
    case SDLK_4: // Changer couleur pions noirs (suivant)
        blackColorIndex = (blackColorIndex + 1) % NUM_PRESET_COLORS;
        set_black_piece_color(blackColorIndex);
        break;
    }
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

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

    printf("\n=== Contrôles des couleurs ===\n");
    printf("Touche 1: Couleur précédente pour pions BLANCS\n");
    printf("Touche 2: Couleur suivante pour pions BLANCS\n");
    printf("Touche 3: Couleur précédente pour pions NOIRS\n");
    printf("Touche 4: Couleur suivante pour pions NOIRS\n");
    printf("===============================\n\n");

    while (ui.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Gestion des touches pour les couleurs
            if (event.type == SDL_KEYDOWN) {
                handle_color_change_keys(event.key.keysym.sym);
            }

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
