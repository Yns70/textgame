#include "textgame.h"
#include <stdio.h>


int main(const int argc, const char* argv[]) {
    terminal_init();
    
    // Create the map. This could be different from the size of the terminal.
    Image map;
    image_resize(map, terminal_size());

    // Player position
    Vector2i player_pos(map.size.x / 2, map.size.y / 2);

    // Screen border
    for (int x = 1; x < map.size.x - 1; ++x) {        
        image_set(map, Vector2i(x, 0), Pixel(RED, u'═'));
        image_set(map, Vector2i(x, map.size.y - 1), Pixel(RED, u'═'));
    }
    
    for (int y = 1; y < map.size.y - 1; ++y) {
        image_set(map, Vector2i(0, y), Pixel(RED, u'║'));
        image_set(map, Vector2i(map.size.x - 1, y), Pixel(RED, u'║'));
        image_set(map, Vector2i(map.size.x - 15, y), Pixel(RED, u'│'));
    }
    image_set(map, Vector2i(0, 0), Pixel(RED, u'╔'));
    image_set(map, Vector2i(0, map.size.y - 1), Pixel(RED, u'╚'));
    
    image_set(map, Vector2i(map.size.x - 15, 0), Pixel(RED, u'╤'));
    image_set(map, Vector2i(map.size.x - 15, map.size.y - 1), Pixel(RED, u'╧'));

    image_set(map, Vector2i(map.size.x - 1, 0), Pixel(RED, u'╗'));
    image_set(map, Vector2i(map.size.x - 1, map.size.y - 1), Pixel(RED, u'╝'));

    // Random trees
    for (int i = 0; i < 100; ++i) {
        const int x = rand() % (map.size.x - 17) + 1;
        const int y = rand() % (map.size.y - 2) + 1;
        // Don't plant on top of the player
        if (x != player_pos.x || y != player_pos.y) {
            image_set(map, Vector2i(x, y), Pixel(GREEN, u'♣'));
        }
    }
    
    // Faux UI
    image_print(map, Vector2i(map.size.x - 14, 1), U"Score: 0");
    image_print(map, Vector2i(map.size.x - 14, map.size.y - 2), U"Press ESC to quit");

    // Create framebuffer
    Image framebuffer;
    image_resize(framebuffer, terminal_size());

    Key ch = KEY_NONE;
    while (ch != KEY_ESCAPE) {
        ch = terminal_key();
        
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
        
        // Check if new position is valid (space character)
        Pixel target_pixel = image_get(map, new_pos);
        if (target_pixel.ch == u' ') {
            player_pos = new_pos;
        }
        
        // Blit map to framebuffer
        image_blit(framebuffer, Vector2i(0, 0), map, Vector2i(0, 0), map.size, true);
        
        // Draw player
        image_set(framebuffer, player_pos, Pixel(YELLOW, u'@'));
        
        image_display(framebuffer);
        
        // 1/60 second
        sleep(1.0f / 60.0f);
    }

    terminal_cleanup();
    return 0;
}
