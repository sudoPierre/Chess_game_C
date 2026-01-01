#include "ui.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

static SDL_Rect make_rect(int x, int y, int w, int h) {
    SDL_Rect rect = { x, y, w, h };
    return rect;
}

#define PIECE_ICON_RES 8

static const uint8_t PIECE_ICON_DATA[PIECE_TYPE_COUNT][PIECE_ICON_RES] = {
    {
        0x3C,
        0x3C,
        0x3C,
        0x7E,
        0x66,
        0x66,
        0x7E,
        0x24
    },
    {
        0x18,
        0x3C,
        0x7E,
        0x7E,
        0x3E,
        0x1E,
        0x3E,
        0x3C
    },
    {
        0x18,
        0x3C,
        0x66,
        0x66,
        0x3C,
        0x3C,
        0x7E,
        0x18
    },
    {
        0x5A,
        0x3C,
        0x7E,
        0x7E,
        0x7E,
        0x7E,
        0x3C,
        0x3C
    }
};

static SDL_Texture *load_texture(UiState *ui, const char *path) {
    if (!ui || !ui->renderer || !path) {
        return NULL;
    }

    SDL_Texture *texture = NULL;
    stbi_uc *pixels = NULL;
    int width = 0;
    int height = 0;
    int channels = 0;
    char *basePath = SDL_GetBasePath();
    if (basePath) {
        char fullPath[512];
        if (snprintf(fullPath, sizeof(fullPath), "%s%s", basePath, path) < (int)sizeof(fullPath)) {
            pixels = stbi_load(fullPath, &width, &height, &channels, STBI_rgb_alpha);
        }
        SDL_free(basePath);
    }

    if (!pixels) {
        pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    }

    if (!pixels) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to load PNG '%s': %s", path, stbi_failure_reason());
        return NULL;
    }

    if (width <= 0 || height <= 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Invalid image dimensions for '%s'", path);
        stbi_image_free(pixels);
        return NULL;
    }

    texture = SDL_CreateTexture(ui->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);

    if (!texture) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from '%s': %s", path, SDL_GetError());
        stbi_image_free(pixels);
        return NULL;
    }

    if (SDL_UpdateTexture(texture, NULL, pixels, width * 4) != 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to upload texture data for '%s': %s", path, SDL_GetError());
        SDL_DestroyTexture(texture);
        stbi_image_free(pixels);
        return NULL;
    }

    stbi_image_free(pixels);

    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to set blend mode for '%s': %s", path, SDL_GetError());
    }

    return texture;
}

static SDL_Texture *load_texture_with_variants(UiState *ui, const char *paths[], size_t count) {
    if (!paths || count == 0) {
        return NULL;
    }

    SDL_Texture *texture = NULL;
    for (size_t i = 0; i < count; ++i) {
        texture = load_texture(ui, paths[i]);
        if (texture) {
            return texture;
        }
    }

    const char *reason = stbi_failure_reason();
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to load icon texture '%s': %s", paths[count - 1], reason ? reason : "unknown error");
    return NULL;
}

static void destroy_piece_textures(UiState *ui) {
    if (!ui) {
        return;
    }
    for (int owner = 0; owner < PLAYER_COUNT; ++owner) {
        for (int type = 0; type < PIECE_TYPE_COUNT; ++type) {
            if (ui->pieceTextures[owner][type]) {
                SDL_DestroyTexture(ui->pieceTextures[owner][type]);
                ui->pieceTextures[owner][type] = NULL;
            }
        }
    }
}

