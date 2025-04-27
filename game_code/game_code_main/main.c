#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <unistd.h>

// ANSI Colors
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

// Gameplay Parameters
#define DISPLAY_RESOLUTION 60 // how much time in ms one LED row takes up. Analogous to Approach Rate in osu (inversel so)
#define WINDOW_OFFSET 4 // this is where the perfect window begins, measured from the bottom of the board
#define FORESIGHT_DISTANCE DISPLAY_RESOLUTION*29 // 29 leds are >0
#define HINDSIGHT_DISTANCE DISPLAY_RESOLUTION*3 // 3 leds are <0 w.r.t realtime
#define MAX_HIT_OBJS 360 // maximum numebr of allowable hitobjects in song. (360 isn't that much, but ra is tiht)
#define PRINT_PERIOD 30 // Min period in ms before screen can refresh

// 1. Common Objects
// a. The grid
char grid[32][8];
int grid_refresh_timestamp = 0;
// b. The song class
struct Song { // note that, in the actual game, you wont have much room to print
  // You can trigger playback though?
  const char *artist;
  const char *song_name;
  const char *path_to_song;
  //char diffname[MAX_DIFFNAME_LEN]; unneeded
  //unsigned int song_length; // not obtainable from osu metadata, p sure...
  unsigned int preview_time; // time in ms when the song preview starts
  unsigned short num_notes;
  unsigned int hit_objs[MAX_HIT_OBJS];
}; // size is variable. Realistically, no larger than 100B per song tho (everything besides hit objs)
// for the hit objs: up to 512 objects, each one 4 bytes (int + flags) => 2kB. Total: ~2.1kB/song max

