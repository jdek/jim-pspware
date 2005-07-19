/***************************************************************************
                          help.c  -  description
                             -------------------
    begin                : Sat Dec 15 2001
    copyright            : (C) 2001 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lbreakout.h"
#include "event.h"

StkFont *help_caption_font = 0;
StkFont *help_font = 0;
SDL_Surface *help_bkgnd = 0;
int side_count = 4;

extern SDL_Surface *stk_display;
extern SDL_Surface *extra_pic;
extern SDL_Surface *brick_pic;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Draw title.
====================================================================
*/
void draw_title( )
{
	help_caption_font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, 
        stk_display, stk_display->w / 2, 20, STK_OPAQUE, "Quick Help" );
}
/*
====================================================================
Add footnote.
====================================================================
*/
void draw_footnote( int side )
{
	char buf[256];
	help_font->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_BOTTOM;
	sprintf( buf, "%i / %i", side, side_count );
	stk_font_write( help_font, stk_display, 
        stk_display->w - 2, stk_display->h - 2, STK_OPAQUE, buf );
	help_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_BOTTOM;
	sprintf( buf, "<ESCAPE>: Quit  <LEFT BUTTON>: Next Page  <RIGHT BUTTON>: Previous Page" );
	stk_font_write( help_font, 
        stk_display, 2, stk_display->h - 2, STK_OPAQUE, buf );
}

/*
====================================================================
Draw bonus info
====================================================================
*/
void draw_bonus_info( int x, int y, int id, char *text )
{
    stk_surface_blit( 
        extra_pic, id * BRICK_WIDTH, 0, BRICK_WIDTH, BRICK_HEIGHT, 
        stk_display, x, y );
	help_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_CENTER_Y;
	stk_font_write( help_font, stk_display, 
        x + BRICK_WIDTH + 10, y + BRICK_HEIGHT / 2, STK_OPAQUE, text );
}

/*
====================================================================
Draw brick info
====================================================================
*/
void draw_brick_info( int x, int y, int id, char *text )
{
    stk_surface_blit( 
        brick_pic, id * BRICK_WIDTH, 0, BRICK_WIDTH, BRICK_HEIGHT, 
        stk_display, x, y );
	help_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_CENTER_Y;
	stk_font_write( help_font, stk_display, 
        x + BRICK_WIDTH + 10, y + BRICK_HEIGHT / 2, STK_OPAQUE, text );
}

