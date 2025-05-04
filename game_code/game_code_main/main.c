#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

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
#define MAX_HIT_OBJS 266 // maximum numebr of allowable hitobjects in song. (max across all songs rn is ETA at 266)
#define PRINT_PERIOD 20 // Min period in ms before screen can refresh

char gameplay = 1; // 1 if the game is being played
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
    .path_to_song = "../../oszs/charts/ETA/audio.mp3",//"../../oszs/charts/ETA/audio.mp3",
    .preview_time = 49205,
    .num_notes = 266,
    .hit_objs = {0x48001d05, 0x08001f9f, 0x0800223a, 0x480025b3, 0x0400276f, 0x0400292c, 0x48002ae8, 0x48002d83, 0x0800301d, 0x08003396, 0x44003553, 0x0400370f, 0x480038cc, 0x48003c45, 0x08003fbd, 0x08004336, 0x480046af, 0x0400486c, 0x44004a28, 0x48004b06, 0x08004da1, 0x48005493, 0x0400572d, 0x4400580c, 0x080059c8, 0x04005d41, 0x04005efd, 0x440060ba, 0x08006276, 0x44006511, 0x440065ef, 0x080067ac, 0x04006968, 0x48006b25, 0x48006ce1, 0x4800705a, 0x08007216, 0x480073d3, 0x0800774c, 0x48007908, 0x48007ac5, 0x48007e3d, 0x08007ffa, 0x080081b6, 0x08008373, 0x040088a8, 0x44008986, 0x48008a65, 0x44008c21, 0x44008ddd, 0x08008f9a, 0x44009313, 0x040094cf, 0x0800968c, 0x44009a05, 0x04009bc1, 0x08009d7d, 0x4400a018, 0x0400a0f6, 0x4400a2b3, 0x4800a46f, 0x0400a70a, 0x0400a7e8, 0x0400a9a5, 0x4800ab61, 0x0800aeda, 0x0800b253, 0x0400b5cc, 0x4400b945, 0x0400bcbd, 0x4800c036, 0x0400c1f3, 0x4800c3af, 0x0800c56c, 0x4400c806, 0x0400c8e5, 0x4800caa1, 0x4800ce1a, 0x0800cfd6, 0x0400d193, 0x4800d34f, 0x4800d50c, 0x4400d6c8, 0x4800d885, 0x0400dbfd, 0x4400dcdc, 0x4800ddba, 0x4400df76, 0x4800e133, 0x0400e3cd, 0x4400e4ac, 0x4800e668, 0x4400e9e1, 0x0400eabf, 0x4800eb9d, 0x4800ed5a, 0x4800ef16, 0x0400f1b1, 0x4400f28f, 0x4800f44c, 0x4400f8a3, 0x4800f981, 0x0400fb3d, 0x4800fcfa, 0x4800feb6, 0x44010073, 0x4801022f, 0x080103ec, 0x040105a8, 0x48010765, 0x44010921, 0x48010add, 0x08010c9a, 0x04010e56, 0x08011013, 0x080111cf, 0x4801138c, 0x04011626, 0x04011705, 0x480118c1, 0x08011a7d, 0x44011c3a, 0x08011df6, 0x48011fb3, 0x0801216f, 0x4401232c, 0x480124e8, 0x480126a5, 0x08012a1d, 0x48012bda, 0x48012e75, 0x4401310f, 0x080132cc, 0x44013566, 0x44013645, 0x08013801, 0x440139bd, 0x08013b7a, 0x48013d36, 0x04013ef3, 0x480140af, 0x0401434a, 0x44014428, 0x080145e5, 0x440147a1, 0x0801495d, 0x08014b1a, 0x44014cd6, 0x08014e93, 0x4801504f, 0x0801520c, 0x48015585, 0x48015741, 0x480158fd, 0x48015c76, 0x48015e33, 0x48015fef, 0x480161ac, 0x040166e1, 0x440167bf, 0x4801689d, 0x44016a5a, 0x04016c16, 0x48016dd3, 0x0401714c, 0x44017308, 0x080174c5, 0x4401783d, 0x040179fa, 0x08017bb6, 0x44017e51, 0x44017f2f, 0x040180ec, 0x080182a8, 0x44018543, 0x04018621, 0x040187dd, 0x4801899a, 0x08018d13, 0x4801908c, 0x04019405, 0x0401977d, 0x04019af6, 0x08019e6f, 0x4401a02c, 0x4801a1e8, 0x4801a3a5, 0x0401a63f, 0x4401a71d, 0x0801a8da, 0x4801ac53, 0x0801ae0f, 0x4401afcc, 0x4801b188, 0x4801b345, 0x4401b501, 0x4801b6bd, 0x4801ba36, 0x0801bbf3, 0x4401bdaf, 0x0801bf6c, 0x4401c206, 0x0401c2e5, 0x0801c4a1, 0x4401c81a, 0x0401c8f8, 0x4801c9d6, 0x4801cb93, 0x0801cd4f, 0x4401cfea, 0x0401d0c8, 0x4801d285, 0x4401d6dc, 0x4801d7ba, 0x4401d976, 0x4801db33, 0x0801dcef, 0x0401deac, 0x4801e068, 0x0801e225, 0x4401e3e1, 0x4801e59d, 0x0401e75a, 0x0801e916, 0x4801ead3, 0x4401ec8f, 0x0801ee4c, 0x0801f008, 0x0801f1c5, 0x4401f45f, 0x0401f53d, 0x4801f6fa, 0x0801f8b6, 0x4401fa73, 0x4801fc2f, 0x0801fdec, 0x0801ffa8, 0x04020165, 0x48020321, 0x480204dd, 0x08020856, 0x08020a13, 0x08020cad, 0x08020f48, 0x44021105, 0x480212c1, 0x0802163a, 0x080217f6, 0x48021b6f, 0x08021d2c, 0x08021ee8, 0x04022261, 0x4802241d, 0x48022953, 0x08022b0f, 0x48022ccc, 0x44023045, 0x48023201, 0x4402357a, 0x44023736, 0x08023815, 0x48023aaf, 0x44023fe5, 0x480241a1, 0x0802451a, 0x440247b5}
  }; 