static bool load_piece_textures(UiState *ui) {
    if (!ui || !ui->renderer) {
        return false;
    }

    destroy_piece_textures(ui);

    bool success = true;

    const char *whiteRook[] = { "icons/Rook-white.png" };
    ui->pieceTextures[PLAYER_WHITE][PIECE_ROOK] = load_texture_with_variants(ui, whiteRook, sizeof(whiteRook) / sizeof(whiteRook[0]));
    success &= ui->pieceTextures[PLAYER_WHITE][PIECE_ROOK] != NULL;

    const char *whiteKnight[] = { "icons/Knight-white.png" };
    ui->pieceTextures[PLAYER_WHITE][PIECE_KNIGHT] = load_texture_with_variants(ui, whiteKnight, sizeof(whiteKnight) / sizeof(whiteKnight[0]));
    success &= ui->pieceTextures[PLAYER_WHITE][PIECE_KNIGHT] != NULL;

    const char *whiteBishop[] = { "icons/Bishop-white.png", "icons/Bishop-hite.png" };
    ui->pieceTextures[PLAYER_WHITE][PIECE_BISHOP] = load_texture_with_variants(ui, whiteBishop, sizeof(whiteBishop) / sizeof(whiteBishop[0]));
    success &= ui->pieceTextures[PLAYER_WHITE][PIECE_BISHOP] != NULL;

    const char *whiteQueen[] = { "icons/Queen-white.png" };
    ui->pieceTextures[PLAYER_WHITE][PIECE_QUEEN] = load_texture_with_variants(ui, whiteQueen, sizeof(whiteQueen) / sizeof(whiteQueen[0]));
    success &= ui->pieceTextures[PLAYER_WHITE][PIECE_QUEEN] != NULL;

    const char *blackRook[] = { "icons/Rook-black.png" };
    ui->pieceTextures[PLAYER_BLACK][PIECE_ROOK] = load_texture_with_variants(ui, blackRook, sizeof(blackRook) / sizeof(blackRook[0]));
    success &= ui->pieceTextures[PLAYER_BLACK][PIECE_ROOK] != NULL;

    const char *blackKnight[] = { "icons/Knight-black.png" };
    ui->pieceTextures[PLAYER_BLACK][PIECE_KNIGHT] = load_texture_with_variants(ui, blackKnight, sizeof(blackKnight) / sizeof(blackKnight[0]));
    success &= ui->pieceTextures[PLAYER_BLACK][PIECE_KNIGHT] != NULL;

    const char *blackBishop[] = { "icons/Bishop-black.png" };
    ui->pieceTextures[PLAYER_BLACK][PIECE_BISHOP] = load_texture_with_variants(ui, blackBishop, sizeof(blackBishop) / sizeof(blackBishop[0]));
    success &= ui->pieceTextures[PLAYER_BLACK][PIECE_BISHOP] != NULL;

    const char *blackQueen[] = { "icons/Queen-black.png" };
    ui->pieceTextures[PLAYER_BLACK][PIECE_QUEEN] = load_texture_with_variants(ui, blackQueen, sizeof(blackQueen) / sizeof(blackQueen[0]));
    success &= ui->pieceTextures[PLAYER_BLACK][PIECE_QUEEN] != NULL;

    return success;
}

static void draw_piece_fallback(SDL_Renderer *renderer, SDL_Rect tile, PieceType type, Player owner) {
    if (!renderer || type < 0 || type >= PIECE_TYPE_COUNT) {
        return;
    }

    int scale = tile.w / PIECE_ICON_RES;
    if (tile.h / PIECE_ICON_RES < scale) {
        scale = tile.h / PIECE_ICON_RES;
    }
    if (scale < 3) {
        scale = 3;
    }

    int marginX = (tile.w - scale * PIECE_ICON_RES) / 2;
    int marginY = (tile.h - scale * PIECE_ICON_RES) / 2;

    SDL_Color baseColor = owner == PLAYER_WHITE ? (SDL_Color){ 240, 240, 240, 255 } : (SDL_Color){ 30, 35, 50, 255 };
    SDL_Color accentColor = owner == PLAYER_WHITE ? (SDL_Color){ 200, 210, 220, 255 } : (SDL_Color){ 120, 130, 170, 255 };

    SDL_SetRenderDrawColor(renderer, baseColor.r, baseColor.g, baseColor.b, baseColor.a);

    const uint8_t *rows = PIECE_ICON_DATA[type];
    for (int row = 0; row < PIECE_ICON_RES; ++row) {
        uint8_t rowMask = rows[row];
        for (int col = 0; col < PIECE_ICON_RES; ++col) {
            if ((rowMask & (1u << (7 - col))) == 0u) {
                continue;
            }
            SDL_Rect pixel = {
                tile.x + marginX + col * scale,
                tile.y + marginY + row * scale,
                scale,
                scale
            };
            SDL_RenderFillRect(renderer, &pixel);
        }
    }

    SDL_SetRenderDrawColor(renderer, accentColor.r, accentColor.g, accentColor.b, accentColor.a);
    SDL_Rect outline = {
        tile.x + marginX,
        tile.y + marginY,
        scale * PIECE_ICON_RES,
        scale * PIECE_ICON_RES
    };
    SDL_RenderDrawRect(renderer, &outline);
}

