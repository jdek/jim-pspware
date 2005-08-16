/***************************************************************************
                                 Egg Chess!
                             -------------------
    begin                : Thu Apr 20 16:38:30 CEST 2000
    copyright            : (C) 2000-2001 by Anders Lindström
    email                : cal@swipnet.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// The playing board contains 3*3 boxes
// The chooser contains 3 boxes
//
// Numbering:
// +-+-+-+|+--+
// |0|1|2|||09|
// +-+-+-+|+--+
// |3|4|5|||10|
// +-+-+-+|+--+
// |6|7|8|||11|
// +-+-+-+|+--+
// Board   Chooser

#include <SDL/SDL.h>
#include "sge.h"
#include <stdio.h>
#include <stdlib.h>

const Sint32 side=100;
const Sint32 xcoords[]={0,side,2*side,3*side+5};
const Sint32 ycoords[]={0,side,2*side};

enum box_status{empty, shell, half, full};
box_status board[12]={};

enum _player{player1, player2};
_player player=player1;  //The current player

SDL_Surface *img_empty, *img_shell, *img_half, *img_full, *screen;

sge_bmpFont *font=NULL;

int selected=-1; //The selected box

int load_gfx(void);
void draw_box(int box_nr);
void draw_board(void);
void draw_lines(void);
void select(int box_nr);
void check_click(Sint32 x,Sint32 y);
void draw_chooser(void);
int test_win(void);
void msg(char *text);
void byebye(int exit_status);
void change_player(void);
void show_info(void);

//==================================================================================
// Main
//==================================================================================
int main(int argc, char *argv[])
{
  /* Init SDL */
	if ( SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
		exit(1);
	}

	/* Clean up on exit */
	atexit(SDL_Quit);

	/* Set window title */
	SDL_WM_SetCaption("Egg Chess", "EggChess");

	/* Initialize the display */
	screen = SDL_SetVideoMode(410, 330, 16, SDL_SWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}
	
	
	//=====================================
	
	
	/* Load all gfx */
	if(load_gfx()){
		fprintf(stderr, "Error: %s\n", SDL_GetError());
		exit(1);	
	}
	
	sge_ClearSurface(screen, 0,0,0);
	
	draw_lines();
	draw_board();
	SDL_UpdateRect(screen, 0,0,0,0);
	
	msg("Start the game, Player 1! Press [SPACE] for info.");
	
	//Wait for an event
	SDL_Event event;
	do{
		SDL_WaitEvent(&event);
		if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) break;
		if(event.type==SDL_QUIT) break;
		if(event.type==SDL_MOUSEBUTTONDOWN) 
			check_click(event.button.x,event.button.y);
		if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_SPACE) 
			show_info();
	}while(true);
	
	byebye(0);
	
  return 0;
}


//==================================================================================
// Load gfx
//==================================================================================
int load_gfx(void)
{
	SDL_Surface *tmp;

	tmp=SDL_LoadBMP("gfx/empty.bmp");
	if(!tmp){SDL_SetError("Couldn't load gfx/empty.bmp"); return -1;}
	img_empty=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	
	tmp=SDL_LoadBMP("gfx/shell.bmp");
	if(!tmp){SDL_SetError("Couldn't load gfx/shell.bmp"); return -1;}
	img_shell=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	
	tmp=SDL_LoadBMP("gfx/half.bmp");
	if(!tmp){SDL_SetError("Couldn't load gfx/half.bmp"); return -1;}
	img_half=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	
	tmp=SDL_LoadBMP("gfx/full.bmp");
	if(!tmp){SDL_SetError("Couldn't load gfx/full.bmp"); return -1;}
	img_full=SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	
	return 0;
}


//==================================================================================
// Draw a box
//==================================================================================
void draw_box(int box_nr)
{
	if(box_nr>11 || box_nr<0)
		return;
	
	
	SDL_Surface *tmp=NULL;
	Sint32 x,y;

	switch(board[box_nr]){
		case(empty):{
			tmp=img_empty;
		}
		break;
		case(shell):{
			tmp=img_shell;		
		}
		break;
		case(half):{
			tmp=img_half;	
		}
		break;
		case(full):{
			tmp=img_full;
		}
		break;
	}
	
	if(box_nr<9){
		x=xcoords[box_nr%3];
		y=ycoords[box_nr/3];
	}
	else{
		x=xcoords[3];
		y=ycoords[box_nr-9];
	}

	sge_Update_OFF();
	sge_FilledRect(screen, x+1,y+1, x+side-1, y+side-1, 0,0,0);
	sge_Update_ON();	
	sge_Blit(tmp,screen, 0,0, x+(side/2)-(tmp->w/2), y+(side/2)-(tmp->h/2), tmp->w,tmp->h);
}


//==================================================================================
// Init and redraw the board
//==================================================================================
void draw_board(void){

	for(int i=0; i<12; i++){
		board[i]=empty;
		draw_box(i);
	}
}