/*
====================================================================
Draw bonus info screen.
====================================================================
*/
void draw_bonus_screen()
{
	int bonus_x = 20, bonus_y = 80, bonus_w = 200, bonus_h = 30;
	int malus_x = 20, malus_y = 330, malus_w = 200, malus_h = 30;
	
    stk_surface_blit( help_bkgnd, 0,0,-1,-1, stk_display, 0,0 );
	draw_title();
	
	/* bonuses */
	help_caption_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, 
        stk_display, bonus_x, bonus_y - 30, STK_OPAQUE, "Bonuses:" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 0, 8,  "Expand paddle" );
	draw_bonus_info( bonus_x + bonus_w * 1, bonus_y + bonus_h * 0, 9,  "Extra life" );
	draw_bonus_info( bonus_x + bonus_w * 2, bonus_y + bonus_h * 0, 10, "Sticky paddle" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 1, 15, "Plasma weapon" );
	draw_bonus_info( bonus_x + bonus_w * 1, bonus_y + bonus_h * 1, 2,  "200 - 10,000 points extra score" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 2, 12, "Extra ball" );
	draw_bonus_info( bonus_x + bonus_w * 1, bonus_y + bonus_h * 2, 11, "Energy balls (penetrate bricks)" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 3, 13, "Bonus floor" );
	draw_bonus_info( bonus_x + bonus_w * 1, bonus_y + bonus_h * 3, 18, "Deccelerate balls to minimum speed" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 4, 6,  "1,000 points extra score from bricks with no bonus" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 5, 19, "Instantly collect all bonuses and destroy all maluses" );
	draw_bonus_info( bonus_x + bonus_w * 0, bonus_y + bonus_h * 6, 25, "Explosive balls" );
	draw_bonus_info( bonus_x + bonus_w * 1, bonus_y + bonus_h * 6, 26, "Paddle attracts bonuses" );
    
	/* maluses */
	stk_font_write( help_caption_font, stk_display, 
        malus_x, malus_y - 30, STK_OPAQUE, "Maluses:" );
	draw_bonus_info( malus_x + malus_w * 0, malus_y + malus_h * 0, 7,  "Shrink paddle" );
	draw_bonus_info( malus_x + malus_w * 0, malus_y + malus_h * 3, 17,  "Accelerate balls" );
	draw_bonus_info( malus_x + malus_w * 0, malus_y + malus_h * 1, 14,  "Freeze paddle" );
	draw_bonus_info( malus_x + malus_w * 1, malus_y + malus_h * 1, 21,  "Random ball reflection at bricks" );
	draw_bonus_info( malus_x + malus_w * 0, malus_y + malus_h * 2, 20,  "Darkness" );
	draw_bonus_info( malus_x + malus_w * 1, malus_y + malus_h * 2, 27,  "Paddle attracts maluses" );
	draw_bonus_info( malus_x + malus_w * 1, malus_y + malus_h * 0, 22,  "Paddle disappears when not moving" );
	draw_bonus_info( malus_x + malus_w * 1, malus_y + malus_h * 3, 28,  "40% chance that a ball doesn't damage brick" );
	
	draw_footnote( 1 );
	
    stk_display_update( STK_UPDATE_ALL );
}
/*
====================================================================
Draw hint
====================================================================
*/
void draw_hint( int x, int y, char *text )
{
	stk_font_write( help_font, stk_display, 
        x, y, STK_OPAQUE, text );
}
/*
====================================================================
Draw hints
====================================================================
*/
void draw_hints_screen()
{
	int hint_x = 20, hint_y = 80, hint_h = 20;
	
    stk_surface_blit( help_bkgnd, 0,0,-1,-1, stk_display, 0,0 );
	draw_title();

	help_caption_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, stk_display, 
        hint_x, hint_y - 40, STK_OPAQUE, "Hints:" );
	
	draw_hint( hint_x, hint_y + hint_h * 0, "<<'In Game'-Keys>>" );
	draw_hint( hint_x, hint_y + hint_h * 1, " p      Pause game. (and enter chatroom in network game)" );
	draw_hint( hint_x, hint_y + hint_h * 2, " s      Enable/Disable sound." );
	draw_hint( hint_x, hint_y + hint_h * 3, " a      Change animation level (off/low/high)." );
	draw_hint( hint_x, hint_y + hint_h * 4, " f      Switch fullscreen/windowed mode." );
	draw_hint( hint_x, hint_y + hint_h * 5, "        NOTE: Changing resolution takes a while so this is done best" );
	draw_hint( hint_x, hint_y + hint_h * 6, "        when game's paused." );
	draw_hint( hint_x, hint_y + hint_h * 7, " r      Restart level." );
	draw_hint( hint_x, hint_y + hint_h * 8, " d      Disintegrate single bricks. (AddOn's only)" );
	draw_hint( hint_x, hint_y + hint_h * 9, " w      Warp to next level after enough bricks where cleared. (AddOn's only)" );
	draw_hint( hint_x, hint_y + hint_h * 10, " Shift  Shows highest score of set instead of your score as long as you" );
	draw_hint( hint_x, hint_y + hint_h * 11, "        hold it down." );
	draw_hint( hint_x, hint_y + hint_h * 12, " Tab    Take a screenshot." );
	draw_hint( hint_x, hint_y + hint_h * 13, " Esc    Quit game." );

	draw_hint( hint_x, hint_y + hint_h * 15, "Pressing the left or right mouse button will fire attached balls either" );
	draw_hint( hint_x, hint_y + hint_h * 16, "to the left or right direction if 'Ball Fire Angle' in 'Advanced Options'" );
	draw_hint( hint_x, hint_y + hint_h * 17, "is not set to 'Random'." );
	
	draw_footnote( 3 );
	
    stk_display_update( STK_UPDATE_ALL );
}
/*
====================================================================
Draw ingame hints
====================================================================
*/
void draw_ingame_hints_screen() 
{
    int brick_x = 20, brick_y = 210, brick_h = 30;
    int extra_x = 20, extra_y = 80, extra_h = 30;
	
    stk_surface_blit( help_bkgnd, 0,0,-1,-1, stk_display, 0,0 );
	draw_title();

	help_caption_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, stk_display, 
        extra_x, extra_y - 30, STK_OPAQUE, "Neutral Power-Ups:" );

    draw_bonus_info( extra_x, extra_y + extra_h * 0, 16, "Any of the listed bonuses/maluses." );
    draw_bonus_info( extra_x, extra_y + extra_h * 1, 23, "Resets all active bonuses and maluses." );
    draw_bonus_info( extra_x, extra_y + extra_h * 2, 24, "Adds 7 seconds to all active bonuses/maluses." );
    
	help_caption_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, stk_display, 
        brick_x, brick_y - 30, STK_OPAQUE, "Special Bricks:" );

    draw_brick_info( brick_x, brick_y + brick_h * 0, 0, "Indestructible." );
    draw_brick_info( brick_x, brick_y + brick_h * 1, 1, "May only be destroyed by energy ball else it's indestructible." );
    draw_brick_info( brick_x, brick_y + brick_h * 2, 2, "As above and balls are reflected randomly at this brick." );
    draw_brick_info( brick_x, brick_y + brick_h * 3, 5, "Needs three hits to be destroyed." );
    draw_brick_info( brick_x, brick_y + brick_h * 4, 9, "As above and regenerates durability every 4 seconds." );
    draw_brick_info( brick_x, brick_y + brick_h * 5, 18, "Explodes and destroys all nearby bricks." );
    draw_brick_info( brick_x, brick_y + brick_h * 6, 19, "Creates up to 8 bricks on destruction." );

    draw_footnote( 2 );
	
    stk_display_update( STK_UPDATE_ALL );
}
/*
====================================================================
Draw trouble shooting
====================================================================
*/
void draw_trouble( int x, int y, char *text )
{
	draw_hint( x, y, text );
}
void draw_trouble_screen()
{
	int trouble_x = 20, trouble_y = 90, trouble_h = 20;
	int manual_x = 20, manual_y = 310, manual_h = 20;
    stk_surface_blit( help_bkgnd, 0,0,-1,-1, stk_display, 0,0 );
	draw_title();

	help_caption_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, stk_display, 
        trouble_x, trouble_y - 40, STK_OPAQUE, "Troubleshooting:" );
	
	draw_trouble( trouble_x, trouble_y + trouble_h * 0, "* In fullscreen mode the window keeps it size just adding a black frame?" );
	draw_trouble( trouble_x, trouble_y + trouble_h * 1, "  - Maybe you do not have 640x480 as resolution available? Check your" );
	draw_trouble( trouble_x, trouble_y + trouble_h * 2, "    X configuration." );
	draw_trouble( trouble_x, trouble_y + trouble_h * 3, "* Sounds seem to be out of sync and are played with some delay?" );
	draw_trouble( trouble_x, trouble_y + trouble_h * 4, "  - Set SDL_AUDIODRIVER to dma (export SDL_AUDIODRIVER=dma). If this results" );
	draw_trouble( trouble_x, trouble_y + trouble_h * 5, "    in a lot of errors killing artsd (or esd) may help." );
	draw_trouble( trouble_x, trouble_y + trouble_h * 6, "* LBreakout2 gets mute while playing when switching on/off sounds?" );
	draw_trouble( trouble_x, trouble_y + trouble_h * 7, "  - SDL_mixer seems to mute active channels. You shouldn't enable/disable" );
	draw_trouble( trouble_x, trouble_y + trouble_h * 8, "    sounds to often as you'll propably loose all channels then." );
	
	help_caption_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
	stk_font_write( help_caption_font, stk_display, 
        manual_x, manual_y - 40, STK_OPAQUE , "Manual:");
	draw_trouble( manual_x, manual_y + manual_h * 0, "This is just a quick help with the most important facts about LBreakout2." );
	draw_trouble( manual_x, manual_y + manual_h * 1, "If you want more and better information check out the manual installed to" );
	draw_trouble( manual_x, manual_y + manual_h * 2, "/usr/doc/lbreakout2 or the online version at http://lgames.sf.net." );
	draw_trouble( manual_x, manual_y + manual_h * 3, "And if you have questions (not answered by the manual) or you found a bug" );
	draw_trouble( manual_x, manual_y + manual_h * 4, "or you just want to drop a general note about LBreakout2 just mail to:" );
	draw_trouble( manual_x, manual_y + manual_h * 5, "  kulkanie@gmx.net" );
	draw_trouble( manual_x, manual_y + manual_h * 6, "                         Enjoy the game!" );
	draw_trouble( manual_x, manual_y + manual_h * 6 + 10, "                                                 Michael Speck" );
	
	draw_footnote( 4 );
	
    stk_display_update( STK_UPDATE_ALL );
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Load/delete help resources.
====================================================================
*/
void help_create()
{
    help_font = stk_font_load( SDL_SWSURFACE, "f_small_yellow.png" );
    help_caption_font = stk_font_load( SDL_SWSURFACE, "f_yellow.png" );
	/* background -- will be filled when running help */
	help_bkgnd = stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
    SDL_SetColorKey( help_bkgnd, 0, 0 );
}
void help_delete()
{
    stk_font_free( &help_font );
    stk_font_free( &help_caption_font );
    stk_surface_free( &help_bkgnd );
}