static void draw_piece(UiState *ui, SDL_Rect tile, PieceType type, Player owner) {
    if (!ui || owner < 0 || owner >= PLAYER_COUNT) {
        return;
    }

    SDL_Texture *texture = NULL;
    if (type >= 0 && type < PIECE_TYPE_COUNT) {
        texture = ui->pieceTextures[owner][type];
    }

    if (texture) {
        int texW = 0;
        int texH = 0;
        if (SDL_QueryTexture(texture, NULL, NULL, &texW, &texH) != 0 || texW <= 0 || texH <= 0) {
            draw_piece_fallback(ui->renderer, tile, type, owner);
            return;
        }

        double scaleX = (double)tile.w / (double)texW;
        double scaleY = (double)tile.h / (double)texH;
        double scale = scaleX < scaleY ? scaleX : scaleY;
        if (scale >= 1.0) {
            scale *= 0.9; // leave a small margin when the texture would fill the tile
        }

        int drawW = (int)(texW * scale);
        int drawH = (int)(texH * scale);
        if (drawW <= 0 || drawH <= 0) {
            draw_piece_fallback(ui->renderer, tile, type, owner);
            return;
        }

        SDL_Rect dest = {
            tile.x + (tile.w - drawW) / 2,
            tile.y + (tile.h - drawH) / 2,
            drawW,
            drawH
        };

        SDL_RenderCopy(ui->renderer, texture, NULL, &dest);
        return;
    }

    draw_piece_fallback(ui->renderer, tile, type, owner);
}

static int board_pixel_left(const UiState *ui) {
    return ui->boardOriginX;
}

static int board_pixel_top(const UiState *ui) {
    return ui->boardOriginY;
}

static int board_pixel_size(const UiState *ui) {
    return ui->tileSize * BOARD_SIZE;
}

static SDL_Rect board_rect(const UiState *ui) {
    return make_rect(board_pixel_left(ui), board_pixel_top(ui), board_pixel_size(ui), board_pixel_size(ui));
}

static SDL_Rect main_menu_button_rect(int index) {
    const int buttonWidth = 320;
    const int buttonHeight = 64;
    const int startY = 240;
    const int gap = 24;
    int x = (WINDOW_WIDTH - buttonWidth) / 2;
    int y = startY + index * (buttonHeight + gap);
    return make_rect(x, y, buttonWidth, buttonHeight);
}

static SDL_Rect pause_menu_button_rect(int index) {
    const int buttonWidth = 280;
    const int buttonHeight = 60;
    const int startY = (WINDOW_HEIGHT / 2) - 90;
    const int gap = 16;
    int x = (WINDOW_WIDTH - buttonWidth) / 2;
    int y = startY + index * (buttonHeight + gap);
    return make_rect(x, y, buttonWidth, buttonHeight);
}

static SDL_Rect game_panel_rect(const UiState *ui) {
    int sideX = ui->boardOriginX + ui->tileSize * BOARD_SIZE + 32;
    int width = WINDOW_WIDTH - sideX - 32;
    if (width < 200) {
        width = 200;
    }
    return make_rect(sideX, 32, width, WINDOW_HEIGHT - 64);
}

static SDL_Rect game_button_rect(const UiState *ui, int index) {
    SDL_Rect panel = game_panel_rect(ui);
    const int buttonHeight = 44;
    const int gap = 12;
    int x = panel.x + 16;
    int y = panel.y + 140 + index * (buttonHeight + gap);
    int width = panel.w - 32;
    return make_rect(x, y, width, buttonHeight);
}

