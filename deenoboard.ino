#include <Keypad.h>
#include <FastLED.h>

#define SAMPLES 8        // Must be a power of 2
#define LED_PIN     6     // Data pin to LEDS
#define NUM_LEDS    270  
#define BRIGHTNESS  128    // from 0 to 255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB 
#define NUM_MODES 4

#define ROWS 5
#define COLS 6

int values[ROWS][COLS]; // 2D array to keep track of the current color of each tile
int mem_values[ROWS][COLS]; // 2D array to keep track Memory Colors
int brightness[ROWS][COLS]; // 2D array to keep track of the current brightness of each tile

bool escape;

// 3 * 2 array
const byte rows = 4;
const byte cols = 2;
char keys[rows][cols] = {
  {1, 2},
  {11, 12},
  {21, 22},
  {31, 32}
};

byte rowPins[rows] = {48, 44, 40, 38};
byte colPins[cols] = {36, 34};
Keypad buttons = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

CRGB leds[NUM_LEDS];
int mode = 1;
CRGBPalette16 currentPalette;

void setup() {
    Serial.begin(9600);
    delay(1000); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    buttons.setHoldTime(500); // require a .5 second hold to change modes
    randomSeed(analogRead(0)); //Seed Random
    // clear_display(); //Make sure the display is blank
}

void loop() {
  draw_tic_border();
}

void light_tile(int row, int col, int color, int bright){ 
    if (color == 256) 
        bright = 0; 
    for(int i=14; i >= 12; i--){
        leds[i - 3*row + 30*col]= CHSV(color, 255, bright); 
    }
    for(int i=15; i <= 17; i++){
        leds[i + 3*row + 30*col]= CHSV(color, 255, bright); 
    }
    values[row][col] = color; 
    brightness[row][col] = bright; 
}

void paint(){
  // clear_display();
  bool tap = 1;
  int color = 0;
  while(tap){                                               //Loop until a button is held
    int location = buttons.getKey();
    if (location){                                          //if a button is pressed
      if(values[location/10][(location%10)-1] != 256){
        color = values[location/10][(location%10)-1] + 32;  //Update Color if tile is already colored
        if (color > 256)
          color = 0;
      }  
      else if (color == 256)                                //Loop back to red at end of rainbow
        color = 0;
      light_tile(location/10, (location%10)-1, color, 255); //Light tile color
      values[location/10][(location%10)-1] = color;
      FastLED.show();                                       //Update display
      delay(100);                                           //wait 1/10th of a second
    }
    if(buttons.getState() == HOLD)                          //Exit function if tile held
      tap = 0;
  }
}


void clear_display(){
  for(int i = 0; i < ROWS; i++){
    for(int j = 0; j < COLS; j++){
      light_tile(i, j, 256, 0);
      values[i][j] = 256;
    }
  }
  FastLED.show();
}

void draw_tic_border(){
  // top of grid
  clear_display();
  int tileColour = 185;
  int tileBrightness = 100;
  for (int i = 2; i <= 6; i++){
      light_tile(0, i, tileColour, tileBrightness);
  }
  // bottom of grid
  for (int i = 2; i <= 6; i++){
      light_tile(4, i, tileColour, tileBrightness);
  }
  // left of grid
  for (int i = 1; i < 4; i++){
    light_tile(i, 2, tileColour, tileBrightness);
  }
  for (int i = 1; i < 4; i++){
    light_tile(i, 6, tileColour, tileBrightness);
  }
   
  FastLED.show();
}
