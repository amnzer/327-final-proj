#include <stdio.h>
#include "miniaudio.h"

#define MAX_ARTIST_LEN 16 // Name of artist
#define MAX_SONG_NAME_LEN 32 //"I "
#define MAX_PATH_LENGTH 128 // this is a lot, like a lot, of storage
#define MAX_DIFFNAME_LEN 8 
/*
Beet Saber Proof-of-Concept
*/
// 1. Define Key Objects
// a. The song class

struct Song { // note that, in the actual game, you wont have much room to print
  // You can trigger playback though?
  char[] artist,
  char[] song_name,
  char[] path_to_song,
  char[] diffname,
  uint32_t song_length // song must be under 72 hours long.
}; // total: 184 
// 1. The game states

enum game_states {
  GAME_START,
  SONG_SELECT,
  IN_GAME,
  REPORT_SCREEN,
  EXIT_GAME
};

int main(){
  // use printf("\r"); to reset line
  printf("Hi");
  //printf("\r");
  printf("Hi");
  //printf("\r");
  printf("Hi\n");
  return 0;
}