//==================================================================================
// Draw the lines
//==================================================================================
void draw_lines(void)
{
	for(int i=1; i<3; i++){
		sge_HLine(screen, xcoords[0]+5, xcoords[2]+side-5, ycoords[i], 60,60,80);
		sge_HLine(screen, xcoords[3]+5, xcoords[3]+side-5, ycoords[i], 60,60,80);
	}
	
	for(int i=1; i<3; i++)
		sge_VLine(screen, xcoords[i], ycoords[0]+5, ycoords[2]+side-5, 60,60,80);
		
	sge_VLine(screen, xcoords[3], ycoords[0]+5, ycoords[2]+side-5, 220,220,240);
	sge_HLine(screen, xcoords[0]+5, xcoords[3]+side-5,ycoords[2]+side, 220,220,240);	
	
}


//==================================================================================
// Select a box
//==================================================================================
void select(int box_nr)
{
	if(box_nr>8 || box_nr<0)
		return;	
	
	selected=box_nr;	
		
	
	/* Draw a rectange around the box */
	
	Sint32 x=xcoords[box_nr%3];
	Sint32 y=ycoords[box_nr/3];
	
	sge_VLine(screen, x+side-5, y+5, y+side-5, 130,130,130);
	sge_VLine(screen, x+side-6, y+6, y+side-6, 80,80,80);
	sge_VLine(screen, x+side-7, y+7, y+side-7, 30,30,30);
	
	sge_HLine(screen, x+5, x+side-5, y+5, 30,30,30);
	sge_HLine(screen, x+6, x+side-6, y+6, 80,80,80);
	sge_HLine(screen, x+7, x+side-7, y+7, 130,130,130);
	
	sge_HLine(screen, x+5, x+side-5, y+side-5, 130,130,130);
	sge_HLine(screen, x+6, x+side-6, y+side-6, 80,80,80);
	sge_HLine(screen, x+7, x+side-7, y+side-7, 30,30,30);

	sge_VLine(screen, x+5, y+5, y+side-5, 30,30,30);
	sge_VLine(screen, x+6, y+6, y+side-5, 80,80,80);
	sge_VLine(screen, x+7, y+7, y+side-5, 130,130,130);
}


//==================================================================================
// Check LMB click
//==================================================================================
void check_click(Sint32 x,Sint32 y)
{
	int row=-1;
	int line=-1;
	int box;
	
	/* Check x axis */
	for(int i=0; i<4; i++){
		if(x>=xcoords[i] && x<xcoords[i]+side){
			row=i;
			break;	
		}
	}
	
	/* Check y axis */
	for(int i=0; i<3; i++){
		if(y>=ycoords[i] && y<ycoords[i]+side){
			line=i;
			break;	
		}
	}
	
	if(row==-1 || line==-1)
		return;
	
	/* Calculate box number */
	if(row<3)
		box=line*3+row;
	else
		box=9+line;
	
	if(selected==box)
		return;
			
	/* Update previous box */
	if(selected>=0 && box<9){
		draw_box(selected);
		sge_UpdateRect(screen,xcoords[selected%3],ycoords[selected/3],side,side);
	}
	
	if(box<9){ /* User selected an egg on the board */
		if(board[box]!=board[selected] || selected==-1){
			select(box);
			draw_chooser();
		}
		else	
			select(box);
	}
	else{ /* User selected an egg on the chooser */
		switch(board[box]){
			case(empty):
				return;
				break;
			case(shell):
				board[selected]=shell;
				break;
			case(half):
				board[selected]=half;
				break;
			case(full):
				board[selected]=full;
				break;
		}
		
		draw_box(selected);
		sge_UpdateRect(screen,xcoords[selected%3],ycoords[selected/3],side,side);
		selected=-1;
		board[9]=empty; board[10]=empty; board[11]=empty;	
		draw_box(9); draw_box(10); draw_box(11);
		sge_UpdateRect(screen,xcoords[3],ycoords[0],side,side*3);
		change_player();
		
		
		if(test_win()){  //Do we have a winner?
			if(player==player1)
				msg("Player 2 Wins!");
			else
				msg("Player 1 Wins!");	
			SDL_Delay(2000);
			sge_Update_OFF();
			draw_board();
			selected=-1;
			msg("Start the game, Player 1! Press [SPACE] for info.");
			player=player1;
			sge_Update_ON();
			SDL_UpdateRect(screen, 0,0,0,0);
		}
	}
		
}


//==================================================================================
// Lets the user choose an egg
//==================================================================================
void draw_chooser(void)
{
	switch(board[selected]){
		case(empty):
			board[9]=shell;
			board[10]=half;
			board[11]=full;	
			break;
		case(full):
    		board[9]=shell;
			board[10]=half;
			board[11]=empty;	
			break;
		case(half):
    		board[9]=shell;
			board[10]=empty;
			board[11]=empty;	
			break;
		case(shell):
    		board[9]=empty;
			board[10]=empty;
			board[11]=empty;	
			break;
	}
	
	draw_box(9);
	draw_box(10);
	draw_box(11);
	
	sge_UpdateRect(screen,xcoords[3],ycoords[0],side,side*3);	
}