struct Song song1_eta = {
    .artist ="NewJeans",
    .song_name = "ETA",
    .path_to_song = "/Users/donu/Desktop/S25/ELEC 327/327-final-proj/oszs/charts/ETA/audio.mp3",//"../../oszs/charts/ETA/audio.mp3",
    .preview_time = 49205,
    .num_notes = 266,
    .hit_objs = {0x08001d05, 0x08001f9f, 0x0800223a, 0x080025b3, 0x0400276f, 0x0400292c, 0x08002ae8, 0x08002d83, 0x0800301d, 0x08003396, 0x04003553, 0x0400370f, 0x080038cc, 0x08003c45, 0x08003fbd, 0x08004336, 0x080046af, 0x0400486c, 0x04004a28, 0x08004b06, 0x08004da1, 0x08005493, 0x0400572d, 0x0400580c, 0x080059c8, 0x04005d41, 0x04005efd, 0x040060ba, 0x08006276, 0x04006511, 0x040065ef, 0x080067ac, 0x04006968, 0x08006b25, 0x08006ce1, 0x0800705a, 0x08007216, 0x080073d3, 0x0800774c, 0x08007908, 0x08007ac5, 0x08007e3d, 0x08007ffa, 0x080081b6, 0x08008373, 0x040088a8, 0x04008986, 0x08008a65, 0x04008c21, 0x04008ddd, 0x08008f9a, 0x04009313, 0x040094cf, 0x0800968c, 0x04009a05, 0x04009bc1, 0x08009d7d, 0x0400a018, 0x0400a0f6, 0x0400a2b3, 0x0800a46f, 0x0400a70a, 0x0400a7e8, 0x0400a9a5, 0x0800ab61, 0x0800aeda, 0x0800b253, 0x0400b5cc, 0x0400b945, 0x0400bcbd, 0x0800c036, 0x0400c1f3, 0x0800c3af, 0x0800c56c, 0x0400c806, 0x0400c8e5, 0x0800caa1, 0x0800ce1a, 0x0800cfd6, 0x0400d193, 0x0800d34f, 0x0800d50c, 0x0400d6c8, 0x0800d885, 0x0400dbfd, 0x0400dcdc, 0x0800ddba, 0x0400df76, 0x0800e133, 0x0400e3cd, 0x0400e4ac, 0x0800e668, 0x0400e9e1, 0x0400eabf, 0x0800eb9d, 0x0800ed5a, 0x0800ef16, 0x0400f1b1, 0x0400f28f, 0x0800f44c, 0x0400f8a3, 0x0800f981, 0x0400fb3d, 0x0800fcfa, 0x0800feb6, 0x04010073, 0x0801022f, 0x080103ec, 0x040105a8, 0x08010765, 0x04010921, 0x08010add, 0x08010c9a, 0x04010e56, 0x08011013, 0x080111cf, 0x0801138c, 0x04011626, 0x04011705, 0x080118c1, 0x08011a7d, 0x04011c3a, 0x08011df6, 0x08011fb3, 0x0801216f, 0x0401232c, 0x080124e8, 0x080126a5, 0x08012a1d, 0x08012bda, 0x08012e75, 0x0401310f, 0x080132cc, 0x04013566, 0x04013645, 0x08013801, 0x040139bd, 0x08013b7a, 0x08013d36, 0x04013ef3, 0x080140af, 0x0401434a, 0x04014428, 0x080145e5, 0x040147a1, 0x0801495d, 0x08014b1a, 0x04014cd6, 0x08014e93, 0x0801504f, 0x0801520c, 0x08015585, 0x08015741, 0x080158fd, 0x08015c76, 0x08015e33, 0x08015fef, 0x080161ac, 0x040166e1, 0x040167bf, 0x0801689d, 0x04016a5a, 0x04016c16, 0x08016dd3, 0x0401714c, 0x04017308, 0x080174c5, 0x0401783d, 0x040179fa, 0x08017bb6, 0x04017e51, 0x04017f2f, 0x040180ec, 0x080182a8, 0x04018543, 0x04018621, 0x040187dd, 0x0801899a, 0x08018d13, 0x0801908c, 0x04019405, 0x0401977d, 0x04019af6, 0x08019e6f, 0x0401a02c, 0x0801a1e8, 0x0801a3a5, 0x0401a63f, 0x0401a71d, 0x0801a8da, 0x0801ac53, 0x0801ae0f, 0x0401afcc, 0x0801b188, 0x0801b345, 0x0401b501, 0x0801b6bd, 0x0801ba36, 0x0801bbf3, 0x0401bdaf, 0x0801bf6c, 0x0401c206, 0x0401c2e5, 0x0801c4a1, 0x0401c81a, 0x0401c8f8, 0x0801c9d6, 0x0801cb93, 0x0801cd4f, 0x0401cfea, 0x0401d0c8, 0x0801d285, 0x0401d6dc, 0x0801d7ba, 0x0401d976, 0x0801db33, 0x0801dcef, 0x0401deac, 0x0801e068, 0x0801e225, 0x0401e3e1, 0x0801e59d, 0x0401e75a, 0x0801e916, 0x0801ead3, 0x0401ec8f, 0x0801ee4c, 0x0801f008, 0x0801f1c5, 0x0401f45f, 0x0401f53d, 0x0801f6fa, 0x0801f8b6, 0x0401fa73, 0x0801fc2f, 0x0801fdec, 0x0801ffa8, 0x04020165, 0x08020321, 0x080204dd, 0x08020856, 0x08020a13, 0x08020cad, 0x08020f48, 0x04021105, 0x080212c1, 0x0802163a, 0x080217f6, 0x08021b6f, 0x08021d2c, 0x08021ee8, 0x04022261, 0x0802241d, 0x08022953, 0x08022b0f, 0x08022ccc, 0x04023045, 0x08023201, 0x0402357a, 0x04023736, 0x08023815, 0x08023aaf, 0x04023fe5, 0x080241a1, 0x0802451a, 0x040247b5}
  }; 

