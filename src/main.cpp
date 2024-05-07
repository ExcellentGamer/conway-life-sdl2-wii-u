#include <coreinit/debug.h>
#include <coreinit/title.h>
#include <padscore/kpad.h>
#include <sndcore2/core.h>
#include <sysapp/launch.h>
#include <whb/proc.h>
#include <SDL.h>

#include "input/CombinedInput.h"
#include "input/VPADInput.h"
#include "input/WPADInput.h"

#include "game.hpp"
#include "render.hpp"
#include "util.hpp"

// Constants
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const int UNIT_SIZE = 10;
const int STEP_INCREMENT = 25;
const int MIN_SPAWN_BOX_SIZE = 1;
const int MAX_SPAWN_BOX_SIZE = (WINDOW_HEIGHT / UNIT_SIZE) / 2;
const int MIN_STEP_LENGTH = 0;
const int MAX_STEP_LENGTH = 100;
const int MAX__POINTER_SPEED = 25;
const int MIN_POINTER_SPEED = 1;

// Global data
SDL_Window *main_window;
SDL_Renderer *main_renderer;
SDL_Event event;
GameState *game_state;
Uint64 delay;
Uint64 ticks;
int x_pointer = WINDOW_WIDTH / 2;
int y_pointer = WINDOW_HEIGHT / 2;
int pointer_speed = 2;
int population_size;

// Global controls
bool paused = true;
bool quit = false;
bool fill_cells = true;
bool color_cells = true;
int step_length = 0;
int spawn_box_size = 3;

