#include <stdio.h>
#include "miniaudio.h"

#define MAX_ARTIST_LEN 16 // Name of artist
#define MAX_SONG_NAME_LEN 32 //"I "
#define MAX_PATH_LEN 128 // this is a lot, like a lot, of storage
#define MAX_DIFFNAME_LEN 8 
/*
Beet Saber Proof-of-Concept
*/
// 1. Define Key Objects
// a. The song class

struct Song { // note that, in the actual game, you wont have much room to print
  // You can trigger playback though?
  char artist[MAX_ARTIST_LEN];
  char song_name[MAX_SONG_NAME_LEN];
  char path_to_song[MAX_PATH_LEN];
  char diffname[MAX_DIFFNAME_LEN];
  unsigned int song_length; // song must be under 72 hours long.
}; // total: 184 

// b. The game states

enum Game_States {
  GAME_START,
  SONG_SELECT,
  IN_GAME,
  REPORT_SCREEN,
  EXIT_GAME
};
enum Game_States game_state = GAME_START;
// c. Special Strings
// Welcome message. Sources: 
// https://patorjk.com/software/taag/
// https://tomeko.net/online_tools/cpp_text_escape.php?
char welcome_msg[] = "  _      __    __                     __         ___          __    ____     __             __\n | | /| / /__ / /______  __ _  ___   / /____    / _ )___ ___ / /_  / __/__ _/ /  ___ ____  / /\n | |/ |/ / -_) / __/ _ \\/  ' \\/ -_) / __/ _ \\  / _  / -_) -_) __/ _\\ \\/ _ `/ _ \\/ -_) __/ /_/ \n |__/|__/\\__/_/\\__/\\___/_/_/_/\\__/  \\__/\\___/ /____/\\__/\\__/\\__/ /___/\\_,_/_.__/\\__/_/   (_) \n\n";

// d. Misc Variables
char user_input = '\0'; // tracks user input
// e. Helper functions for strings
void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n') { } // read until you hit the end
}
// The Game
int main(){
  while (1){
  // Phase 1: Game start
  if (game_state == GAME_START){
    printf("%s",welcome_msg);
    printf("Press e + Enter to continue!\n");
    while ((user_input = getchar())!='e'){ // respond to specific button
      if(user_input != '\n'){ // if you press char, then enter, newline will process the char and \n. 
        printf("Not e...\n");
      }
      clear_input_buffer();
    }
  }
  if (game_state == SONG_SELECT){
    printf("Welcome to song select!\n");
  }
  break;
  }
  return 0;
}