//==================================================================================
// Tests win condition
//==================================================================================
int test_win(void)
{
	/* Check lines */
	for(int i=0; i<3; i++)
		if( (board[i*3] != empty) && (board[i*3] == board[i*3+1]) && (board[i*3+1] == board[i*3+2]) )
			return 1;
	
	/* Check rows */
	for(int i=0; i<3; i++)
		if( (board[i] != empty) && (board[i] == board[i+3]) && (board[i+3] == board[i+6]) )
			return 1;
	
	/* Check crosses */
	if( (board[0] != empty) && (board[0] == board[4]) && (board[4] == board[8]) )
		return 1;
	if( (board[2] != empty) && (board[2] == board[4]) && (board[4] == board[6]) )
		return 1;	
	
	return 0;	
}


//==================================================================================
// Displays the text
//==================================================================================
void msg(char *text)
{
	/* Open font */
	if(!font){
		if( !(font=sge_BF_OpenFont("gfx/font.bmp", SGE_BFTRANSP)) ){
			fprintf(stderr, "Error: %s\n", SDL_GetError());
			byebye(1);
		}
	}
	
	sge_Update_OFF();
	sge_FilledRect(screen, 0,ycoords[2]+side+1 ,screen->w-1,screen->h-1, 0,0,0);
	sge_BF_textout(screen, font, text, xcoords[0]+7, ycoords[2]+side+10);	
	sge_Update_ON();
	sge_UpdateRect(screen,0,ycoords[2]+side+1,screen->w, screen->h-(ycoords[2]+side+2));
}


//==================================================================================
// Clean up and exit
//==================================================================================
void byebye(int exit_status)
{
	SDL_FreeSurface(img_empty);
	SDL_FreeSurface(img_shell);
	SDL_FreeSurface(img_half);
	SDL_FreeSurface(img_full);
	
	if(font)
		sge_BF_CloseFont(font);
		
	exit(exit_status);
}


//==================================================================================
// Change player
//==================================================================================
void change_player(void)
{
	if(player==player1){
		player=player2;
		msg("Player 2");
	}
	else{
		player=player1;
		msg("Player 1");
	}
}


//==================================================================================
// Show some info
//==================================================================================
void show_info(void)
{
	const int ld=10;
	Sint32 y=5;
	SDL_Surface *buffer=SDL_DisplayFormat(screen),*tmp,*egg=NULL;
	
	tmp=SDL_LoadBMP("gfx/big.bmp");
	if(tmp){
		egg=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
		sge_ClearSurface(screen, 0,0,0);
		SDL_SetAlpha(egg,SDL_SRCALPHA, 100);
		sge_Blit(egg, screen, 0,0, screen->w/2-egg->w/2, screen->h/2-egg->h/2, egg->w, egg->h);
	}
	else
		sge_ClearSurface(screen, 0,0,0);	
	
	sge_Update_OFF();
	sge_BF_textout(screen, font, "Egg Chess v1.1 (010910) by Anders Lindstrom", 5, y+=ld);
	sge_BF_textout(screen, font, "==================================================================", 5, y+=ld);
	sge_BF_textout(screen, font, "This is free software (GNU General Public License)", 5, y+=ld);
	y+=ld;
	sge_BF_textout(screen, font, "Rules:", 5, y+=ld);
	sge_BF_textout(screen, font, "Player 1 places an egg - whole, half eaten or empty - in one of", 5, y+=ld);
	sge_BF_textout(screen, font, "the nine nests. Next, player 2 can place a new egg (or part", 5, y+=ld);
	sge_BF_textout(screen, font, "thereof), eat a part of the existing egg or eat the whole egg.", 5, y+=ld);
	sge_BF_textout(screen, font, "Next, player 1 does the same.", 5, y+=ld);
	sge_BF_textout(screen, font, "The winner is the player who gets three whole, half eaten or empty", 5, y+=ld);
	sge_BF_textout(screen, font, "eggs in a row.", 5, y+=ld);
	y+=2*ld;
	sge_BF_textout(screen, font, "Rules invented by Stefan Sandstrom.", 5, y+=ld);
	sge_BF_textout(screen, font, "Graphics borrowed from 'MacAggaschack'.", 5, y+=ld);
	y+=2*ld;
	sge_BF_textout(screen, font, "This game uses the Simple DirectMedia Layer (SDL) by Sam Lantinga:", 5, y+=ld);
	sge_BF_textout(screen, font, "http://www.libsdl.org/", 5, y+=ld);
	sge_BF_textout(screen, font, "and the SGE library by me:", 5, y+=ld);
	sge_BF_textout(screen, font, "http://www.etek.chalmers.se/~e8cal1/sge/index.html", 5, y+=ld);
	sge_Update_ON();
	
	SDL_UpdateRect(screen, 0,0,0,0);
	
	//Wait for an event
	SDL_Event event;
	do{
		SDL_WaitEvent(&event);
		if(event.type==SDL_KEYDOWN) break;
		if(event.type==SDL_QUIT) break;
		if(event.type==SDL_MOUSEBUTTONDOWN) break;
	}while(true);
	
	sge_Blit(buffer,screen,0,0,0,0, screen->w, screen->h);
	SDL_UpdateRect(screen, 0,0,0,0);
	SDL_FreeSurface(buffer);
	
	if(egg)
		SDL_FreeSurface(egg);
}