//
// Setup everything needed for the game loop
//
int initialise()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        OSReport("SDL_Init failed with error: ", SDL_GetError(), "\n");
        return EXIT_FAILURE;
    }

    // Handle window creation
    main_window = SDL_CreateWindow(
        "Game of Life SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);

    if (main_window == nullptr)
    {
        OSReport("SDL_CreateWindow failed with error: ", SDL_GetError(), "\n");
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Handle renderer creation
    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // Handle game state creation
    game_state = create_game_state(WINDOW_WIDTH, WINDOW_HEIGHT, UNIT_SIZE);
    population_size = 0;

    return EXIT_SUCCESS;
}

//
// Called before quitting for cleanup
//
void shutdown()
{
    SDL_DestroyWindow(main_window);
    SDL_DestroyRenderer(main_renderer);
    SDL_Quit();
    destroy_game_state(game_state);
}

//
// Called every frame for handling input
//
void input(Input &input)
{
    // Delete / Create cells
    if (input.data.buttons_h & Input::BUTTON_A || input.data.buttons_h & Input::BUTTON_B)
    {
        int offset = floor(spawn_box_size / 2);
        int row_selected = x_pointer / UNIT_SIZE;
        int col_selected = y_pointer / UNIT_SIZE;
        for (int ii = row_selected; ii < row_selected + spawn_box_size; ii++)
        {
            for (int jj = col_selected; jj < col_selected + spawn_box_size; jj++)
            {
                int row = ii - offset;
                int col = jj - offset;
                if (input.data.buttons_h & Input::BUTTON_A)
                {
                    spawn_cell(game_state, row, col, &population_size);
                }
                else if (input.data.buttons_h & Input::BUTTON_B)
                {
                    kill_cell(game_state, row, col, &population_size);
                }
            }
        }
    }

    // Move the pointer
    if (input.data.buttons_h & Input::STICK_L_UP)
    {
        y_pointer -= pointer_speed;

        if (y_pointer < (0 - spawn_box_size))
        {
            y_pointer = WINDOW_HEIGHT;
        }
    }

    if (input.data.buttons_h & Input::STICK_L_DOWN)
    {
        y_pointer += pointer_speed;

        if (y_pointer > (WINDOW_HEIGHT + spawn_box_size))
        {
            y_pointer = 0;
        }
    }

    if (input.data.buttons_h & Input::STICK_L_LEFT)
    {
        x_pointer -= pointer_speed;

        if (x_pointer < (0 - spawn_box_size))
        {
            x_pointer = WINDOW_WIDTH;
        }
    }

    if (input.data.buttons_h & Input::STICK_L_RIGHT)
    {
        x_pointer += pointer_speed;

        if (x_pointer > (WINDOW_WIDTH + spawn_box_size))
        {
            x_pointer = 0;
        }
    }

    // Other inputs
    if (input.data.buttons_d & Input::BUTTON_X) {
        spawn_box_size = clamp(spawn_box_size * 2,
                                            MIN_SPAWN_BOX_SIZE, MAX_SPAWN_BOX_SIZE);
    } else if (input.data.buttons_d & Input::BUTTON_Y) {
        spawn_box_size = clamp(spawn_box_size / 2,
                                            MIN_SPAWN_BOX_SIZE, MAX_SPAWN_BOX_SIZE);
    }

    if (input.data.buttons_d & Input::BUTTON_PLUS) {
        paused = !paused;
        OSReport("PAUSED: ", paused, "\n");
    }

    if (input.data.buttons_d & Input::BUTTON_R) {
        fill_cells = !fill_cells;
        OSReport("FILL: ", fill_cells, "\n");
    }

    if (input.data.buttons_d & Input::BUTTON_L) {
        color_cells = !color_cells;
        OSReport("COLOR: ", color_cells, "\n");
    }

    if (input.data.buttons_h & Input::BUTTON_ZR) {
        pointer_speed = clamp(pointer_speed + 1,
                                            MIN_POINTER_SPEED, MAX__POINTER_SPEED);
    }

    if (input.data.buttons_h & Input::BUTTON_ZL) {
        pointer_speed = clamp(pointer_speed - 1,
                                            MIN_POINTER_SPEED, MAX__POINTER_SPEED);
    }

    if (input.data.buttons_d & Input::BUTTON_UP) {
        int updated_step = clamp(step_length + STEP_INCREMENT,
                                        MIN_STEP_LENGTH, MAX_STEP_LENGTH);
        if (updated_step != step_length)
        {
            step_length = updated_step;
            OSReport("STEP: ", step_length, "\n");
        }
    }

    if (input.data.buttons_d & Input::BUTTON_DOWN) {
        int updated_step = clamp(step_length - STEP_INCREMENT,
                                        MIN_STEP_LENGTH, MAX_STEP_LENGTH);
        if (updated_step != step_length)
        {
            step_length = updated_step;
            OSReport("STEP: ", step_length, "\n");
        }
    }

    switch (event.type)
        {
        case SDL_QUIT:
            quit = true;
            break;

        default:
            break;
        }
}

//
// Called every frame for processing game state
//
void update()
{
    ticks = SDL_GetTicks64();
    bool updating = !paused && ticks >= delay;
    if (updating)
    {
        delay = ticks + step_length;
    }

    // Background
    render_set_color(main_renderer, COLOR_GRAY_DARK);
    SDL_RenderClear(main_renderer);

    // Cells
    render_set_color(main_renderer, COLOR_WHITE);
    for (int ii = 0; ii < game_state->CELLS_WIDE; ii++)
    {
        for (int jj = 0; jj < game_state->CELLS_HIGH; jj++)
        {
            // Render cells
            if (game_state->front_buffer[ii][jj] >= 1)
            {
                if (color_cells)
                {
                    int distance_from_mouse = clamp(
                        find_distance(x_pointer, y_pointer, ii * UNIT_SIZE, jj * UNIT_SIZE), 1, 255);
                    render_set_color(main_renderer,
                                     {255 - game_state->front_buffer[ii][jj],
                                      game_state->front_buffer[ii][jj],
                                      255 - distance_from_mouse,
                                      SDL_ALPHA_OPAQUE});
                }
                render_rectangle(main_renderer, ii * UNIT_SIZE, jj * UNIT_SIZE,
                                 UNIT_SIZE, UNIT_SIZE, fill_cells);
            }

            // Process cells
            if (updating)
            {
                process_cell(game_state, ii, jj, &population_size);
            }
        }
    }

    // Bring the processed cells to the front for rendering on the next call
    if (updating)
    {
        swap_buffers(game_state);
    }

    // Spawn area
    render_set_color(main_renderer, COLOR_GRAY_LIGHT);
    render_rectangle(main_renderer,
                     ((x_pointer / UNIT_SIZE) * UNIT_SIZE) - ((spawn_box_size) / 2 * UNIT_SIZE),
                     ((y_pointer / UNIT_SIZE) * UNIT_SIZE) - ((spawn_box_size) / 2 * UNIT_SIZE),
                     (spawn_box_size)*UNIT_SIZE,
                     (spawn_box_size)*UNIT_SIZE, false);

    // Commit render
    SDL_RenderPresent(main_renderer);
}

inline bool RunningFromMiiMaker() {
    return (OSGetTitleID() & 0xFFFFFFFFFFFFF0FFull) == 0x000500101004A000ull;
}

//
// Run demo
//
int main(int argc, char const *argv[])
{
    if (initialise() != EXIT_SUCCESS)
    {
        shutdown();
    }

    WHBProcInit();

    // Call AXInit to stop already playing sounds
    AXInit();
    AXQuit();

    KPADInit();
    WPADEnableURCC(TRUE);

    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3};

    delay = step_length;
    while (WHBProcIsRunning())
    {
        baseInput.reset();
        if (vpadInput.update(1280, 720)) {
            baseInput.combine(vpadInput);
        }
        for (auto &wpadInput : wpadInputs) {
            if (wpadInput.update(1280, 720)) {
                baseInput.combine(wpadInput);
            }
        }
        baseInput.process();

        input(baseInput);

        if (quit)
        {
           if (RunningFromMiiMaker()) {
                // Legacy way, just quit
                break;
            } else {
                // Launch menu otherwise
                SYSLaunchMenu();
            }
        }

        update();
    }

    shutdown();

    WHBProcShutdown();
    return EXIT_SUCCESS;
}