static void draw_rect(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_rect_outline(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

static void render_text_center(SDL_Renderer *renderer, const BitmapFont *font, SDL_Rect rect, const char *text, SDL_Color color) {
    int textWidth = bitmap_font_measure_text(font, text);
    int yOffset = font->glyphHeight * BITMAP_FONT_SCALE;
    int x = rect.x + (rect.w - textWidth) / 2;
    int y = rect.y + (rect.h - yOffset) / 2;
    bitmap_font_draw_text(renderer, font, x, y, text, color);
}

static void render_board(UiState *ui, const GameState *game) {
    SDL_Renderer *renderer = ui->renderer;
    SDL_Rect boardArea = board_rect(ui);
    SDL_Color light = { 240, 217, 181, 255 };
    SDL_Color dark = { 181, 136, 99, 255 };
    SDL_Color highlight = { 208, 82, 107, 200 };
    SDL_Color hoverColor = { 250, 250, 120, 160 };

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            SDL_Rect tile = make_rect(boardArea.x + col * ui->tileSize, boardArea.y + row * ui->tileSize, ui->tileSize, ui->tileSize);
            SDL_Color tileColor = ((row + col) % 2 == 0) ? light : dark;

            draw_rect(renderer, tile, tileColor);

            if (ui->hasSelection && ui->selectedSquare.row == row && ui->selectedSquare.col == col) {
                draw_rect(renderer, tile, highlight);
            } else if (ui->hasSelection && game_is_valid_move(game, ui->selectedSquare, (Position){ row, col })) {
                SDL_Color moveColor = { 90, 200, 120, 140 };
                draw_rect(renderer, tile, moveColor);
            }

            if (ui->hasHover && ui->hoverSquare.row == row && ui->hoverSquare.col == col) {
                draw_rect(renderer, tile, hoverColor);
            }

            SDL_Color outline = { 30, 30, 30, 255 };
            draw_rect_outline(renderer, tile, outline);

            const Square *sq = &game->board[row][col];
            if (sq->occupied) {
                draw_piece(ui, tile, sq->type, sq->owner);
            }
        }
    }
}

static void render_scores(UiState *ui, const GameState *game) {
    SDL_Rect panel = game_panel_rect(ui);
    SDL_Renderer *renderer = ui->renderer;
    SDL_Color panelColor = { 45, 45, 55, 255 };
    draw_rect(renderer, panel, panelColor);

    SDL_Color accent = { 200, 200, 210, 255 };
    bitmap_font_draw_text(renderer, &ui->font, panel.x + 16, panel.y + 12, "SCORE", accent);

    char scoreLine[64];
    snprintf(scoreLine, sizeof(scoreLine), "WHITE: %d", game->score[PLAYER_WHITE]);
    bitmap_font_draw_text(renderer, &ui->font, panel.x + 16, panel.y + 36, scoreLine, (SDL_Color){ 235, 235, 240, 255 });

    snprintf(scoreLine, sizeof(scoreLine), "BLACK: %d", game->score[PLAYER_BLACK]);
    bitmap_font_draw_text(renderer, &ui->font, panel.x + 16, panel.y + 60, scoreLine, (SDL_Color){ 235, 235, 240, 255 });

    const char *turnText = (game->currentPlayer == PLAYER_WHITE) ? "TURN: WHITE" : "TURN: BLACK";
    bitmap_font_draw_text(renderer, &ui->font, panel.x + 16, panel.y + 92, turnText, (SDL_Color){ 160, 220, 255, 255 });
}

static void render_game_buttons(UiState *ui, const GameState *game) {
    SDL_Renderer *renderer = ui->renderer;
    const char *labels[] = {
        game->isPaused ? "RESUME" : "PAUSE",
        "SAVE",
        "LOAD",
        "MAIN MENU",
        "SWAP CHAT"
    };
    SDL_Color buttonColor = { 70, 70, 90, 255 };
    SDL_Color outlineColor = { 110, 110, 140, 255 };
    SDL_Color textColor = { 230, 230, 240, 255 };

    for (int i = 0; i < 5; ++i) {
        SDL_Rect button = game_button_rect(ui, i);
        draw_rect(renderer, button, buttonColor);
        draw_rect_outline(renderer, button, outlineColor);
        render_text_center(renderer, &ui->font, button, labels[i], textColor);
    }
}

