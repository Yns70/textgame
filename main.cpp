/*

  This file is a demo of how to use the textgame library for terminal mode
  games.
  
 */
#include "textgame.h"
#include <stdio.h>

// Helper function to get rainbow pastel color based on value (8-18 range)
Color3 get_stat_color(int value) {
    // Normalize 8-18 to 0-1
    float t = clamp(float(value - 8) / 10.0f, 0.0f, 1.0f);
    // Map to hue: blue (2/3) for low values to red (0) for high values
    return hsv_to_color3((2.0f / 3.0f) * (1.0f - t), 0.5f, 1.0f);
}


int main(const int argc, const char* argv[]) {
    terminal_init();
    
    // Create the map. This could be different from the size of the terminal.
    Image map;
    image_resize(map, terminal_size());

    // Player position
    Vector2i player_pos(map.size.x / 2, map.size.y / 2);

    // Screen border
    for (int x = 1; x < map.size.x - 1; ++x) {        
        image_set(map, Vector2i(x, 0), Pixel(RED, U'═'), true);
        image_set(map, Vector2i(x, map.size.y - 1), Pixel(RED, U'═'), true);
    }
    
    for (int y = 1; y < map.size.y - 1; ++y) {
        image_set(map, Vector2i(0, y), Pixel(RED, U'║'), true);
        image_set(map, Vector2i(map.size.x - 1, y), Pixel(RED, U'║'), true);
        image_set(map, Vector2i(map.size.x - 15, y), Pixel(RED, U'│'), true);
    }
    
    image_set(map, Vector2i(0, 0), Pixel(RED, U'╔'), true);
    image_set(map, Vector2i(0, map.size.y - 1), Pixel(RED, U'╚'), true);
    
    image_set(map, Vector2i(map.size.x - 15, 0), Pixel(RED, U'╤'), true);
    image_set(map, Vector2i(map.size.x - 15, map.size.y - 1), Pixel(RED, U'╧'), true);

    image_set(map, Vector2i(map.size.x - 1, 0), Pixel(RED, U'╗'), true);
    image_set(map, Vector2i(map.size.x - 1, map.size.y - 1), Pixel(RED, U'╝'), true);

    // Random trees
    for (int i = 0; i < 100; ++i) {
        const int x = rand() % (map.size.x - 17) + 1;
        const int y = rand() % (map.size.y - 2) + 1;
        // Don't plant on top of the player
        if (x != player_pos.x || y != player_pos.y) {
            image_set(map, Vector2i(x, y), Pixel(GREEN, U'♣'), true);
        }
    }
    
    
    // Sprinkle gold coins
    const Color3 gold(1.0f, 0.8f, 0.0f);
    for (int i = 0; i < 10; ++i) {
        int coin_x, coin_y;
        do {
            coin_x = rand() % (map.size.x - 17) + 1;
            coin_y = rand() % (map.size.y - 2) + 1;
        } while (coin_x == player_pos.x && coin_y == player_pos.y);
        
        image_set(map, Vector2i(coin_x, coin_y), Pixel(gold, U'◎'), true);
    }
    
    // Add pink hearts
    for (int i = 0; i < 2; ++i) {
        int heart_x, heart_y;
        do {
            heart_x = rand() % (map.size.x - 17) + 1;
            heart_y = rand() % (map.size.y - 2) + 1;
        } while (heart_x == player_pos.x && heart_y == player_pos.y);
        
        image_set(map, Vector2i(heart_x, heart_y), Pixel(PINK, U'♥'), true);
    }

    // Draw river - random walk from top to bottom
    const Pixel river(Color3(0.0f, 0.6f, 1.0f), U'≈', BLUE);
    int river_x = (map.size.x - 15) / 4;
    
    for (int y = 1; y < map.size.y - 1; ++y) {
        // Random walk: -1 (left), 0 (straight), +1 (right)
        int direction = (rand() % 3) - 1;
        river_x += direction;
        
        // Keep river within bounds (away from borders and UI)
        river_x = std::max(1, std::min(map.size.x - 16, river_x));
        
        if (direction == 0) {
            // Moving straight down - draw one block
            image_set(map, Vector2i(river_x, y), river, true);
        } else {
            // Moving left or right - draw two blocks for 4-connectivity
            image_set(map, Vector2i(river_x, y), river, true);
            image_set(map, Vector2i(river_x - direction, y), river, true);
        }
    }

    // Faux UI - Character stats
    const Color3 golden_yellow(1.0f, 0.8f, 0.2f);
    
    // Define stat values and colors (higher = red end, lower = blue end)
    int str_val = 18, dex_val = 16, int_val = 8, wis_val = 12, con_val = 16, cha_val = 10;
    
    // Print each stat on its own line
    int ui_x = map.size.x - 13;
    image_print(map, Vector2i(ui_x, 1), U"STR:", golden_yellow);
    image_print(map, Vector2i(ui_x + 5, 1), format("%2d", str_val), get_stat_color(str_val));
    
    image_print(map, Vector2i(ui_x, 2), U"DEX:", golden_yellow);
    image_print(map, Vector2i(ui_x + 5, 2), format("%2d", dex_val), get_stat_color(dex_val));
    
    image_print(map, Vector2i(ui_x, 3), U"INT:", golden_yellow);
    image_print(map, Vector2i(ui_x + 5, 3), format("%2d", int_val), get_stat_color(int_val));
    
    image_print(map, Vector2i(ui_x, 4), U"WIS:", golden_yellow);
    image_print(map, Vector2i(ui_x + 5, 4), format("%2d", wis_val), get_stat_color(wis_val));
    
    image_print(map, Vector2i(ui_x, 5), U"CON:", golden_yellow);
    image_print(map, Vector2i(ui_x + 5, 5), format("%2d", con_val), get_stat_color(con_val));
    
    image_print(map, Vector2i(ui_x, 6), U"CHA:", golden_yellow);
    image_print(map, Vector2i(ui_x + 5, 6), format("%2d", cha_val), get_stat_color(cha_val));
    
    // Mini-map below stats
    const Color3 gray(0.5f, 0.5f, 0.5f);
    const Color3 map_red(1.0f, 0.3f, 0.3f);
    
    // Create a simple maze pattern (8x6 mini-map)
    // Using 'X' for walls and ' ' for spaces to avoid UTF-8 issues in pattern
    const String maze_pattern[] = {
        U"  X        ",
        U"  X   XXX↗ ",
        U"XXXX@ X    ",
        U"1   X X  X2",
        U"   XX?XXXX ",
        U"XX    X    "
    };
    
    int minimap_start_y = 10;
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 11; ++col) {
            const Character ch = maze_pattern[row][col];
            if (ch != U' ') {
                // Make one specific block red (player position on mini-map)
                Color3 block_color = (ch == '@') ? RED : GRAY;
                image_set(map, Vector2i(ui_x + col, minimap_start_y + row), Pixel(WHITE, ch == U'X' ? U' ' : ch, block_color), true);
            }
        }
    }
    
    image_print(map, Vector2i(map.size.x - 13, map.size.y - 2), U"ESC to Quit", WHITE * 0.5f);

    // Create framebuffer
    Image framebuffer;
    image_resize(framebuffer, terminal_size());

    // Main loop
    Key ch = KEY_NONE;
    
    // For debugging
    Key last_key = KEY_NONE;
    while (ch != KEY_ESCAPE) {
        ch = terminal_read_keyboard();

        if (ch) {
            last_key = ch;
        }

        Mouse mouse = terminal_read_mouse();
        
        // Handle player movement
        Vector2i new_pos = player_pos;
        if (ch == 'w' || ch == 'W' || ch == KEY_UP) {
            --new_pos.y;
        } else if (ch == 's' || ch == 'S' || ch == KEY_DOWN) {
            ++new_pos.y;
        } else if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
            --new_pos.x;
        } else if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
            ++new_pos.x;
        }
        
        // Check if new position is traversable
        Pixel target_pixel = image_get(map, new_pos);
        if (target_pixel.ch == u' ' || target_pixel.ch == river.ch) {
            player_pos = new_pos;
        }
        
        // Blit map to framebuffer
        image_blit(framebuffer, Vector2i(0, 0), map, Vector2i(0, 0), map.size, true);
        
        // Draw player
        image_set(framebuffer, player_pos, Pixel(RED, U'@'));

        // Draw debugging info
        image_print(framebuffer, framebuffer.size - Vector2i(14, 8),
                    format("Last Key: %3d\nMouse:  %2dx%2d\nButton:   %3x",
                           last_key, mouse.position.x, mouse.position.y,
                           mouse.button));
        
        // Show the framebuffer on the screen
        image_display(framebuffer);
        
        // 1/60 second
        sleep(1.0f / 60.0f);
    }

    terminal_cleanup();
    return 0;
}