struct Song song2_duvet = {
  .artist = "Boa",
  .song_name = "Duvet",
  .path_to_song = "/Users/donu/Desktop/S25/ELEC 327/327-final-proj/oszs/charts/duvet/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 0, // zeroed out since the preview starts really late
  .num_notes = 78,
  .hit_objs = {0x08000776, 0x04000bd1, 0x04000d10, 0x08000eee, 0x080013f0, 0x080017bd, 0x04001e07, 0x04001feb, 0x0400212e, 0x04002312, 0x08002592, 0x04002959, 0x08002a9c, 0x04002f9c, 0x0400317d, 0x0800335f, 0x08003863, 0x04003c26, 0x08003d67, 0x08003fe9, 0x0400464b, 0x0800478d, 0x04004b53, 0x08004dd7, 0x04005235, 0x04005375, 0x08005554, 0x04005a67, 0x08005ba9, 0x080061f3, 0x080065b4, 0x04006a0b, 0x08006bfa, 0x08006fc9, 0x080074de, 0x040078a7, 0x080079e9, 0x0400801d, 0x040082a1, 0x08008525, 0x080088eb, 0x04008f36, 0x08009078, 0x0800943e, 0x08009804, 0x08009a89, 0x0400a088, 0x2000a2f0, 0x0400b734, 0x0800b9b6, 0x0800bd79, 0x0400c279, 0x0400c3ba, 0x0800c63b, 0x0400cb4a, 0x2000cdcc, 0x0800e97b, 0x0400f0f5, 0x0400f38e, 0x0800f4d0, 0x0400f896, 0x0800f9d8, 0x04010022, 0x040102a7, 0x080103e9, 0x080108f1, 0x08010cb7, 0x0801107e, 0x080116c8, 0x04011d13, 0x08011e55, 0x0801221b, 0x04012866, 0x04012aea, 0x08012c26, 0x0801311c, 0x080134e2, 0x200138a8}
};
struct Song song3_killing_my_love = {
  .artist = "Leslie Parrish",
  .song_name = "Killing My Love",
  .path_to_song = "/Users/donu/Desktop/S25/ELEC 327/327-final-proj/oszs/charts/killing-my-love/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 76293,
  .num_notes = 256,
  .hit_objs = {0x0800068b, 0x0800080a, 0x04000b08, 0x08000c87, 0x08000e06, 0x08001103, 0x04001282, 0x040017be, 0x0800187e, 0x04001cfb, 0x08001e79, 0x04002177, 0x040022f6, 0x08002475, 0x040028f2, 0x08002a71, 0x04002d6e, 0x04002eed, 0x0800306c, 0x040032ab, 0x0800336a, 0x08003668, 0x080038a6, 0x04003ae4, 0x08003c63, 0x04003ea2, 0x08003f61, 0x0800425f, 0x0800449d, 0x040046dc, 0x0800485b, 0x040049dc, 0x04004a9c, 0x08004eb0, 0x08005331, 0x080054b1, 0x04005813, 0x08005933, 0x08005ab3, 0x08005f35, 0x080060b5, 0x04006417, 0x08006537, 0x080066b7, 0x080068f8, 0x08006b39, 0x08006cb9, 0x0400701a, 0x0800713b, 0x080072bb, 0x080074fc, 0x0800773d, 0x080078bd, 0x04007c1e, 0x08007d3f, 0x08007ebf, 0x08008100, 0x08008341, 0x080084c1, 0x080090c5, 0x08009306, 0x08009547, 0x040096c7, 0x04009848, 0x08009908, 0x08009b49, 0x08009cc9, 0x08009f0a, 0x0800a14b, 0x0400a2cb, 0x0400a44c, 0x0800a50c, 0x0800a74d, 0x0800a8cd, 0x0800ab0e, 0x0800ad4f, 0x0400aecf, 0x0400b04f, 0x0800b110, 0x0800b350, 0x0800b4d1, 0x0800b712, 0x0800b952, 0x0400bad3, 0x0400bc53, 0x0800bd14, 0x0800bf54, 0x0800c0d5, 0x0800c316, 0x0800c556, 0x0400c6d7, 0x0800c857, 0x0400ca98, 0x0400cc19, 0x0800ccd9, 0x0400d15a, 0x0800d21b, 0x0400d45b, 0x0400d5dc, 0x0800d75c, 0x0400d99d, 0x0800da5d, 0x0400dbde, 0x0400dc9e, 0x0800de1f, 0x0400e2a0, 0x0800e360, 0x0800e4e1, 0x0400e721, 0x0400e7e2, 0x0400e962, 0x0800ea22, 0x0800ec63, 0x0400ede4, 0x0400ef64, 0x0400f024, 0x0400f1a5, 0x0800f265, 0x0400f4a6, 0x0800f626, 0x0400f867, 0x0400f9e8, 0x0400fb68, 0x0800fce9, 0x0800fe69, 0x0400ffea, 0x040100aa, 0x0801022a, 0x0801046b, 0x040105ec, 0x040106ac, 0x0401082c, 0x080108ed, 0x04010bee, 0x08010cae, 0x08010e2e, 0x0401106f, 0x040111ef, 0x04011370, 0x080114f0, 0x08011671, 0x040117f1, 0x040118b2, 0x08011a32, 0x08011c73, 0x04011df3, 0x04011eb4, 0x04012034, 0x080120f4, 0x040123f5, 0x080124b6, 0x08012636, 0x04012877, 0x040129f7, 0x04012b78, 0x08012cf8, 0x04012e79, 0x08012f39, 0x0801317a, 0x040132fa, 0x080133bb, 0x0801353b, 0x0401377c, 0x080138fc, 0x04013a7d, 0x08013b3d, 0x04013d7e, 0x08013e3e, 0x08013fbe, 0x0401413f, 0x080142bf, 0x08014440, 0x04014681, 0x08014741, 0x04014982, 0x08014a42, 0x08014bc2, 0x08014d43, 0x08015044, 0x04015285, 0x08015345, 0x04015586, 0x08015646, 0x080157c6, 0x08015947, 0x04015b88, 0x08015d08, 0x04015e89, 0x08015f49, 0x0801618a, 0x0401630a, 0x080163ca, 0x0801654b, 0x0401678c, 0x0801690c, 0x04016a8d, 0x08016b4d, 0x04016d8e, 0x08016e4e, 0x08016fce, 0x0401714f, 0x080172cf, 0x08017450, 0x04017690, 0x08017751, 0x04017991, 0x08017a52, 0x08017bd2, 0x08017d53, 0x04017f93, 0x08018114, 0x04018294, 0x08018355, 0x04018595, 0x08018656, 0x080187d6, 0x08018957, 0x04018b97, 0x08018d18, 0x08018f59, 0x08019199, 0x0401931a, 0x0401949a, 0x0801955b, 0x0801979b, 0x0801991c, 0x08019b5d, 0x08019d9d, 0x04019f1e, 0x0401a09e, 0x0801a15e, 0x0801a39f, 0x0801a520, 0x0801a760, 0x0801a9a1, 0x0401ab22, 0x0401aca2, 0x0801ad62, 0x0801afa3, 0x0801b124, 0x0801b364, 0x0801b5a5, 0x0401b726, 0x0401b8a6, 0x0801b966, 0x0801bba7, 0x0801bd28, 0x0801bf68, 0x0801c1a9, 0x0401c32a, 0x0801c4aa, 0x0401c6eb, 0x0401c86b, 0x0801c92c, 0x0401caac, 0x0801cb6c}
};
// initialize all of these things
char *artist;
char *song_name;
char *path_to_song;
unsigned int song_length;