struct Song song2_duvet = {
  .artist = "Boa",
  .song_name = "Duvet",
  .path_to_song = "../../oszs/charts/duvet/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 0, // zeroed out since the preview starts really late
  .num_notes = 78,
  .hit_objs = {0x08000776, 0x04000bd1, 0x04000d10, 0x48000eee, 0x480013f0, 0x080017bd, 0x44001e07, 0x04001feb, 0x4400212e, 0x44002312, 0x08002592, 0x44002959, 0x48002a9c, 0x04002f9c, 0x0400317d, 0x0800335f, 0x48003863, 0x44003c26, 0x48003d67, 0x48003fe9, 0x0400464b, 0x4800478d, 0x44004b53, 0x08004dd7, 0x44005235, 0x44005375, 0x08005554, 0x44005a67, 0x48005ba9, 0x480061f3, 0x080065b4, 0x04006a0b, 0x48006bfa, 0x08006fc9, 0x480074de, 0x440078a7, 0x080079e9, 0x0400801d, 0x040082a1, 0x08008525, 0x480088eb, 0x04008f36, 0x48009078, 0x4800943e, 0x08009804, 0x08009a89, 0x4400a088, 0x2000a2f0, 0x0400b734, 0x0800b9b6, 0x4800bd79, 0x4400c279, 0x4400c3ba, 0x4800c63b, 0x4400cb4a, 0x6000cdcc, 0x0800e97b, 0x0400f0f5, 0x4400f38e, 0x4800f4d0, 0x4400f896, 0x0800f9d8, 0x04010022, 0x040102a7, 0x080103e9, 0x480108f1, 0x08010cb7, 0x4801107e, 0x480116c8, 0x44011d13, 0x48011e55, 0x0801221b, 0x04012866, 0x44012aea, 0x48012c26, 0x0801311c, 0x080134e2, 0x600138a8}
};
struct Song song3_killing_my_love = {
  .artist = "Leslie Parrish",
  .song_name = "Killing My Love",
  .path_to_song = "../../oszs/charts/killing-my-love/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 76293,
  .num_notes = 256,
  .hit_objs = {0x0800068b, 0x4800080a, 0x44000b08, 0x08000c87, 0x48000e06, 0x48001103, 0x04001282, 0x040017be, 0x0800187e, 0x04001cfb, 0x08001e79, 0x04002177, 0x440022f6, 0x48002475, 0x040028f2, 0x08002a71, 0x04002d6e, 0x44002eed, 0x4800306c, 0x040032ab, 0x0800336a, 0x08003668, 0x480038a6, 0x04003ae4, 0x48003c63, 0x44003ea2, 0x48003f61, 0x4800425f, 0x0800449d, 0x040046dc, 0x0800485b, 0x040049dc, 0x04004a9c, 0x08004eb0, 0x48005331, 0x480054b1, 0x44005813, 0x08005933, 0x48005ab3, 0x08005f35, 0x080060b5, 0x44006417, 0x08006537, 0x080066b7, 0x080068f8, 0x48006b39, 0x08006cb9, 0x4400701a, 0x0800713b, 0x480072bb, 0x480074fc, 0x0800773d, 0x080078bd, 0x44007c1e, 0x48007d3f, 0x48007ebf, 0x08008100, 0x48008341, 0x480084c1, 0x080090c5, 0x08009306, 0x08009547, 0x040096c7, 0x44009848, 0x08009908, 0x08009b49, 0x08009cc9, 0x08009f0a, 0x4800a14b, 0x4400a2cb, 0x0400a44c, 0x0800a50c, 0x0800a74d, 0x0800a8cd, 0x4800ab0e, 0x4800ad4f, 0x0400aecf, 0x0400b04f, 0x4800b110, 0x0800b350, 0x0800b4d1, 0x0800b712, 0x4800b952, 0x0400bad3, 0x0400bc53, 0x4800bd14, 0x0800bf54, 0x0800c0d5, 0x4800c316, 0x4800c556, 0x4400c6d7, 0x4800c857, 0x0400ca98, 0x0400cc19, 0x0800ccd9, 0x4400d15a, 0x0800d21b, 0x0400d45b, 0x4400d5dc, 0x0800d75c, 0x4400d99d, 0x4800da5d, 0x4400dbde, 0x0400dc9e, 0x0800de1f, 0x0400e2a0, 0x0800e360, 0x4800e4e1, 0x4400e721, 0x0400e7e2, 0x4400e962, 0x4800ea22, 0x4800ec63, 0x0400ede4, 0x4400ef64, 0x0400f024, 0x0400f1a5, 0x4800f265, 0x0400f4a6, 0x4800f626, 0x0400f867, 0x4400f9e8, 0x4400fb68, 0x0800fce9, 0x4800fe69, 0x4400ffea, 0x040100aa, 0x0801022a, 0x0801046b, 0x040105ec, 0x040106ac, 0x4401082c, 0x480108ed, 0x04010bee, 0x08010cae, 0x48010e2e, 0x0401106f, 0x040111ef, 0x04011370, 0x080114f0, 0x48011671, 0x440117f1, 0x440118b2, 0x08011a32, 0x48011c73, 0x04011df3, 0x44011eb4, 0x44012034, 0x480120f4, 0x040123f5, 0x480124b6, 0x08012636, 0x44012877, 0x440129f7, 0x44012b78, 0x48012cf8, 0x04012e79, 0x08012f39, 0x4801317a, 0x440132fa, 0x480133bb, 0x4801353b, 0x4401377c, 0x480138fc, 0x44013a7d, 0x08013b3d, 0x44013d7e, 0x48013e3e, 0x08013fbe, 0x4401413f, 0x480142bf, 0x48014440, 0x04014681, 0x48014741, 0x44014982, 0x08014a42, 0x48014bc2, 0x08014d43, 0x08015044, 0x44015285, 0x08015345, 0x04015586, 0x08015646, 0x080157c6, 0x08015947, 0x04015b88, 0x08015d08, 0x44015e89, 0x48015f49, 0x4801618a, 0x4401630a, 0x080163ca, 0x0801654b, 0x4401678c, 0x0801690c, 0x44016a8d, 0x08016b4d, 0x44016d8e, 0x08016e4e, 0x48016fce, 0x0401714f, 0x480172cf, 0x48017450, 0x44017690, 0x48017751, 0x44017991, 0x08017a52, 0x48017bd2, 0x08017d53, 0x44017f93, 0x08018114, 0x44018294, 0x48018355, 0x44018595, 0x08018656, 0x480187d6, 0x48018957, 0x04018b97, 0x08018d18, 0x48018f59, 0x08019199, 0x4401931a, 0x0401949a, 0x0801955b, 0x4801979b, 0x4801991c, 0x48019b5d, 0x48019d9d, 0x44019f1e, 0x0401a09e, 0x4801a15e, 0x0801a39f, 0x4801a520, 0x0801a760, 0x4801a9a1, 0x0401ab22, 0x0401aca2, 0x4801ad62, 0x4801afa3, 0x0801b124, 0x4801b364, 0x4801b5a5, 0x0401b726, 0x0401b8a6, 0x0801b966, 0x4801bba7, 0x0801bd28, 0x4801bf68, 0x4801c1a9, 0x4401c32a, 0x4801c4aa, 0x0401c6eb, 0x0401c86b, 0x0801c92c, 0x4401caac, 0x0801cb6c}
};
struct Song song4_reality_surf = {
  .artist = "Bladee",
  .song_name = "Reality Surf",
  .path_to_song = "../../oszs/charts/reality-surf/audio.mp3",
  .preview_time = 72030,
  .num_notes = 193,
  .hit_objs = {0x0400030c, 0x040005fa, 0x040008e8, 0x04000ec4, 0x440011b2, 0x040014a0, 0x04001a7c, 0x04001d6a, 0x04002058, 0x44002634, 0x04002922, 0x04002c10, 0x440031ec, 0x440034da, 0x44003651, 0x440037c8, 0x04003ab6, 0x44003da4, 0x04004092, 0x04004209, 0x44004380, 0x4400466e, 0x0400495c, 0x44004dc1, 0x44004f38, 0x440050af, 0x04005226, 0x44005514, 0x04005979, 0x04005af0, 0x44005dde, 0x04005f55, 0x040060cc, 0x44006531, 0x440066a8, 0x4400681f, 0x04006996, 0x04006c84, 0x04006f72, 0x440070e9, 0x04007260, 0x4400754e, 0x040076c5, 0x4400783c, 0x04007b2a, 0x44007ca1, 0x04007e18, 0x04008106, 0x4400827d, 0x040083f4, 0x040089d0, 0x44008b47, 0x04008fac, 0x0400929a, 0x04009588, 0x440096ff, 0x04009876, 0x04009b64, 0x04009e52, 0x0400a140, 0x4400a2b7, 0x4400a42e, 0x4400a5a5, 0x4400a71c, 0x4400acf8, 0x0400ae6f, 0x0400afe6, 0x0400b2d4, 0x0400b739, 0x4400b8b0, 0x4400ba27, 0x4400bb9e, 0x4400bd15, 0x0400be8c, 0x4400c17a, 0x4400c2f1, 0x4400c756, 0x0400c8cd, 0x0400cbbb, 0x0400cd32, 0x0400cea9, 0x4400d020, 0x4400d30e, 0x4400d485, 0x4400d5fc, 0x4400d8ea, 0x4400dbd8, 0x4400dd4f, 0x4400dec6, 0x0400e03d, 0x4400e1b4, 0x4400e4a2, 0x0400e619, 0x4400e790, 0x4400e907, 0x4400ea7e, 0x4400f05a, 0x0400f348, 0x0400f4bf, 0x0400f636, 0x4400f7ad, 0x0400f924, 0x4400fc12, 0x4400ff00, 0x44010077, 0x440101ee, 0x04010365, 0x040104dc, 0x040107ca, 0x44010ab8, 0x04010c2f, 0x04010da6, 0x04010f1d, 0x44011094, 0x44011382, 0x040114f9, 0x0401195e, 0x44011c4c, 0x440120b1, 0x44012228, 0x04012516, 0x0401268d, 0x04012804, 0x04012de0, 0x04012f57, 0x440130ce, 0x04013245, 0x040133bc, 0x040136aa, 0x44013998, 0x44013b0f, 0x04013c86, 0x44013dfd, 0x44013f74, 0x44014262, 0x040143d9, 0x44014550, 0x440146c7, 0x0401483e, 0x04014b2c, 0x44014e1a, 0x04014f91, 0x44015108, 0x040156e4, 0x440159d2, 0x44015b49, 0x44015cc0, 0x44015fae, 0x0401629c, 0x4401658a, 0x44016878, 0x04016b66, 0x44016e54, 0x44017430, 0x44017a0c, 0x44017cfa, 0x04017fe8, 0x4401815f, 0x040182d6, 0x040185c4, 0x440188b2, 0x04018ba0, 0x04018d17, 0x04018e8e, 0x4401917c, 0x44019758, 0x040198cf, 0x04019a46, 0x04019d34, 0x4401a022, 0x0401a310, 0x4401a487, 0x0401a5fe, 0x0401a775, 0x0401a8ec, 0x0401abda, 0x4401ad51, 0x4401b1b6, 0x0401b4a4, 0x4401b792, 0x4401b9c4, 0x0401bd6e, 0x4401c05c, 0x4401c34a, 0x4401c4c1, 0x4401c926, 0x4401cc14, 0x0401cf02, 0x0401d134, 0x0401d4de, 0x4401d7cc, 0x4401daba, 0x4401dc31}
};
/*
struct Song song5_fubuki = {
  .artist = "Street",
  .song_name = "Sakura Fubuki",
  .path_to_song = "../../oszs/charts/fubuki/audio.mp3",
  .preview_time = 68630,
  .num_notes = 219,
  .hit_objs = {0x0400015e, 0x04000967, 0x04000b69, 0x040016cb, 0x04001ed4, 0x040020d6, 0x08002c39, 0x04002e3b, 0x08002ee7, 0x0400303e, 0x08003194, 0x08003442, 0x04003644, 0x080036f0, 0x0800399e, 0x04003ba0, 0x08003c4b, 0x04003ef9, 0x08003fa4, 0x040041a7, 0x040042fe, 0x04004454, 0x040045ab, 0x08004702, 0x08004859, 0x080049b0, 0x04004b07, 0x08004c5e, 0x04005062, 0x080051b9, 0x08005310, 0x04005467, 0x080055be, 0x04005714, 0x040057c0, 0x04005917, 0x040059c3, 0x04005b18, 0x04005bc4, 0x04005d1b, 0x04005dc7, 0x08005f1e, 0x04006074, 0x08006120, 0x04006479, 0x080065d0, 0x04006727, 0x0800687e, 0x040069d4, 0x08006a80, 0x04006c82, 0x08006dd9, 0x04006f30, 0x04007087, 0x04007132, 0x080071de, 0x0400748b, 0x08007537, 0x0800768e, 0x08007890, 0x040079e7, 0x08007b3e, 0x04007c94, 0x08007deb, 0x04007f42, 0x04008099, 0x04008144, 0x080081f0, 0x0800849e, 0x040085f4, 0x080086a0, 0x0800894e, 0x08008aa4, 0x08008bfb, 0x04008f54, 0x080090ab, 0x04009202, 0x08009359, 0x040094af, 0x0800955b, 0x0400975d, 0x080098b4, 0x04009a0b, 0x04009b62, 0x04009c0d, 0x08009cb9, 0x04009f66, 0x0800a012, 0x0800a169, 0x0800a36b, 0x0400a4c2, 0x0800a619, 0x0400a770, 0x0400a8c7, 0x0800aa1e, 0x0400ab74, 0x0800accb, 0x0400aecd, 0x0800af79, 0x0400b0cf, 0x0800b227, 0x0800b4d4, 0x0800b62b, 0x0400b782, 0x0400bc32, 0x0400c18e, 0x0400c6e9, 0x0400ccf0, 0x0400d1a0, 0x0400d6fb, 0x0400dc57, 0x0800e25e, 0x0400e5b7, 0x0800e70e, 0x0400e9bb, 0x0800ea67, 0x0400ec69, 0x0800ed14, 0x0400efc2, 0x0800f06e, 0x0800f270, 0x0400f51e, 0x0800f5c9, 0x0400f7cb, 0x0400f922, 0x0400fa79, 0x0800fbd0, 0x0400fd27, 0x0400fe7e, 0x0800ffd4, 0x0801012b, 0x04010282, 0x040103d9, 0x04010530, 0x040105db, 0x08010687, 0x040107de, 0x04010934, 0x08010a8b, 0x04010be2, 0x08010d39, 0x04011092, 0x0401113e, 0x08011294, 0x080113eb, 0x04011699, 0x08011744, 0x04011947, 0x08011a9e, 0x04011bf4, 0x08011ca0, 0x04011ea2, 0x08011ff9, 0x04012150, 0x080122a7, 0x040124a9, 0x04012600, 0x080126ab, 0x08012802, 0x04012a04, 0x08012b5b, 0x08012cb2, 0x08012f60, 0x04013162, 0x0801320e, 0x08013410, 0x04013567, 0x080136be, 0x04013815, 0x080138c0, 0x04013ac2, 0x04013c19, 0x08013cc4, 0x04013ec7, 0x0401401e, 0x080140c9, 0x04014220, 0x080142cb, 0x040144ce, 0x04014624, 0x080146d0, 0x04014827, 0x0801497e, 0x04014b80, 0x08014c2b, 0x08014d82, 0x04014f84, 0x040150db, 0x08015187, 0x080152de, 0x040154e0, 0x08015637, 0x0401578e, 0x08015839, 0x04015a3b, 0x04015b92, 0x08015c3e, 0x04015d94, 0x04015eeb, 0x08016042, 0x08016199, 0x040162f0, 0x040167a0, 0x04016cfb, 0x04017257, 0x0401785e, 0x04017d0e, 0x04018269, 0x040187c4, 0x04018dcb, 0x0401927b, 0x040197d6, 0x04019d32, 0x0401a339, 0x0401a7e9, 0x0401ad44, 0x0401b29f, 0x0801b750, 0x0801b8a7}
};
*/
struct Song song5_stupid_horse = {
  .artist = "100 gecs",
  .song_name = "Stupid Horse",
  .path_to_song = "../../oszs/charts/stupid-horse/audio.mp3",
  .preview_time = 29778,
  .num_notes = 182,
  .hit_objs = {0x04001410, 0x080014c2, 0x040016da, 0x0800183f, 0x48001c6e, 0x04001e86, 0x04001f39, 0x4800209e, 0x48002368, 0x48002798, 0x440029af, 0x04002a62, 0x08002bc7, 0x48002e91, 0x480032c1, 0x040034d8, 0x0400358b, 0x480036f0, 0x080039ba, 0x08003dea, 0x04004002, 0x440040b4, 0x4400437e, 0x040044e4, 0x44004596, 0x48004649, 0x44004913, 0x04004a79, 0x04004b2b, 0x48004bde, 0x08004ea8, 0x480052d7, 0x4800543c, 0x48005706, 0x440059d1, 0x44005b36, 0x04005c9b, 0x44005e00, 0x48005f65, 0x0400617d, 0x08006230, 0x080064fa, 0x48006a8e, 0x48006bf4, 0x48006d59, 0x44006f70, 0x08007023, 0x08007452, 0x480075b8, 0x040079e7, 0x08007b4c, 0x08007f7c, 0x080080e1, 0x04008510, 0x08008675, 0x48008aa5, 0x48008c0a, 0x04009039, 0x4800919e, 0x080095ce, 0x48009733, 0x04009b62, 0x48009cc8, 0x0800a0f7, 0x0800a25c, 0x0400a68c, 0x0800a7f1, 0x4800ac20, 0x0800ad85, 0x0400b1b5, 0x4800b31a, 0x0800b749, 0x0800b8ae, 0x4400bcde, 0x4800be43, 0x0800c272, 0x0800c3d8, 0x0800c807, 0x0400dcf4, 0x4800dda7, 0x0800dfbe, 0x4400e3ee, 0x4800e553, 0x0400e982, 0x4400eae8, 0x4400edb2, 0x4400f07c, 0x4800f346, 0x2000f776, 0x4401013a, 0x4401029f, 0x44010352, 0x48010404, 0x040106ce, 0x04010833, 0x440108e6, 0x08010998, 0x08010c63, 0x08010dc8, 0x48010f2d, 0x08011092, 0x080111f8, 0x4801135d, 0x44011627, 0x0801178c, 0x080118f1, 0x48011a56, 0x08011bbc, 0x08011d21, 0x08011e86, 0x44012150, 0x480122b5, 0x440124cd, 0x48012580, 0x0801284a, 0x44012a62, 0x08012b14, 0x08012dde, 0x44012ff6, 0x480130a9, 0x48013373, 0x0401358b, 0x4801363d, 0x44013908, 0x44013a6d, 0x44013bd2, 0x44013d37, 0x48013e9c, 0x040140b4, 0x48014166, 0x08014431, 0x480149c5, 0x08014b2a, 0x08014c90, 0x04014ea7, 0x48014f5a, 0x48015389, 0x480154ee, 0x0401591e, 0x08015a83, 0x08015eb2, 0x48016018, 0x04016447, 0x480165ac, 0x080169dc, 0x48016b41, 0x04016f70, 0x480170d5, 0x04017bfe, 0x44017d64, 0x44017e16, 0x08017ec9, 0x04018193, 0x040182f9, 0x440183ab, 0x4801845e, 0x48018728, 0x48018b57, 0x08018cbc, 0x48018f86, 0x44019251, 0x040193b6, 0x0401951b, 0x44019680, 0x080197e5, 0x440199fd, 0x48019ab0, 0x08019d7a, 0x4801a30e, 0x0801a474, 0x4801a5d9, 0x0401a7f0, 0x0801a8a3, 0x0801acd2, 0x0801ae38, 0x0401b267, 0x4801b3cc, 0x4801bef5, 0x0801c325, 0x4801c48a, 0x4401c8b9, 0x4801ca1e}
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
char perf_msg[] = "                                                                         \n ________________________________________________________________________\n/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/\n \\ \\/ /__  __ ______  / _ \\___ ____/ _/__  ______ _  ___ ____  _______   \n  \\  / _ \\/ // / __/ / ___/ -_) __/ _/ _ \\/ __/  ' \\/ _ `/ _ \\/ __/ -_)  \n  /_/\\___/\\_,_/_/   /_/   \\__/_/ /_/ \\___/_/ /_/_/_/\\_,_/_//_/\\__/\\__/   \n ________________________________________________________________________\n/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/___/\n                                                                         \n";
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
      grid[i][j]='.';
    }
  }
}
char get_channel(unsigned int note){
  return (note&1<<30)==(1<<30);
}
void draw_note(short x, short pad, char channel){
  // assuming that x and y are in the board, draw a square around x and y
  // x is depth, y is width
  short y;
  if (channel==0){ // 0 is left
    y=1;
  }
  else{
    y=6;
  }
  for (int i = x-pad; i<x+pad+1; i++){
    for (int j = y-pad; j<y+pad+1; j++){
      if (0<=i && i<32){ // 0<=i<32 aint legal
        grid[i][j] = 'O';
      }
    }
  }
  grid[x][y] = '*';
}
void realtime_feedback(char latest_hit_note){
   // clear until end of screen
  if (latest_hit_note == 4){
    printf("\n\033[2K");
    printf(BLU"Perfect\r\033[A"RESET);
  }
  if (latest_hit_note == 2){
    printf("\n\033[2K");
    printf(GRN"Good\r\033[A"RESET);
  } 
  if (latest_hit_note == 1){
    printf("\n\033[2K");
    printf(YEL"Ok...\r\033[A"RESET);
  }
  if (latest_hit_note == 0){
    printf("\n\033[2K");
    printf(RED"Miss!\r\033[A"RESET); 
  }
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
  printf("Now playing: %s by %s\n", songname, artist);
  // if all is well...
  ma_sound_start(ma_sownd);
  //printf("Song initted\n");
  song_initialized=1;
  //printf("Sound started yippee\n");
}
ma_uint32 get_song_time_ms(ma_sound *sound){ // !impt: 64bit math here. can't be im game
  return (ma_uint32) ma_sound_get_time_in_milliseconds(sound);
}
// c. Relevant variables concerning sound
ma_uint32 sr; // sample rate of current song
ma_uint32 frame; // what PCM frame the engine is at
ma_uint32 startframe; // used for determining songtime
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
short score=0;
short perfects=0;
short goods=0;
short oks=0; 
short misses=0;
float accuracy;
char last_hit_note = 127; // 4: perf, 2: good, 1: ok, 0: miss. 255 isn't real
// 3.b. Parsing hit objects
unsigned int has_been_hit(unsigned int hitobj){
  return (hitobj & (1<<31)) == 1<<31; // see game_readme_technical
}
unsigned int get_note_time_ms(unsigned int hitobj){
  return (hitobj&0b1111111111111111111); // 19 ones. see game_readme_technical
}
char pressed; // stores if a keypress was registered from some key (msut define key later)
// 3.c. Calculating metrics
float measure_accuracy(unsigned short song_note_cnt){
  // measures accuracy [0,1]
  // perfects are worth 100, goods 50, oks 25, miss 0
  return (perfects+0.5*goods+0.25*oks)/(song_note_cnt);
}
short measure_score(float accuracy, int maxcombo){
  float total_notes = perfects+goods+oks+misses;
  return (short) (40*(maxcombo/total_notes)+60*accuracy); // 40 from combo, 60 from acc out of 100. god knows nobody getting 100 tho
}
// later on you will be addin more boolfuncs for finding note type, color, direction, etc.

