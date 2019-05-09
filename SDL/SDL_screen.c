#include "../common.h"
#include "SDL_screen.h"

SDL_Surface *screen = NULL;
SDL_Surface *real_screen = NULL;
u32 last_scale_factor;
u8 *real_screen_pixels;

/* alekmaul's scaler taken from mame4all */
static void bitmap_scale(uint32_t startx, uint32_t starty, uint32_t viswidth, uint32_t visheight, uint32_t newwidth, uint32_t newheight,uint32_t pitchsrc,uint32_t pitchdest, uint16_t* restrict src, uint16_t* restrict dst)
{
    uint32_t W,H,ix,iy,x,y;
    x=startx<<16;
    y=starty<<16;
    W=newwidth;
    H=newheight;
    ix=(viswidth<<16)/W;
    iy=(visheight<<16)/H;

    do 
    {
        uint16_t* restrict buffer_mem=&src[(y>>16)*pitchsrc];
        W=newwidth; x=startx<<16;
        do 
        {
            *dst++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        dst+=pitchdest;
        y+=iy;
    } while (--H);
}

extern uint32_t ismenu, isloadfile;

void Set_Menu_resolution()
{
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0,0,0,0);
}

void Set_InGame_resolution()
{
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 240, 16, 0,0,0,0);
}

void update_screen()
{
	/*if(last_scale_factor != config.scale_factor)
	{
		set_screen_resolution(320, 240);
    }*/
    
	if (SDL_LockSurface(screen) == 0)
	{
		if (ismenu == 1)
		{
			if (isloadfile == 1) bitmap_scale(8, 4, 240, 160-16, real_screen->w, real_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
			else
			{
				bitmap_scale(48, 16, 240, 160, real_screen->w, real_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
			}
		}
		else
		{
				switch (config.scale_factor)
				{
					default:
						bitmap_scale(8, 0, vce.screen_width-8, 224-12, real_screen->w, real_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
					break;
					case 1:
						bitmap_scale(0, 0, vce.screen_width, 240, real_screen->w, real_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
					break;
					case 2:
						bitmap_scale(0, 20, vce.screen_width, 224-12, real_screen->w, real_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
					break;
					case 3:
						bitmap_scale(8, 48, 240, 160, real_screen->w, real_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
					break;
				}
		}
		SDL_UnlockSurface(screen);
	}
	SDL_Flip(real_screen);
}

void set_screen_resolution(u32 width, u32 height)
{
	u16 *old_pixels = NULL;

	if(screen != NULL)
    {
		old_pixels = malloc(320 * 240 * 2);
		copy_screen(old_pixels);
		SDL_FreeSurface(screen);
    }
    
	real_screen = SDL_SetVideoMode(240, 160, 16, SDL_HWSURFACE
	#ifdef SDL_TRIPLEBUF
	
	#endif
	);
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 240, 16, 0,0,0,0);
  
	if(old_pixels != NULL)
		blit_screen(old_pixels);
		
	if(old_pixels != NULL)
		free(old_pixels);

    last_scale_factor = config.scale_factor;
	SDL_WM_SetCaption("Temper PC-Engine Emulator", "Temper");
}

void *get_screen_ptr()
{
	return screen->pixels;
}

u32 get_screen_pitch()
{
	return (screen->pitch / 2);
}

void clear_screen()
{
	u32 i;
	u32 pitch = get_screen_pitch();
	u16 *pixels = get_screen_ptr();

	for(i = 0; i < screen->h; i++)
	{
		memset(pixels, 0, screen->w * 2);
		pixels += pitch;
	}
}

void clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
{
	u32 *dest = (u32 *)((u16 *)get_screen_ptr() + (line_number * get_screen_pitch()));
	u32 i;

	color |= (color << 16);

	edge /= 2;
	middle /= 2;

	for(i = 0; i < edge; i++)
	{
		*dest = color;
		dest++;
	}

	dest += middle;

	for(i = 0; i < edge; i++)
	{
		*dest = color;
		dest++;
	}
}

void set_single_buffer_mode()
{
}

void set_multi_buffer_mode()
{
}

void clear_all_buffers()
{
}
