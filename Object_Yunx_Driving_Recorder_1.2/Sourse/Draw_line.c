#include "camera.h"

void Draw_line(void)
 {
    int x, y, i = 0;
    for(y = 479; y >= 240; y--)
    {
        for(x = i; x < 39 + i; x++)
        {
            if(y > 410)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(410 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(320 >= y && y >= 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }
        for(x = 799 - i; x > 799 - i - 30; x--)
        {
            if(y > 410)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(410 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(320 >= y && y > 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }
        for(x = i; x < 80 + i; x++)
        {
            if(410 >= y && y >= 380)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(350 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(260 >= y && y >= 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }    
        for(x = 799 - i; x > 799 - i - 80; x--)    
        {
            if(410 >= y && y >= 380)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(350 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(260 >= y && y >= 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }
        i++;
    }
 }