/*
====================================================================
Run help.
====================================================================
*/
void help_run()
{
	int leave = 0;
	SDL_Event event;
	int cur_side = 0;
    SDL_Surface *buffer = 
        stk_surface_create(SDL_SWSURFACE,stk_display->w, stk_display->h);
    
	/* buffer screen */
    stk_surface_blit( stk_display, 0,0,-1,-1, buffer, 0,0 );
	SDL_SetColorKey(buffer, 0, 0);
	
    /* gray screen and use as background */
    stk_surface_gray( stk_display, 0,0,-1,-1, 1 );
    stk_surface_blit( stk_display, 0,0,-1,-1, help_bkgnd, 0,0 );
		
	draw_bonus_screen();
	while ( !leave ) {
		SDL_WaitEvent( &event );
		switch ( event.type ) {
			case SDL_KEYDOWN:
				switch ( event.key.keysym.sym ) {
					case SDLK_ESCAPE: leave = 1; break;
					case SDLK_LEFT:
					case SDLK_RIGHT:
						if ( event.key.keysym.sym == SDLK_RIGHT )  {
							cur_side++;
							if ( cur_side == side_count ) cur_side = 0;
						}	
						else {
							cur_side--;
							if ( cur_side < 0 ) cur_side = side_count - 1;
						}
						switch ( cur_side ) {
							case 0: draw_bonus_screen(); break;
							case 2: draw_hints_screen(); break;
							case 1: draw_ingame_hints_screen(); break;
							case 3: draw_trouble_screen(); break;
						}
						break;
					default: break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if ( event.button.button == STK_BUTTON_LEFT )  {
					cur_side++;
					if ( cur_side == side_count ) cur_side = 0;
				}	
				else {
					cur_side--;
					if ( cur_side < 0 ) cur_side = side_count - 1;
				}
				switch ( cur_side ) {
					case 0: draw_bonus_screen(); break;
					case 2: draw_hints_screen(); break;
					case 1: draw_ingame_hints_screen(); break;
					case 3: draw_trouble_screen(); break;
				}
				break;
			default: break;
		}
	}
	
	/* redraw screen */
    stk_surface_blit( buffer, 0,0,-1,-1, stk_display, 0,0 );
    stk_display_update( STK_UPDATE_ALL );
	SDL_FreeSurface( buffer );
	
    /* reset the relative position so paddle wont jump */
    SDL_GetRelativeMouseState(0,0);

}