// 3.d. Finding valid hit objects
short first_valid_note_idx(short backward_idx, short forward_idx, unsigned int hitobjs[]){
  // find the index of the first valid note. if none, return 1.
  short idx = backward_idx;
  while (idx<forward_idx){
    if (!has_been_hit(hitobjs[idx])){
      return idx;
    }
    idx++;
  }
  return -1;
}
short valid_idx; 
short timediff; // time diff btwn hit time and perfect timing

// 3.e. Calculate hit precision
char perf_good_ok(short timediff){
  if (timediff<=DISPLAY_RESOLUTION){
    return 100; // perfect
  }
  if (timediff<=DISPLAY_RESOLUTION*2){
    return 50; // good
  }
  if (timediff<=DISPLAY_RESOLUTION*3){
    return 25; // ok
  }
  return 0; // shouldn't happen at all
}
char hit_result; // 100, 50, 25, 0

void mark_note_as_hit(unsigned int hitobjs[],short idx){
  hitobjs[idx] = hitobjs[idx] | 0x1<<31; // set 31st bit to 1
}
// 4. Reading from terminal
// 4.a. Modify terminal settings
struct termios termios_orig; // original terminal settings

void disable_raw_input(){
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_orig); // basically just reset terminal to original
}

void enable_raw_input(){
  tcgetattr(STDIN_FILENO, &termios_orig); // save original params. have care that the func isn't run twice
  atexit(disable_raw_input); // disable params after terminal exits (or else ur cooked upon ctrl+C prob)

  struct termios new = termios_orig; // copies
  new.c_lflag &= ~(ECHO | ICANON); // echo: repeat input. icanon: wait for enter
  tcsetattr(STDIN_FILENO,TCSAFLUSH,&new); // write

}
// 4.b. Emable non-blocking reads
void set_nonblocking() {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}
void reset_blocking() {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);  
  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}
