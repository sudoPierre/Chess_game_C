#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL.h>
#include "game_logic.h"
#include "chat.h"
#include "bitmap_font.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720
#define CHAT_INPUT_LENGTH 96
#define STATUS_MESSAGE_DURATION_MS 3000

typedef enum {
    UI_VIEW_MAIN_MENU = 0,
    UI_VIEW_GAME,
    UI_VIEW_PAUSE
} UiView;

typedef enum {
    UI_CMD_NONE = 0,
    UI_CMD_QUIT,
    UI_CMD_START_PVE,
    UI_CMD_START_PVP,
    UI_CMD_MAIN_MENU,
    UI_CMD_PAUSE,
    UI_CMD_RESUME,
    UI_CMD_SAVE,
    UI_CMD_LOAD,
    UI_CMD_PLAYER_MOVE,
    UI_CMD_CHAT_MESSAGE
} UiCommandType;

typedef struct {
    UiCommandType type;
    Move move;
    char chatMessage[CHAT_MESSAGE_LENGTH];
} UiCommand;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    BitmapFont font;
    UiView view;
    bool running;
    int boardOriginX;
    int boardOriginY;
    int tileSize;
    bool hasSelection;
    Position selectedSquare;
    bool hasHover;
    Position hoverSquare;
    char chatInput[CHAT_INPUT_LENGTH];
    size_t chatInputLength;
    ChatSpeaker chatSpeaker;
    char statusMessage[128];
    bool statusVisible;
    Uint32 statusVisibleUntil;
} UiState;

bool ui_init(UiState *ui, const char *title);
void ui_cleanup(UiState *ui);
void ui_render(UiState *ui, const GameState *game, const ChatLog *chat);
bool ui_handle_event(UiState *ui, const SDL_Event *event, const GameState *game, UiCommand *outCommand);
void ui_set_view(UiState *ui, UiView view);
void ui_set_status_message(UiState *ui, const char *message);
void ui_update(UiState *ui, Uint32 deltaMs);
ChatSpeaker ui_current_chat_speaker(const UiState *ui);
void ui_toggle_chat_speaker(UiState *ui);
void ui_reset_game_interaction(UiState *ui);

#endif // UI_H