static void render_chat_panel(UiState *ui, const ChatLog *chat) {
    ChatLog emptyLog;
    if (!chat) {
        memset(&emptyLog, 0, sizeof(emptyLog));
        chat = &emptyLog;
    }
    SDL_Renderer *renderer = ui->renderer;
    SDL_Rect panel = game_panel_rect(ui);
    int chatTop = panel.y + 140 + 5 * (44 + 12) + 24;
    int chatHeight = panel.y + panel.h - chatTop - 96;
    if (chatHeight < 120) {
        chatHeight = 120;
    }

    SDL_Rect chatRect = make_rect(panel.x + 16, chatTop, panel.w - 32, chatHeight);
    draw_rect(renderer, chatRect, (SDL_Color){ 35, 35, 45, 255 });
    draw_rect_outline(renderer, chatRect, (SDL_Color){ 80, 80, 110, 255 });

    int lineHeight = ui->font.glyphHeight * BITMAP_FONT_SCALE + 6;
    int maxLines = chatHeight / lineHeight;
    int startIndex = 0;
    if ((int)chat->count > maxLines) {
        startIndex = (int)chat->count - maxLines;
    }

    int y = chatRect.y + 8;
    for (int i = startIndex; i < (int)chat->count; ++i) {
        const ChatEntry *entry = &chat->entries[i];
        SDL_Color speakerColor = { 200, 200, 210, 255 };
        if (entry->speaker == CHAT_SPEAKER_WHITE) {
            speakerColor = (SDL_Color){ 220, 220, 255, 255 };
        } else if (entry->speaker == CHAT_SPEAKER_BLACK) {
            speakerColor = (SDL_Color){ 255, 210, 210, 255 };
        }
        char line[CHAT_MESSAGE_LENGTH + 16];
        snprintf(line, sizeof(line), "%s: %s", chat_speaker_label(entry->speaker), entry->message);
        bitmap_font_draw_text(renderer, &ui->font, chatRect.x + 8, y, line, speakerColor);
        y += lineHeight;
    }

    SDL_Rect inputRect = make_rect(panel.x + 16, chatRect.y + chatRect.h + 12, panel.w - 32, 48);
    draw_rect(renderer, inputRect, (SDL_Color){ 25, 25, 35, 255 });
    draw_rect_outline(renderer, inputRect, (SDL_Color){ 90, 90, 120, 255 });

    char prompt[160];
    snprintf(prompt, sizeof(prompt), "%s > %s_", chat_speaker_label(ui->chatSpeaker), ui->chatInput);
    bitmap_font_draw_text(renderer, &ui->font, inputRect.x + 8, inputRect.y + 12, prompt, (SDL_Color){ 200, 200, 210, 255 });

    bitmap_font_draw_text(renderer, &ui->font, inputRect.x, inputRect.y + 32, "ENTER TO SEND / TAB TO SWITCH", (SDL_Color){ 120, 160, 200, 255 });
}

static void render_status_banner(UiState *ui) {
    if (!ui->statusVisible) {
        return;
    }
    SDL_Rect rect = make_rect(0, WINDOW_HEIGHT - 48, WINDOW_WIDTH, 48);
    draw_rect(ui->renderer, rect, (SDL_Color){ 20, 40, 60, 220 });
    SDL_Color textColor = { 240, 250, 255, 255 };
    bitmap_font_draw_text(ui->renderer, &ui->font, rect.x + 16, rect.y + 12, ui->statusMessage, textColor);
}

static void render_game_scene(UiState *ui, const GameState *game, const ChatLog *chat) {
    render_board(ui, game);
    render_scores(ui, game);
    render_game_buttons(ui, game);
    render_chat_panel(ui, chat);

    if (game->isGameOver) {
        SDL_Rect boardArea = board_rect(ui);
        SDL_Rect overlay = boardArea;
        draw_rect(ui->renderer, overlay, (SDL_Color){ 10, 10, 10, 140 });
        render_text_center(ui->renderer, &ui->font, overlay, "GAME OVER", (SDL_Color){ 255, 230, 120, 255 });
    }
}

static void render_pause_overlay(UiState *ui) {
    SDL_Rect overlay = make_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    draw_rect(ui->renderer, overlay, (SDL_Color){ 0, 0, 0, 140 });
    SDL_Color titleColor = { 230, 230, 240, 255 };
    SDL_Rect titleRect = make_rect(0, WINDOW_HEIGHT / 2 - 150, WINDOW_WIDTH, 60);
    render_text_center(ui->renderer, &ui->font, titleRect, "PAUSED", titleColor);

    const char *labels[] = { "RESUME", "SAVE", "LOAD", "MAIN MENU" };
    for (int i = 0; i < 4; ++i) {
        SDL_Rect button = pause_menu_button_rect(i);
        draw_rect(ui->renderer, button, (SDL_Color){ 35, 35, 50, 230 });
        draw_rect_outline(ui->renderer, button, (SDL_Color){ 140, 140, 180, 255 });
        render_text_center(ui->renderer, &ui->font, button, labels[i], (SDL_Color){ 235, 235, 240, 255 });
    }
}

