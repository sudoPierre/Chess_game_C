# Simplified Chess GUI

Minimalist chess-inspired board game written in C with an SDL2-powered interface. Two players (or one player vs a lightweight AI) battle over an 8x8 grid using four custom pieces per side. The project showcases a clean separation between UI, game logic, persistence, and chat subsystems.

## Highlights
- Four-piece squads (Rook, Knight, Bishop, Queen) that all advance with a simplified one-step-forward rule and diagonal captures.
- SDL2 GUI featuring live scoreboard, pause overlay, clickable sidebar buttons, and a pixel-art icon set for each piece.
- Integrated chat log for local multiplayer conversations, including speaker switching.
- Save/Load flow backed by a readable text format stored at `saves/save_slot1.dat`.
- Modular code layout in `src/` and `include/` for straightforward extension or classroom demos.

## Rules At A Glance
- Board: 8 rows x 8 columns.
- Starting lineup: four pieces per side, positioned on the back rank.
- Movement: all pieces (regardless of type) move one square forward toward the opponent. Straight moves require an empty square; diagonal moves capture.
- Turn order: White moves first, then players alternate.
- Scoring: each capture yields one point. The game ends when the current player cannot move.

## Controls
- **Main Menu**
	- Left-click buttons to start a PvE or PvP game, load a save, or quit.
- **During a Match**
	- Left-click a piece to select it, then click a legal destination square to move.
	- Right-click cancels the selection.
	- Sidebar buttons: Pause/Resume, Save, Load, Main Menu, and Chat speaker swap.
	- Keyboard shortcuts: `Esc` toggles pause, `Tab` switches chat speaker, `Enter` sends chat, `Backspace` edits the chat input.
- **Pause Menu**
	- Left-click to Resume, Save, Load, or return to the Main Menu.

## Saving & Loading
- Use the sidebar or pause menu buttons to save or load.
- The game writes to `saves/save_slot1.dat`. Make sure the `saves/` directory exists and is writable.
- Saved data includes board state, scores, turn, pause status, and full chat history.

## Project Layout
- `src/` — C sources for the UI, game logic, AI helper, chat log, save system, and bitmap font.
- `include/` — Public headers exposing the modular APIs.
- `saves/` — Default save-file location (plain text).
- `Makefile` — Build script using `sdl2-config` for platform portability.

## Build Requirements
- SDL2 development libraries (macOS: `brew install sdl2`, Ubuntu: `sudo apt install libsdl2-dev`).
- ANSI C11 compiler (Clang or GCC).
- `make` build tool.

## Quick Start
```bash
make          # compile sources into ./chess_game
./chess_game  # launch the application
```

## Troubleshooting
- **SDL2/SDL.h not found**: ensure `sdl2-config` is in your `PATH` or install SDL2 development headers.
- **Window does not open**: check that your machine allows GUI apps (if over SSH, enable X forwarding or run locally).
- **Save issues**: verify write permissions on the `saves/` directory.

## Extending The Project
- Adjust starting formations or movement rules via `src/game_logic.c`.
- Improve AI decision-making in `src/ai.c` by swapping the random move picker with heuristics.
- Drop new icons in `src/ui.c` by editing the `PIECE_ICON_DATA` bitmap arrays.
- Add networking or alternate UIs by reusing the existing game-state module.

---

Built with simplicity in mind—fork, experiment, and enjoy the mini-chess experience!