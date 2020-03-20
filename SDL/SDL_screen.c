#include "../common.h"
#include "SDL_screen.h"

SDL_Surface *screen = NULL;
SDL_Surface *real_screen = NULL;
u32 last_scale_factor;
u8 *real_screen_pixels;

#define RSHIFT32(X) (((X) & 0xF7DEF7DE) >>1)
//downscaling for ra-90
static void bitmap_scale_crop(uint32_t* restrict src, uint32_t* restrict dst)
{
    uint16_t y=4;
    uint32_t* __restrict__ buffer_mem;
 
    const uint16_t ix=1, iy=7;
    
    for(int H = 0; H < 160 / 5; H++)
    {
	    buffer_mem = &src[y*128*2];
        uint16_t x = 4;
        for(int W = 0; W < 120; W++) 
        {
            // Vertical Scaling( 7px to 5px)
            uint32_t a,b,c,d,e,f,g;
            a = RSHIFT32(buffer_mem[x]);
            b = RSHIFT32(buffer_mem[x+256]);
            c = RSHIFT32(buffer_mem[x+256*2]);
            d = RSHIFT32(buffer_mem[x+256*3]);
            e = RSHIFT32(buffer_mem[x+256*4]);
            f = RSHIFT32(buffer_mem[x+256*5]);
            g = RSHIFT32(buffer_mem[x+256*6]);          

            *dst =  a +  RSHIFT32(a + b);
	        *(dst+120) = b + c;
	        *(dst+120*2) = d + RSHIFT32(d + RSHIFT32(c + e));
	        *(dst+120*3) = e + f;
	        *(dst+120*4) = g + RSHIFT32(f + g);
            dst++;
            x += ix;
        }
        dst += 120*4;
        y += iy;
    }
}

#define RSHIFT(X) (((X) & 0xF7DE) >>1)
//downscaling for ra-90
static void bitmap_scale_256to240(uint16_t* restrict src, uint16_t* restrict dst)
{
    uint16_t y=8;  //crop top 8 pixels
    uint16_t* __restrict__ buffer_mem;
 
    const uint16_t ix=1, iy=7;
    
    for(int H = 0; H < 160 / 5; H++)
    {
	    buffer_mem = &src[y*256*2];
        uint16_t x = 0;
        for(int W = 0; W < 240; W++) 
        {
            //Vertical Scaling (7px to 5px)
            uint32_t a,b,c,d,e,f,g;
            a = RSHIFT(buffer_mem[x]);
            b = RSHIFT(buffer_mem[x+512]);
            c = RSHIFT(buffer_mem[x+512*2]);
            d = RSHIFT(buffer_mem[x+512*3]);
            e = RSHIFT(buffer_mem[x+512*4]);
            f = RSHIFT(buffer_mem[x+512*5]);
            g = RSHIFT(buffer_mem[x+512*6]);          

            if(W % 15 == 0){
                a = RSHIFT(a + RSHIFT(buffer_mem[x+1]));
                b = RSHIFT(b + RSHIFT(buffer_mem[x+512+1]));
                c = RSHIFT(c + RSHIFT(buffer_mem[x+512*2+1]));
                d = RSHIFT(d + RSHIFT(buffer_mem[x+512*3+1]));
                e = RSHIFT(e + RSHIFT(buffer_mem[x+512*4+1]));
                f = RSHIFT(f + RSHIFT(buffer_mem[x+512*5+1]));
                g = RSHIFT(g + RSHIFT(buffer_mem[x+512*6+1]));
                x += 1;
            }
            
            *dst =  a +  RSHIFT(a + b);
	        *(dst+240) = b + c;
	        *(dst+240*2) = d + RSHIFT(d + RSHIFT(c + e));
	        *(dst+240*3) = e + f;
	        *(dst+240*4) = g + RSHIFT(f + g);
            dst++;
            x += ix;
        }
        dst += 240*4;
        y += iy;
    }
}

void bitmap_scale_320to240(uint16_t* __restrict__ src, uint16_t* __restrict__ dst)
{
    uint16_t y=8;  //crop top 8 pixels
    uint16_t* __restrict__ buffer_mem;
    
    const uint16_t ix=1, iy=7;
    
    //Holizonal Scaling 
#define RMASK 0b1111100000000000
#define GMASK 0b0000011111100000
#define BMASK 0b0000000000011111

    for(int H = y; H < 224+y; H++){
        uint16_t* s1 = src+512*H;
        uint16_t* s2 = src+512*H;
        for(int W = 0; W <240/3; W++){
            int r[4],g[4],b[4],R0,G0,B0,R1,G1,B1,R2,G2,B2;
            for(int i = 0; i<4; i++){
                r[i] = *(s2+i) & RMASK;
                g[i] = *(s2+i) & GMASK;
                b[i] = *(s2+i) & BMASK;
            }
            R0 = r[0];
            G0 = g[0];
            B0 = ((b[0] + b[1])>>1) & BMASK;
            R1 = r[1];
            G1 = ((g[1] + g[2])>>1) & GMASK;
            B1 = b[2];
            R2 = ((r[2]>>1) + (r[3]>>1)) & RMASK;
            G2 = g[3];
            B2 = b[3];
            *(s1++) = R0 | G0 | B0;
            *(s1++) = R1 | G1 | B1;
            *(s1++) = R2 | G2 | B2;            
            s2+=4;
        }
    }
    
    //Vertical Scaling  (7px to 5px)
    for(int H = 0; H < 160 / 5; H++)
    {
	    buffer_mem = &src[y*512];
        uint16_t x = 0;
        for(int W = 0; W < 240; W++) 
        {
            uint16_t a,b,c,d,e,f,g;
            a = RSHIFT(buffer_mem[x]);
            b = RSHIFT(buffer_mem[x+512]);
            c = RSHIFT(buffer_mem[x+512*2]);
            d = RSHIFT(buffer_mem[x+512*3]);
            e = RSHIFT(buffer_mem[x+512*4]);
            f = RSHIFT(buffer_mem[x+512*5]);
            g = RSHIFT(buffer_mem[x+512*6]);          

            *dst =  a +  RSHIFT(a + b);
	        *(dst+240) = b + c;
	        *(dst+240*2) = d + RSHIFT(d + RSHIFT(c + e));
	        *(dst+240*3) = e + f;
	        *(dst+240*4) = g + RSHIFT(f + g);
            dst++;
            x += ix;
        }
        dst += 240*4;
        y += iy;
    }
}

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
					case 1: //Crop
                        bitmap_scale_crop((uint32_t* restrict)screen->pixels, (uint32_t* restrict)real_screen->pixels);
                        break;
					case 2:
						bitmap_scale_320to240((uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
                        break;
					case 3:
						bitmap_scale_256to240((uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
                        break;
					default: // auto
                        if(vce.screen_width == 256)
                                bitmap_scale_256to240((uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);
                        else
                                bitmap_scale_320to240((uint16_t* restrict)screen->pixels, (uint16_t* restrict)real_screen->pixels);

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
    
	real_screen = SDL_SetVideoMode(240, 160, 16, SDL_HWSURFACE);
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
