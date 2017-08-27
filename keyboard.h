#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include "config.h"
#include <string>
#include <vector>
#include <list>


const string KEYCAP_BACKSPACE = "\u2190";
const string KEYCAP_SHIFT     = "\u2191";
const string KEYCAP_SYMBOLS   = "SYM";
const string KEYCAP_ABC       = "abc";
const string KEYCAP_SPACE     = " ";
const string KEYCAP_RETURN    = "\n";

struct touchArea {
  string keyChar;
  int x1;
  int x2;
  int y1;
  int y2;
};

struct rgb {
  int r;
  int g;
  int b;
};

struct argb{
  int a;
  int r;
  int g;
  int b;
};

struct KeyboardLayer{
  SDL_Surface *surface;
  SDL_Texture *texture;
  list<string> row1;
  list<string> row2;
  list<string> row3;
  list<string> row4;
  vector<touchArea> keyList;
  int layerNum;
};

class Keyboard {

  public:
    Keyboard(int pos, int targetPos, int width,
             int height, Config *config);
    ~Keyboard();
    string getCharForCoordinates(int x, int y);
    int setKeyboardColor(int r, int g, int b);
    int setInputColor(int r, int g, int b);
    int setDotColor(int r, int g, int b);
    float getPosition();
    void setPosition(float p);
    float getTargetPosition();
    void setTargetPosition(float p);
    float getHeight();
    void draw(SDL_Renderer *renderer, int screenHeight);
    int getActiveLayer();
    void setActiveLayer(int layerNum);
    int setLayout(int layoutNum);
    int init(SDL_Renderer *renderer);

  private:
    rgb keyboardColor;
    rgb inputColor;
    rgb dotColor;
    float position;
    float targetPosition;
    int keyboardWidth;
    int keyboardHeight;
    int activeLayer;
    list<KeyboardLayer> keyboard;
    Config *config;

    void drawRow(SDL_Surface *surface, vector<touchArea> *keyList, int x, int y,
                 int width, int height, list<string> *keys, int padding,
                 TTF_Font *font);
    void drawKey(SDL_Surface *surface, vector<touchArea> *keyList, int x, int y,
                 int width, int height, char *cap, string key, int padding,
                 TTF_Font *font);
    SDL_Surface *makeKeyboard(KeyboardLayer *layer);
    void loadKeymap(string keymapPath);
};