// c. The game states
enum Game_States {
  GAME_START,
  SONG_SELECT,
  IN_GAME,
  REPORT_SCREEN,
  EXIT_GAME
};

enum Game_States game_state = GAME_START; // initialize game state
// d. Special Strings
// Welcome message. Sources: 
// https://patorjk.com/software/taag/
// https://tomeko.net/online_tools/cpp_text_escape.php?
char welcome_msg[] = "  _      __    __                     __         ___          __    ____     __             __\n | | /| / /__ / /______  __ _  ___   / /____    / _ )___ ___ / /_  / __/__ _/ /  ___ ____  / /\n | |/ |/ / -_) / __/ _ \\/  ' \\/ -_) / __/ _ \\  / _  / -_) -_) __/ _\\ \\/ _ `/ _ \\/ -_) __/ /_/ \n |__/|__/\\__/_/\\__/\\___/_/_/_/\\__/  \\__/\\___/ /____/\\__/\\__/\\__/ /___/\\_,_/_.__/\\__/_/   (_) \n\n";

// e. Misc Variables
char user_input = '\0'; // tracks user input
int prev_carousel_idx = 0; 
int carousel_idx = 0;

// f. Helper functions for strings
void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n') { } // read until you hit the end
}
void reset_grid(){
  for (int i=0;i<32;i++){
    for (int j=0;j<8;j++){
      grid[i][j]='x';
    }
  }
}
void draw_note(short x, short pad){
  // assuming that x and y are in the board, draw a square around x and y
  // x is depth, y is width
  short y = 3; // arbitrary start column!!
  for (int i = x-pad; i<x+pad+1; i++){
    for (int j = y-pad; j<y+pad+1; j++){
      if (0<=i && i<32){ // 0<=i<32 aint legal
        grid[i][j] = 'O';
      }
    }
  }
  grid[x][y] = '*';
}
// 2. Sound
// a. Playback
unsigned short song_initialized = 0; //true if song is playing