// The Game
int main(){
  //printf("%s\n",perf_msg);
  //printf(RED "yippee\n" RESET);
  //printf("Foresight/hindsight %d %d\n",FORESIGHT_DISTANCE,HINDSIGHT_DISTANCE);
  
  //printf("%d\n", get_note_time_ms(0x0800223a));
  //printf("%d\n", 0x0800223a&0b1111111111111111111);
  // Declare song list
  struct Song songlist[] = {song1_eta, song2_duvet, song3_killing_my_love, song4_reality_surf, song5_stupid_horse};
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
  while (gameplay){
   switch (game_state){ // 1 of 5 game states
   
    /* State 1/5: Game start
        User is greeted with a welcome message.
        User can proceed to SONG_SELECT, or EXIT_GAME.
    */
    case GAME_START:
      // print a nice welcome message
      printf(CYN"%s"RESET, welcome_msg);
      printf("Enter e to continue! Or, enter x to exit...\n");
      // monitor button input
      while (1){ // while loop about responding to a specific button
        // get a key
        user_input = getchar();
        clear_input_buffer(); // just get one key
        // e to proceed (go to SONG_SELECT)
        if(user_input == 'e'){
          game_state = SONG_SELECT;
          printf("Going to song select...\n\n");
          load_song(0,songlist,&engine, &sound,1); // should be random, not zero
          break;
        }
        // x to leave (go to EXIT_GAME)
        else if (user_input == 'x'){
          game_state = EXIT_GAME;
          break;
        }
        // neither
        else{
          printf("Not e, and not x...\n");
        }
      }
      break;
    
    /* State 2/5: Song Select
        User is given a carousel of songs to pick from. User can hover over any song.
        User then hears the song, starting at a highlighted section, and sees the name of the song.
    */
    case SONG_SELECT:
    // How this might work on the MSP:
    // 1. Fetch random start song
    // // Keep track of millisecond time since song start, then get left 3 bits
    // // Use an LFSR and XOR it with a floating number (e.g. TIMA0->VALUE)
    // 2. Play random start song
    // // Find out if you can play starting at a specific time in ms using the audio chip
      printf("Welcome to song select! Enter A and D to scroll through songs. Enter space to confirm\n");
      while (1){ // Carousel
        // get a key
        printf("Song %d of %d\n",carousel_idx+1,songlist_len);
        user_input = getchar();
        clear_input_buffer();
        // If key was A, go left
        if (user_input=='a'){
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
      break;
    /* State 3: Gameplay (the meat)

    */
    case IN_GAME:
      // greet the player
      printf("Welcome to the game!\n");
      usleep(500000); // 0.5s sleep. Want extra long sleep b4 game starts
      
      // reset grid
      reset_grid();

      // load the song
      load_song(carousel_idx,songlist,&engine, &sound,0);
      sr = ma_engine_get_sample_rate(&engine); // sample rate of selected song

      // get song struct, hit objects, and num of notes
      song = &songlist[carousel_idx];
      hitobjs = song->hit_objs;
      song_note_cnt = song->num_notes;
      //printf("num notes %d\n",song_note_cnt);
      //printf("First num: %x\n",hitobjs[0]);

      // initialize/reset game variables
      forward_index = 0; // index of the next note that will enter the hit window
      backward_index = 0; // index of the next note that will leave the hit window
      forward_time = get_note_time_ms(hitobjs[forward_index]);
      backward_time = get_note_time_ms(hitobjs[backward_index]);

      // hide inputs and enable RT reads
      enable_raw_input();
      set_nonblocking();
      //printf("Wtf bruh\n");
      //printf("%x\n",hitobjs[0]);

      // core loop
      //printf("Forward time init: %d\n",forward_time);
      while (ma_sound_at_end(&sound)==MA_FALSE){
        // 1/5 find current time
        time_ms = (ma_uint32) get_song_time_ms(&sound); // this is probably correct !impt: You need to be very careful in the game if you want to do 32bit timing representation
        //printf("initial %d. Compare: %llu\n", time_ms, ma_sound_get_time_in_milliseconds(&sound));
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
            last_hit_note = 0;
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
        // 3/4: draw the window
        // 3a/4: prepare the window
        // !impt: especially on the actual msp, you might need to force a global offset due to lag
        // only update the board if you need to
        time_ms = get_song_time_ms(&sound);
        if (time_ms>grid_refresh_timestamp+PRINT_PERIOD){
          //printf("%d\n",time_ms);
          
          reset_grid();
          for (window_idx=backward_index; window_idx<forward_index; window_idx++){
            // first, get the row you're in (note that below implementation freezes time_ms)
            if (!has_been_hit(hitobjs[window_idx])){
              row = (time_ms+FORESIGHT_DISTANCE-get_note_time_ms(hitobjs[window_idx]))/DISPLAY_RESOLUTION;
              draw_note(row,1,get_channel(hitobjs[window_idx]));
            }
            //printf("Row: %d",row);
            // gng ur not checking if ntoe vanishes
            
          }
          //3b/4: print the window
          // putchar is fast, printf is not
          // ansi reset here
           // move cursor up 32 lines
           //printf("\033[2J");
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
          realtime_feedback(last_hit_note);
          fflush(stdout);
          printf("\033[32A\r");
          //clear // not doing due to inconsistent behavior
          grid_refresh_timestamp = get_song_time_ms(&sound);
          
        }
        
        // 4/4: measure hits
        
        if (read(STDIN_FILENO,&pressed,1)==1){ // i.e. if there was a non-arrowkey keypress
          //printf("you read\n");
          if (pressed == 'x'){ // i.e. leave
            // see results
            game_state = REPORT_SCREEN;
            // turn off sound
            conditional_uninit(&sound);
            // terminal back to normal
            disable_raw_input();
            reset_blocking(); 
            // clear grid and such
            printf("\033[J\n");
            printf("\nRage quit?\n");
            break;
          }
          // timing calculation
          time_ms = get_song_time_ms(&sound);
          valid_idx = first_valid_note_idx(backward_index,forward_index,hitobjs);
          if (valid_idx>-1){ // only do anything  if a note being hit happens at a valid time
            // calculate perfect/good/ok
            timediff = get_note_time_ms(hitobjs[valid_idx])-time_ms; // could this be chronically delayed?
            timediff = timediff>-1? timediff: -timediff;// abs value
            hit_result = perf_good_ok(timediff);
            // modify score results
            if (hit_result == 100) {
              perfects+=1;
              last_hit_note = 4;
              //printf("Perfect");
            }
            if (hit_result == 50) {
              goods+=1;
              last_hit_note = 2;
              //printf("Good");
            }
            if (hit_result == 25) {
              last_hit_note = 1;
              oks+=1;
              //printf("Ok");
            }
            // hit too early
            if (hit_result == 0){
              last_hit_note = 0;
              combo = 0;
              misses+=1; // actually whatever just handle the entire input who caress who cares
              //printf("Hit result was 0. Note time %d, note %d, hit time %d, index %d \n",get_note_time_ms(hitobjs[valid_idx]),hitobjs[valid_idx],time_ms,valid_idx);
             //break;
            }
            // constant tasks
            combo+=1;
            maxcombo = combo>maxcombo ? combo: maxcombo;
            // mark note as hit
            mark_note_as_hit(hitobjs,valid_idx);
          }
        }
      }
      // clear grid
      //printf("\033[32A\r");
      //printf("\033[J");
      // leave
      disable_raw_input();
      reset_blocking(); // gameplay done so this is off
      printf("Going to report screen...\n");
      game_state = REPORT_SCREEN;
      break;
      // print the last hit result on the final line. 
      // find out how to ambiently monitor hits
      // if a hit comes, do the 3 bin thing
      // the 3 bin thing should result in a print and a combo/maxcombo/accuracy modification.(EXT) (maxcombo only upon break/end?)
      // part 4 and beyond:
      // (EXT) calculate score at end 
      // (EXT) **btw, you might want to trim songs to a certain length. so actually have a time limit instead of note count limit?**
      //printf("Final time: %d\n",time_ms);
      //break;
    case REPORT_SCREEN:
      accuracy = measure_accuracy(song_note_cnt);
      printf(CYN"%s\n"RESET,perf_msg);
      printf(RED"Accuracy: %.2f\n"RESET,accuracy*100);
      printf(GRN"Perfect %d | Good: %d | Ok: %d\n"RESET,perfects,goods,oks);
      printf(MAG"Max combo: %d/%d\n"RESET,maxcombo,song_note_cnt);
      printf(CYN"Overall score: %d\n"RESET, measure_score(accuracy,maxcombo));
      printf("Thank you for playing! Enter x to leave, and e to select a new song!\n\n");
      // literally the exact same as part 1. would wrap in a function btw.
      while (1){ // while loop about responding to a specific button
        // get a key
        user_input = getchar();
        clear_input_buffer(); // just get one key
        // e to proceed (go to SONG_SELECT)
        if(user_input == 'e'){
          game_state = SONG_SELECT;
          printf("Going to song select...\n\n");
          load_song(carousel_idx,songlist,&engine, &sound,1); // should be random tbf
          break;
        }
        // x to leave (go to EXIT_GAME)
        else if (user_input == 'x'){
          game_state = EXIT_GAME; // redundant tbh but w/e
          break;
        }
        // neither
        else{
          printf("Not e, and not x...\n");
        }
      }
      game_state = (game_state == SONG_SELECT) ? SONG_SELECT : EXIT_GAME; // if you go to song select, go to song select. else exit game
      break;
    // Game State 5: Exit Game
    case EXIT_GAME:
      printf("Thanks for playing!\n\n");
      gameplay = 0;
      break;
    default: // how do you even get here?
      printf("No mans land...\n\n");
      break;
    } // end switch statement
  } // end while (1)

  // clear sound and engine
  disable_raw_input(); // if this is still on
  reset_blocking(); // redundant but w/e
  // remove sound and engine
  conditional_uninit(&sound);
  ma_engine_uninit(&engine);

  // goodbye
  return 0;
}