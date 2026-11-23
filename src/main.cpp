#include <SDL.h>
#include <orbis/Sysmodule.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <proto-include.h>
#include <stdio.h>
#include <string.h>

static const int W=1920,H=1080;
static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Joystick *pad;
static FT_Library ft;
static FT_Face face;
static int page=0, selected=0, accent=0, running=1;
static Uint32 lastMove=0, toastEnd=0;
static char toast[128]="WELCOME TO PAY-LAUNCH";

struct Entry { const char *name,*kind,*description; };
static Entry entries[]={
 {"GOLDHEN","HOME BREW ENVIRONMENT","Reference entry for a homebrew feature environment."},
 {"FTP SERVER","NETWORK UTILITY","Reference entry for local network file transfer."},
 {"BINLOADER","DEVELOPER TOOL","Reference entry for development-time loading tools."},
 {"APP2USB","STORAGE UTILITY","Reference entry for external storage management."},
 {"WEBRTE","DEBUG UTILITY","Reference entry for runtime testing tools."},
 {"LINUX LOADER","BOOT UTILITY","Reference entry for alternate operating system tooling."}
};
static const int N=sizeof(entries)/sizeof(entries[0]);
static const char *nav[]={"OVERVIEW","PAYLOAD LIBRARY","CONSOLE","ACTIVITY LOG","SETTINGS"};
static SDL_Color col(int r,int g,int b){SDL_Color c={(Uint8)r,(Uint8)g,(Uint8)b,255};return c;}
static SDL_Color blue(){return accent?col(180,105,255):col(40,174,255);}
static void rect(int x,int y,int w,int h,SDL_Color c){SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);SDL_Rect r={x,y,w,h};SDL_RenderFillRect(ren,&r);}
static void edge(int x,int y,int w,int h,SDL_Color c){SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);SDL_Rect r={x,y,w,h};SDL_RenderDrawRect(ren,&r);}
static void txt(const char *s,int x,int y,int size,SDL_Color c){
 if(!face||!s)return; FT_Set_Pixel_Sizes(face,0,size); int pen=x,base=y+size;
 for(const unsigned char *p=(const unsigned char*)s;*p;p++){
  if(FT_Load_Char(face,*p,FT_LOAD_RENDER)){pen+=size/2;continue;}
  FT_GlyphSlot g=face->glyph; FT_Bitmap *b=&g->bitmap;
  SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,255);
  for(unsigned int yy=0;yy<b->rows;yy++)for(unsigned int xx=0;xx<b->width;xx++){
   int pitch=b->pitch; unsigned char a=pitch>=0?b->buffer[yy*pitch+xx]:b->buffer[(b->rows-1-yy)*(-pitch)+xx];
   if(a>48)SDL_RenderDrawPoint(ren,pen+g->bitmap_left+(int)xx,base-g->bitmap_top+(int)yy);
  }
  pen+=(int)(g->advance.x>>6);
 }
}
static void message(const char *s){snprintf(toast,sizeof(toast),"%s",s);toastEnd=SDL_GetTicks()+2400;}
static void panel(int x,int y,int w,int h){rect(x,y,w,h,col(12,23,38));edge(x,y,w,h,col(31,55,78));}
static void draw(){
 rect(0,0,W,H,col(7,12,21));
 for(int x=0;x<W;x+=48){SDL_SetRenderDrawColor(ren,14,24,38,255);SDL_RenderDrawLine(ren,x,0,x,H);}
 for(int y=0;y<H;y+=48){SDL_SetRenderDrawColor(ren,14,24,38,255);SDL_RenderDrawLine(ren,0,y,W,y);}
 rect(0,0,W,6,blue()); rect(0,6,W,86,col(10,18,30));
 txt("PAY-LAUNCH",64,25,34,col(244,249,255));
 txt("PS4 HOME BREW CONTROL CENTER",370,40,16,col(112,143,169));
 rect(1515,28,320,38,col(18,34,51));edge(1515,28,320,38,col(38,68,94));
 rect(1530,42,10,10,col(85,222,164));txt("APP READY",1552,36,15,col(184,207,225));
 rect(0,92,350,H-92,col(9,16,28));rect(348,92,2,H-92,col(28,47,67));
 txt("NAVIGATION",48,132,14,col(92,124,151));
 for(int i=0;i<5;i++){int y=188+i*82;if(page==i){rect(26,y-12,292,58,col(18,49,75));rect(26,y-12,4,58,blue());edge(26,y-12,292,58,col(37,89,128));}
  txt(nav[i],56,y,18,page==i?col(245,250,255):col(140,162,183));}
 txt("BUILD",48,684,13,col(92,124,151));txt("PAY-LAUNCH  01.00",48,716,15,col(179,201,219));
 txt("OPENORBIS TARGET",48,754,13,col(92,124,151));txt("PS4 HOME BREW",48,782,14,col(179,201,219));
 txt("L1 / R1: SWITCH SECTION",48,830,12,col(92,124,151));
 if(page==0){
  txt("OVERVIEW",400,134,30,col(244,249,255));txt("Your console utility hub.",404,180,17,col(128,155,179));
  panel(400,235,1030,300);rect(400,235,6,300,blue());txt("WELCOME BACK",448,276,15,blue());
  txt("ONE PLACE. YOUR TOOLS.",448,330,34,col(245,250,255));
  txt("Browse utility references and organize your homebrew workspace.",448,390,18,col(160,184,205));
  rect(448,452,240,48,col(20,67,100));edge(448,452,240,48,blue());txt("OPEN LIBRARY  >",470,465,16,col(240,248,255));
  panel(1458,235,378,300);txt("SYSTEM STATUS",1492,275,15,col(111,144,173));txt("READY",1492,330,32,col(91,226,166));
  txt("Firmware",1492,390,14,col(111,144,173));txt("Not detected",1640,390,14,col(206,220,232));
  txt("Payloads",1492,432,14,col(111,144,173));txt("Not bundled",1640,432,14,col(255,190,105));
  txt("QUICK ACCESS",400,590,17,col(214,229,243));
  const char *labels[]={"PAYLOAD LIBRARY","CONSOLE INFO","ACTIVITY LOG"};
  const char *desc[]={"Browse the local catalogue","View app-side status","See recent app messages"};
  for(int i=0;i<3;i++){int x=400+i*490;panel(x,636,450,190);txt(labels[i],x+26,674,18,col(238,247,255));txt(desc[i],x+26,720,15,col(130,158,183));txt("L1 / R1 TO OPEN",x+26,778,15,blue());}
 }else if(page==1){
  txt("PAYLOAD LIBRARY",400,134,30,col(244,249,255));txt("Catalogue entries for common homebrew tools.",404,180,17,col(128,155,179));
  panel(400,224,840,730);txt("AVAILABLE ENTRIES",434,260,15,col(111,144,173));
  for(int i=0;i<N;i++){int y=304+i*96;if(i==selected){rect(422,y-14,796,78,col(19,48,73));edge(422,y-14,796,78,blue());}
   rect(442,y+1,48,48,col(20,39,58));txt("P",457,y+12,22,blue());txt(entries[i].name,510,y,18,col(237,246,255));txt(entries[i].kind,510,y+32,12,col(105,139,168));txt("INFO ONLY",1050,y+13,12,col(255,190,105));}
  panel(1265,224,570,730);txt("ENTRY DETAILS",1300,260,15,col(111,144,173));
  txt(entries[selected].name,1300,320,25,col(244,249,255));txt(entries[selected].kind,1300,364,13,blue());
  txt(entries[selected].description,1300,422,16,col(169,190,209));
  txt("STATUS",1300,510,13,col(111,144,173));txt("CATALOGUE ONLY",1300,548,18,col(255,190,105));
  rect(1300,630,470,54,col(28,43,60));edge(1300,630,470,54,col(58,80,102));txt("RUN / INSTALL - NOT AVAILABLE",1322,646,14,col(255,190,105));
  txt("No executable payloads are included.",1300,730,14,col(113,143,169));
  txt("UP / DOWN: BROWSE     X: DETAILS     L1 / R1: SECTION",434,910,13,col(103,132,157));
 }else if(page==2){
  txt("CONSOLE",400,134,30,col(244,249,255));txt("Local application diagnostics.",404,180,17,col(128,155,179));panel(400,235,1435,620);
  const char *a[]={"Platform","Firmware","Display canvas","Network","Controller","Payload execution"};
  const char *b[]={"PlayStation 4 target","Not detected","1920 x 1080","Not queried","SDL joystick support","Disabled in this build"};
  for(int i=0;i<6;i++){int y=320+i*78;txt(a[i],444,y,16,col(126,153,177));txt(b[i],820,y,16,i==5?col(255,190,105):col(220,232,243));SDL_SetRenderDrawColor(ren,29,49,68,255);SDL_RenderDrawLine(ren,444,y+42,1795,y+42);}
  txt("This screen only reports information available to the app.",444,810,14,col(112,140,166));
 }else if(page==3){
  txt("ACTIVITY LOG",400,134,30,col(244,249,255));txt("Session messages and app status.",404,180,17,col(128,155,179));panel(400,235,1435,620);
  const char *logs[]={"PAY-LAUNCH interface initialized","Local catalogue loaded","Payload execution disabled in this build","Controller input ready when connected","No network requests made"};
  txt("SAMPLE EVENTS",444,280,13,col(103,135,163));
  for(int i=0;i<5;i++){int y=340+i*86;txt("DEMO",444,y,15,col(107,138,165));txt(logs[i],650,y,16,col(211,226,239));}
  txt("STATIC DEMO DATA - NOT A PERSISTENT SYSTEM LOG",444,790,13,col(255,190,105));
 }else{
  txt("SETTINGS",400,134,30,col(244,249,255));txt("Application preferences.",404,180,17,col(128,155,179));panel(400,235,1435,620);
  txt("APPEARANCE",444,280,16,col(111,144,173));txt("Accent color",444,350,18,col(224,236,246));rect(810,340,42,24,blue());txt(accent?"VIOLET":"BLUE",878,344,15,col(183,202,220));
  txt("Press X to switch the accent color.",444,405,14,col(118,147,172));
  txt("SAFETY & BEHAVIOR",444,500,16,col(111,144,173));txt("Payload execution",444,560,18,col(224,236,246));txt("Disabled in this catalogue build",810,560,16,col(255,190,105));
  txt("Network access",444,620,18,col(224,236,246));txt("No requests are made",810,620,16,col(143,216,188));
 }
 if(SDL_GetTicks()<toastEnd){rect(600,990,720,52,col(15,35,53));edge(600,990,720,52,blue());txt(toast,624,1004,15,col(226,239,250));}
 SDL_UpdateWindowSurface(win);
}
static void move(int d){if(page==1){selected=(selected+d+N)%N;}else{page=(page+d+5)%5;selected=0;}}
static void switchPage(int d){page=(page+d+5)%5;selected=0;message(nav[page]);}
static void confirm(){if(page==0){page=1;message("PAYLOAD LIBRARY OPENED");}else if(page==4){accent=!accent;message("ACCENT THEME UPDATED");}else if(page==1){message("ENTRY IS INFORMATION ONLY");}else message("SECTION SELECTED");}
static void event(SDL_Event *e){
 if(e->type==SDL_QUIT)running=0;
 if(e->type==SDL_JOYBUTTONDOWN){if(e->jbutton.button==0)confirm();else if(e->jbutton.button==1){page=0;message("HOME");}else if(e->jbutton.button==3&&page==1)message("NO PAYLOAD WAS EXECUTED");else if(e->jbutton.button==4)switchPage(-1);else if(e->jbutton.button==5)switchPage(1);}
 if(e->type==SDL_JOYHATMOTION&&SDL_GetTicks()-lastMove>180){if(e->jhat.value&SDL_HAT_DOWN){move(1);lastMove=SDL_GetTicks();}else if(e->jhat.value&SDL_HAT_UP){move(-1);lastMove=SDL_GetTicks();}}
 if(e->type==SDL_JOYAXISMOTION&&SDL_GetTicks()-lastMove>180&&e->jaxis.axis==1){if(e->jaxis.value>18000){move(1);lastMove=SDL_GetTicks();}else if(e->jaxis.value< -18000){move(-1);lastMove=SDL_GetTicks();}}
 if(e->type==SDL_KEYDOWN){if(e->key.keysym.sym==SDLK_ESCAPE)page=0;if(e->key.keysym.sym==SDLK_DOWN)move(1);if(e->key.keysym.sym==SDLK_UP)move(-1);if(e->key.keysym.sym==SDLK_RETURN||e->key.keysym.sym==SDLK_SPACE)confirm();if(e->key.keysym.sym==SDLK_RIGHT&&page==1)selected=(selected+1)%N;if(e->key.keysym.sym==SDLK_LEFT&&page==1)selected=(selected+N-1)%N;if(e->key.keysym.sym==SDLK_PAGEDOWN)switchPage(1);if(e->key.keysym.sym==SDLK_PAGEUP)switchPage(-1);}
}
int main(int argc,char **argv){
 (void)argc;(void)argv;setvbuf(stdout,NULL,_IONBF,0);
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK)!=0)return 1;
 sceSysmoduleLoadModule((OrbisSysModule)0x009A);
 if(FT_Init_FreeType(&ft)!=0)return 2;
 if(FT_New_Face(ft,"/app0/assets/DejaVuSans.ttf",0,&face)!=0)return 3;
 win=SDL_CreateWindow("PAY-LAUNCH",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,W,H,SDL_WINDOW_SHOWN);if(!win)return 4;
 ren=SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(win));if(!ren)return 5;
 if(SDL_NumJoysticks()>0)pad=SDL_JoystickOpen(0);
 message("WELCOME TO PAY-LAUNCH");
 while(running){SDL_Event e;while(SDL_PollEvent(&e))event(&e);draw();SDL_Delay(16);}
 if(pad)SDL_JoystickClose(pad);if(ren)SDL_DestroyRenderer(ren);if(win)SDL_DestroyWindow(win);
 if(face)FT_Done_Face(face);if(ft)FT_Done_FreeType(ft);SDL_Quit();return 0;
}