unsigned short conditional_uninit(ma_sound *ma_sownd){
  if (song_initialized){
    ma_sound_stop(ma_sownd);
    ma_sound_uninit(ma_sownd);
    //printf("Stopped and uninit\n");
    usleep(360000); // !impt: delay in us btwn songs. Without this you get artifacting.
    song_initialized = 0;
    return 0; 
  }
  return 1; // if this is true, already uninit
}
void load_song(int idx, struct Song song_array[], ma_engine *ma_enjine, ma_sound *ma_sownd, short preview){
  // clear old song
  int double_uninit = conditional_uninit(ma_sownd);
  // get metadata
  const char *songname = song_array[idx].song_name;
  const char *artist = song_array[idx].artist;
  unsigned int prevyew_time = song_array[idx].preview_time;
  const char *path = song_array[idx].path_to_song;
  // play the song
  // Load sound from file (but don’t auto-start)
  if (ma_sound_init_from_file(ma_enjine, path, 0, NULL, NULL,ma_sownd) != MA_SUCCESS){
    printf("oop!!\n");
    return;
  }
  // start playback at preview time
  // // part 1: algebra
  float time_to_go_to = prevyew_time/1000.0; // preview time is in ms
  ma_uint32 sr = ma_engine_get_sample_rate(ma_enjine); // sample rate. calculated per song?
  ma_uint32 frameToSeek;
  if (preview!=0){
    frameToSeek = (ma_uint32)(time_to_go_to * sr);
  }
  else{
    frameToSeek = 0;
  }
  // // Part a: Start playback
  if (ma_sound_seek_to_pcm_frame(ma_sownd, frameToSeek) != MA_SUCCESS) {
    printf("Couldn't find preview time... !\n");
  }
  usleep(80000);
  printf("Now playing: %s by %s\n\n", songname, artist);
  // if all is well...
  ma_sound_start(ma_sownd);
  //printf("Song initted\n");
  song_initialized=1;
  //printf("Sound started yippee\n");
}
int current_song_time_ms(ma_uint32 frame, ma_uint32 sr){
  return ((frame * 250)/sr)*4; // !impt: tradeoff between memory and resolution is key asf. see readme
}
// c. Relevant variables concerning sound
ma_uint32 sr; // sample rate of current song
ma_uint32 frame; // what PCM frame the song is at
int time_ms; //  what time, in ms, the song is currently at

// 3. Gameplay
// 3.a. Gameplay variables
// 3.a.1 Song variables
unsigned short forward_index = 0; // index of the next note that will enter the hit window
unsigned short backward_index = 0; // index of the next note that will leave the hit window
unsigned short window_idx; // for iterating through window
short row; // for grid iteration
int forward_time; // time corresponding to forward_idx
int backward_time; // time corresponding to backward_idx
short change = 1; // if the board state should change
unsigned short song_note_cnt;// number of notes in the song
unsigned int *hitobjs;
struct Song *song;
// 3.a.2. Performance Variables
short combo=0;
short maxcombo=0;
float accuracy=100;
short score=0;
short perfects=0;
short goods=0;
short oks=0; 
short misses=0;
// 3.b. Parsing hit objects
unsigned int has_been_hit(unsigned int hitobj){
  return (hitobj & (1<<31)) == 1<<31; // see game_readme_technical
}
unsigned int get_note_time_ms(unsigned int hitobj){
  return (hitobj&0b1111111111111111111); // 19 ones. see game_readme_technical
}
// 3.c. Calculating metrics
float measure_accuracy(){
  // perfects are worth 100, goods 50, oks 25, miss 0
  return (perfects+0.5*goods+0.25*oks)/(perfects+goods+oks+misses);
}
int measure_score(float accuracy, int maxcombo){
  float total_notes = perfects+goods+oks+misses;
  return (int) (40*(maxcombo/total_notes)+60*accuracy); // 40 from combo, 60 from acc out of 100. god knows nobody getting 100 tho
}
// later on you will be addin more boolfuncs for finding note type, color, direction, etc.