static void render_main_menu(UiState *ui) {
    SDL_Renderer *renderer = ui->renderer;
    SDL_SetRenderDrawColor(renderer, 18, 22, 36, 255);
    SDL_RenderClear(renderer);

    SDL_Rect titleRect = make_rect(0, 120, WINDOW_WIDTH, 64);
    render_text_center(renderer, &ui->font, titleRect, "SIMPLIFIED CHESS", (SDL_Color){ 240, 240, 255, 255 });

    const char *labels[] = {
        "PLAY VS COMPUTER",
        "PLAY VS PLAYER",
        "LOAD GAME",
        "QUIT"
    };

    for (int i = 0; i < 4; ++i) {
        SDL_Rect button = main_menu_button_rect(i);
        draw_rect(renderer, button, (SDL_Color){ 35, 48, 82, 255 });
        draw_rect_outline(renderer, button, (SDL_Color){ 120, 140, 190, 255 });
        render_text_center(renderer, &ui->font, button, labels[i], (SDL_Color){ 235, 235, 240, 255 });
    }

    SDL_Rect footerRect = make_rect(0, WINDOW_HEIGHT - 48, WINDOW_WIDTH, 32);
    render_text_center(renderer, &ui->font, footerRect, "LEFT CLICK TO SELECT OPTIONS", (SDL_Color){ 160, 200, 220, 255 });
}

bool ui_init(UiState *ui, const char *title) {
    if (!ui) {
        return false;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        return false;
    }

    memset(ui, 0, sizeof(*ui));

    ui->window = SDL_CreateWindow(title ? title : "Simplified Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!ui->window) {
        SDL_Quit();
        return false;
    }

    ui->renderer = SDL_CreateRenderer(ui->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ui->renderer) {
        SDL_DestroyWindow(ui->window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawBlendMode(ui->renderer, SDL_BLENDMODE_BLEND);

    if (!load_piece_textures(ui)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "One or more piece textures failed to load. Using fallback renderer for missing assets.");
    }

    if (!bitmap_font_init(&ui->font)) {
        destroy_piece_textures(ui);
        SDL_DestroyRenderer(ui->renderer);
        SDL_DestroyWindow(ui->window);
        SDL_Quit();
        return false;
    }

    ui->view = UI_VIEW_MAIN_MENU;
    ui->running = true;
    ui->tileSize = 64;
    ui->boardOriginX = 48;
    ui->boardOriginY = 48;
    ui->chatSpeaker = CHAT_SPEAKER_WHITE;
    ui->statusVisible = false;

    SDL_StartTextInput();
    return true;
}

void ui_cleanup(UiState *ui) {
    if (!ui) {
        return;
    }
    destroy_piece_textures(ui);
    bitmap_font_shutdown(&ui->font);
    if (ui->renderer) {
        SDL_DestroyRenderer(ui->renderer);
        ui->renderer = NULL;
    }
    if (ui->window) {
        SDL_DestroyWindow(ui->window);
        ui->window = NULL;
    }
    SDL_StopTextInput();
    SDL_Quit();
}

void ui_render(UiState *ui, const GameState *game, const ChatLog *chat) {
    if (!ui) {
        return;
    }

    SDL_SetRenderDrawColor(ui->renderer, 18, 22, 36, 255);
    SDL_RenderClear(ui->renderer);

    if (ui->view == UI_VIEW_MAIN_MENU) {
        render_main_menu(ui);
    } else {
        if (game) {
            render_game_scene(ui, game, chat);
        }
        if (ui->view == UI_VIEW_PAUSE) {
            render_pause_overlay(ui);
        }
    }

    render_status_banner(ui);

    SDL_RenderPresent(ui->renderer);
}

static bool point_in_rect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h;
}

