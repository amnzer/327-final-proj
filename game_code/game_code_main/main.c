#include <stdio.h>
#include "miniaudio.h"

#define MAX_ARTIST_LEN 16 // Name of artist
#define MAX_SONG_NAME_LEN 32 //"I "
#define MAX_PATH_LEN 48 // Use relative path to reduce
#define MAX_DIFFNAME_LEN 8 // tells you song difficulty
/*
Beet Saber Proof-of-Concept
*/
// 1. Define Key Objects
// a. The song class
// ../../oszs/charts/audio.ogg
struct Song { // note that, in the actual game, you wont have much room to print
  // You can trigger playback though?
  char artist[MAX_ARTIST_LEN];
  char song_name[MAX_SONG_NAME_LEN];
  char path_to_song[MAX_PATH_LEN];
  char diffname[MAX_DIFFNAME_LEN];
  //unsigned int song_length; // not obtainable from osu metadata, p sure...
  unsigned int preview_time; // time in ms when the song preview starts
}; // total: 108 Bytes per song object (just in metadata )
// for the actual song: up to 512 objects, each one 4 bytes (int + flags) => 2kB

struct Song song1_ETA = {
    .artist ="NewJeans",
    .song_name ="ETA",
    .path_to_song ="../../oszs/charts/ETA/audio.mp3",
    .diffname ="Normal",
    .preview_time = 49205
  };
struct Song song2_
// initialize all of these things
char artist[MAX_ARTIST_LEN];
char song_name[MAX_SONG_NAME_LEN];
char path_to_song[MAX_PATH_LEN];
char diffname[MAX_DIFFNAME_LEN];
unsigned int song_length;

// b. The game states

enum Game_States {
  GAME_START,
  SONG_SELECT,
  IN_GAME,
  REPORT_SCREEN,
  EXIT_GAME
};

enum Game_States game_state = GAME_START; // initialize game state
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
      printf("Enter e to continue! Or, enter x to exit...\n");
      while (1){ // while loop about responding to a specific button
        // get a key
        user_input = getchar();
        clear_input_buffer();
        //
        if(user_input == 'e'){
          game_state = SONG_SELECT;
          printf("Going to song select...\n\n");
          break;
        }
        else if (user_input == 'x'){
          game_state = EXIT_GAME;
          break;
        }
        else{
          printf("Not e, and not x...\n");
        }
      }
    }
    /* Game State 2:  Song Select
          User is given a carousel of songs to pick from. User can hover over any song
          User hears the song, starting at a highlighted section, and sees the name of the song.
    */
    
    if (game_state == SONG_SELECT){
      printf("Welcome to song select!\n");
      break;
    }

    // Game State 5: Exit Game
    if (game_state == EXIT_GAME){
      printf("Thanks for playing!\n\n");
      break;
    }
    else{
      printf("No mans land...\n\n");
      break;
    }
  }
  return 0;
}