// 3.b. The display window
// The Game
int main(){
  //printf(RED "yippee\n" RESET);
  //printf("Foresight/hindsight %d %d\n",FORESIGHT_DISTANCE,HINDSIGHT_DISTANCE);
  
  //printf("%d\n", get_note_time_ms(0x0800223a));
  //printf("%d\n", 0x0800223a&0b1111111111111111111);
  // Declare song list
  struct Song songlist[] = {song1_eta, song2_duvet, song3_killing_my_love};
  int songlist_len = sizeof(songlist)/sizeof(songlist[0]);
  // Set up miniaudio
  ma_sound sound;
  ma_engine engine;
  ma_result result;

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    return result;  // Failed to initialize the engine.
  }
  // Main game loop
  while (1){
    /*Phase 1: Game start
        User is greeted with a welcome message.
        User can proceed to SONG_SELECT, or EXIT_GAME.
    */
    if (game_state == GAME_START){
      printf("%s",welcome_msg);
      printf("Enter e to continue! Or, enter x to exit...\n");
      /*
      printf("testing 1\n");
      printf("testing 2\n");
      printf("\033[2A"); //go up
      printf("hi\n");
      */
      while (1){ // while loop about responding to a specific button
        // get a key
        user_input = getchar();
        clear_input_buffer();
        // proceed
        if(user_input == 'e'){
          game_state = SONG_SELECT;
          printf("Going to song select...\n\n");
          load_song(0,songlist,&engine, &sound,1); // should be random, not zero
          break;
        }
        // enter x to leave
        else if (user_input == 'x'){
          game_state = EXIT_GAME;
          break;
        }
        // you messed up
        else{
          printf("Not e, and not x...\n");
        }
      }
    }
    /* Game State 2:  Song Select
          User is given a carousel of songs to pick from. User can hover over any song
          User hears the song, starting at a highlighted section, and sees the name of the song.
    */
    // How this might work on the MSP:
    // 1. Fetch random start song
    // // Keep track of millisecond time since song start, then get left 3 bits
    // // Use an LFSR and XOR it with a floating number (e.g. TIMA0->VALUE)
    // 2. Play random start song
    // // Find out if you can play starting at a specific time in ms using the audio chip
    if (game_state == SONG_SELECT){
      printf("Welcome to song select! Enter A and D to scroll through songs. Enter space to confirm\n");
      while (1){ // Carousel
        // get a key
        printf("Carousel idx: %d\n",carousel_idx);
        user_input = getchar();
        clear_input_buffer();
        // If key was A, go left
        if (user_input=='a'){
          // clear song
          // go left
          carousel_idx--; 
          // wrap around to end if needed
          if (carousel_idx<0){
            carousel_idx = songlist_len-1; 
          }
          // play song 
          load_song(carousel_idx,songlist,&engine, &sound,1); 
        }
        // If key was D, go right
        else if (user_input=='d'){
          // go right
          carousel_idx++;
          // wrap around to start if needed
          if (carousel_idx==songlist_len){ 
            carousel_idx=0;
          }
          // play song 
          load_song(carousel_idx,songlist,&engine, &sound,1); 
        }
        // If key was space, confirm selection
        else if (user_input==' '){ // confirm
          // clear song
          conditional_uninit(&sound);
          game_state = IN_GAME;
          break;
        }
        // If none of the above
        else{
          printf("Invalid input bruh\n");
        }
      }
    }
    /* State 3: Gameplay (the meat)

    */
    if (game_state == IN_GAME){
      // greet the player
      printf("Welcome to the game!\n");
      usleep(500000); // 0.5s sleep. Want extra long sleep b4 game starts
      // reset grid
      reset_grid();
      // load the song
      load_song(carousel_idx,songlist,&engine, &sound,0);
      sr  = ma_engine_get_sample_rate(&engine); // sample rate of selected song
      // get song struct, hit objects, and num of notes
      song = &songlist[carousel_idx];
      hitobjs = song->hit_objs;
      song_note_cnt = song->num_notes;
      printf("num notes %d\n",song_note_cnt);
      printf("First num: %x\n",hitobjs[0]);
      // initialize/reset game variables
      forward_index = 0; // index of the next note that will enter the hit window
      backward_index = 0; // index of the next note that will leave the hit window
      forward_time = get_note_time_ms(hitobjs[forward_index]);
      backward_time = get_note_time_ms(hitobjs[backward_index]);
      //printf("Wtf bruh\n");
      //printf("%x\n",hitobjs[0]);
      // core loop
      //printf("Forward time init: %d\n",forward_time);
      while (ma_sound_at_end(&sound)==MA_FALSE){
        // 1/5 find current time
        frame = ma_engine_get_time_in_pcm_frames(&engine);
        time_ms = current_song_time_ms(frame,sr); // this is probably correct !impt: You need to be very careful in the game if you want to do 32bit timing representation
        
        // 2a/5 check if a note just entered the window
        if (FORESIGHT_DISTANCE+time_ms>forward_time){
          forward_index +=1;
          change = 1;
          if (forward_index == song_note_cnt){
            forward_time = 0x0FFFFFFF; // set to ~inf; there is no next note
          }
          else{
            forward_time = get_note_time_ms(hitobjs[forward_index]);
          }
          //printf("Time: %d | Window %d | Next time: %d |Idx: %d|\n",time_ms,FORESIGHT_DISTANCE,forward_time,forward_index);
          //printf("Forward advance at %d ms with %d ms window. Next time: %d\n. Idx: %d",time_ms,FORESIGHT_DISTANCE,forward_time,forward_index);
        }
        // 2b/5 check if a note just exited the window
        if (backward_time+HINDSIGHT_DISTANCE<time_ms){
          // this means something just left.
          if (has_been_hit(hitobjs[backward_index])==0){ // if hasn't been hit
            combo = 0;
            misses+=1;
            // no need for real time acc. but if you wanted it:
            // - reset accuracy
            // - reset combo
            // - modify score
          }
          backward_index+=1;
          change = 1;
          if (backward_index == song_note_cnt){
            backward_time = 0x0FFFFFFF; // set to ~inf; there is no next note
          }
          else{
            backward_time = get_note_time_ms(hitobjs[backward_index]);
          }
          //printf("Time: %d | Window -%d | Next time: %d |Idx: %d|\n",time_ms,HINDSIGHT_DISTANCE,backward_time,backward_index);
        }
        // 3/5: draw the window
        // 3a/5: prepare the window
        // !impt: especially on the actual msp, you might need to force a global offset due to lag
        // only update the board if you need to
        frame = ma_engine_get_time_in_pcm_frames(&engine);
        if (current_song_time_ms(frame,sr)>grid_refresh_timestamp+PRINT_PERIOD){
          reset_grid();
          for (window_idx=backward_index; window_idx<forward_index; window_idx++){
            // first, get the row you're in (note that below implementation freezes time_ms)
            row = (time_ms+FORESIGHT_DISTANCE-get_note_time_ms(hitobjs[window_idx]))/DISPLAY_RESOLUTION;
            //printf("Row: %d",row);
            draw_note(row,1);
          }
          //3b/5: print the window
          // putchar is fast, printf is not
          // ansi reset here
           // move cursor up 32 lines
           printf("\033[2J");
          for (int a=0; a<32; a++){
            if (a==28 || a==29){ // special colors for some rows
              printf(BLU); // perfect region
            }
            if (a==27 || a==30){
              printf(GRN); // good region
            }
            if (a==26 || a==31){
              printf(YEL); // ok region
            }
            for (int b=0; b<8; b++){
              putchar(grid[a][b]);
            }
            if(a>26){ // stop the color from bleeding over
              printf(RESET);
            }
            putchar('\n');
          }
          fflush(stdout);
          //printf("\033[32A\r");
          //clear // not doing due to inconsistent behavior
          frame = ma_engine_get_time_in_pcm_frames(&engine); // current song time in PCM
          grid_refresh_timestamp = current_song_time_ms(frame,sr);
        }
        
      }
      // REMAINING TASKS:
      // 1. Add P,G,O zones
      // 2. Measure hits
      // 3. 
      //4/4: measure hits
      //printf("Final time: %d\n",time_ms);
      //break;
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
  // clear sound and engine
  conditional_uninit(&sound);
  ma_engine_uninit(&engine);
  // goodbye
  return 0;
}