static bool screen_to_board(const UiState *ui, int x, int y, Position *outPos) {
    if (!ui || !outPos) {
        return false;
    }
    SDL_Rect boardArea = board_rect(ui);
    if (!point_in_rect(x, y, boardArea)) {
        return false;
    }
    int col = (x - boardArea.x) / ui->tileSize;
    int row = (y - boardArea.y) / ui->tileSize;
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return false;
    }
    outPos->row = row;
    outPos->col = col;
    return true;
}

static void update_hover_square(UiState *ui, int x, int y) {
    Position pos;
    if (screen_to_board(ui, x, y, &pos)) {
        ui->hoverSquare = pos;
        ui->hasHover = true;
    } else {
        ui->hasHover = false;
    }
}

static bool handle_main_menu_click(int x, int y, UiCommand *outCommand) {
    for (int i = 0; i < 4; ++i) {
        SDL_Rect button = main_menu_button_rect(i);
        if (point_in_rect(x, y, button)) {
            switch (i) {
                case 0:
                    outCommand->type = UI_CMD_START_PVE;
                    return true;
                case 1:
                    outCommand->type = UI_CMD_START_PVP;
                    return true;
                case 2:
                    outCommand->type = UI_CMD_LOAD;
                    return true;
                case 3:
                    outCommand->type = UI_CMD_QUIT;
                    return true;
                default:
                    break;
            }
        }
    }
    return false;
}

static bool handle_pause_menu_click(int x, int y, UiCommand *outCommand) {
    for (int i = 0; i < 4; ++i) {
        SDL_Rect button = pause_menu_button_rect(i);
        if (point_in_rect(x, y, button)) {
            switch (i) {
                case 0:
                    outCommand->type = UI_CMD_RESUME;
                    return true;
                case 1:
                    outCommand->type = UI_CMD_SAVE;
                    return true;
                case 2:
                    outCommand->type = UI_CMD_LOAD;
                    return true;
                case 3:
                    outCommand->type = UI_CMD_MAIN_MENU;
                    return true;
                default:
                    break;
            }
        }
    }
    return false;
}

static bool handle_game_click(UiState *ui, const GameState *game, int x, int y, UiCommand *outCommand) {
    if (!game) {
        return false;
    }

    Position boardPos;
    if (screen_to_board(ui, x, y, &boardPos)) {
        if (!ui->hasSelection) {
            const Square *sq = &game->board[boardPos.row][boardPos.col];
            if (sq->occupied && sq->owner == game->currentPlayer) {
                ui->hasSelection = true;
                ui->selectedSquare = boardPos;
            }
        } else {
            if (ui->selectedSquare.row == boardPos.row && ui->selectedSquare.col == boardPos.col) {
                ui->hasSelection = false;
            } else {
                outCommand->type = UI_CMD_PLAYER_MOVE;
                outCommand->move.from = ui->selectedSquare;
                outCommand->move.to = boardPos;
                ui->hasSelection = false;
                return true;
            }
        }
        return false;
    }

    for (int i = 0; i < 5; ++i) {
        SDL_Rect button = game_button_rect(ui, i);
        if (point_in_rect(x, y, button)) {
            switch (i) {
                case 0:
                    outCommand->type = game->isPaused ? UI_CMD_RESUME : UI_CMD_PAUSE;
                    return true;
                case 1:
                    outCommand->type = UI_CMD_SAVE;
                    return true;
                case 2:
                    outCommand->type = UI_CMD_LOAD;
                    return true;
                case 3:
                    outCommand->type = UI_CMD_MAIN_MENU;
                    return true;
                case 4:
                    ui_toggle_chat_speaker(ui);
                    return false;
                default:
                    break;
            }
        }
    }

    SDL_Rect panel = game_panel_rect(ui);
    SDL_Rect inputRect = make_rect(panel.x + 16, panel.y + panel.h - 60, panel.w - 32, 48);
    if (point_in_rect(x, y, inputRect)) {
        return false;
    }

    return false;
}

bool ui_handle_event(UiState *ui, const SDL_Event *event, const GameState *game, UiCommand *outCommand) {
    if (!ui || !event || !outCommand) {
        return false;
    }

    memset(outCommand, 0, sizeof(*outCommand));
    outCommand->type = UI_CMD_NONE;

    switch (event->type) {
        case SDL_QUIT:
            outCommand->type = UI_CMD_QUIT;
            return true;

        case SDL_MOUSEMOTION:
            if (ui->view != UI_VIEW_MAIN_MENU) {
                update_hover_square(ui, event->motion.x, event->motion.y);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                int x = event->button.x;
                int y = event->button.y;
                if (ui->view == UI_VIEW_MAIN_MENU) {
                    return handle_main_menu_click(x, y, outCommand);
                }
                if (ui->view == UI_VIEW_PAUSE) {
                    return handle_pause_menu_click(x, y, outCommand);
                }
                if (ui->view == UI_VIEW_GAME) {
                    return handle_game_click(ui, game, x, y, outCommand);
                }
            } else if (event->button.button == SDL_BUTTON_RIGHT) {
                ui->hasSelection = false;
            }
            break;

        case SDL_KEYDOWN: {
            SDL_Keycode key = event->key.keysym.sym;
            if (key == SDLK_ESCAPE) {
                if (ui->view == UI_VIEW_GAME) {
                    outCommand->type = UI_CMD_PAUSE;
                    return true;
                }
                if (ui->view == UI_VIEW_PAUSE) {
                    outCommand->type = UI_CMD_RESUME;
                    return true;
                }
            }
            if (ui->view == UI_VIEW_GAME) {
                if (key == SDLK_BACKSPACE) {
                    if (ui->chatInputLength > 0) {
                        ui->chatInput[--ui->chatInputLength] = '\0';
                    }
                } else if (key == SDLK_RETURN || key == SDLK_RETURN2 || key == SDLK_KP_ENTER) {
                    if (ui->chatInputLength > 0) {
                        outCommand->type = UI_CMD_CHAT_MESSAGE;
                        strncpy(outCommand->chatMessage, ui->chatInput, CHAT_MESSAGE_LENGTH - 1);
                        outCommand->chatMessage[CHAT_MESSAGE_LENGTH - 1] = '\0';
                        ui->chatInputLength = 0;
                        ui->chatInput[0] = '\0';
                        return true;
                    }
                } else if (key == SDLK_TAB) {
                    ui_toggle_chat_speaker(ui);
                }
            }
            break;
        }

        case SDL_TEXTINPUT:
            if (ui->view == UI_VIEW_GAME) {
                size_t len = strnlen(event->text.text, sizeof(event->text.text));
                for (size_t i = 0; i < len; ++i) {
                    if (ui->chatInputLength + 1 < CHAT_INPUT_LENGTH) {
                        ui->chatInput[ui->chatInputLength++] = event->text.text[i];
                        ui->chatInput[ui->chatInputLength] = '\0';
                    }
                }
            }
            break;

        default:
            break;
    }

    return false;
}

void ui_set_view(UiState *ui, UiView view) {
    if (!ui) {
        return;
    }
    ui->view = view;
    if (view != UI_VIEW_GAME) {
        ui->hasSelection = false;
        ui->hasHover = false;
    }
}

void ui_set_status_message(UiState *ui, const char *message) {
    if (!ui || !message) {
        return;
    }
    strncpy(ui->statusMessage, message, sizeof(ui->statusMessage) - 1);
    ui->statusMessage[sizeof(ui->statusMessage) - 1] = '\0';
    ui->statusVisible = true;
    ui->statusVisibleUntil = SDL_GetTicks() + STATUS_MESSAGE_DURATION_MS;
}

void ui_update(UiState *ui, Uint32 deltaMs) {
    (void)deltaMs;
    if (!ui) {
        return;
    }
    if (ui->statusVisible && SDL_TICKS_PASSED(SDL_GetTicks(), ui->statusVisibleUntil)) {
        ui->statusVisible = false;
    }
}

ChatSpeaker ui_current_chat_speaker(const UiState *ui) {
    if (!ui) {
        return CHAT_SPEAKER_SYSTEM;
    }
    return ui->chatSpeaker;
}

void ui_toggle_chat_speaker(UiState *ui) {
    if (!ui) {
        return;
    }
    if (ui->chatSpeaker == CHAT_SPEAKER_WHITE) {
        ui->chatSpeaker = CHAT_SPEAKER_BLACK;
    } else {
        ui->chatSpeaker = CHAT_SPEAKER_WHITE;
    }
}

void ui_reset_game_interaction(UiState *ui) {
    if (!ui) {
        return;
    }
    ui->hasSelection = false;
    ui->hasHover = false;
}
