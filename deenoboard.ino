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

void light_led(int row, int col, int color, int brightness){
  int extra = 0;
  if (col % 2 != 0){
    col -= 1;
    extra = 1 + (row * 2);
  }
  int led = (14 - row) + (15 * col) + extra;
  leds[led]= CHSV(192, 255, 190);
  FastLED.show();    
}

void light_column(int rowStart, int rowEnd, int col, int color, int bright){
    for(int i=rowStart; i >= rowEnd; i--){
        leds[i + 15*col]= CHSV(color, 255, bright); 
        
        // values[i][col] = color; 
        // brightness[i][col] = bright;
    }
}
void light_row(int colStart, int colEnd, int row, int color, int bright){
  for (int i = colStart; i <= colEnd; i++){
    light_led(row, i, color, bright);
  }
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
  // clear_display();
  int tileColour = 185;
  int tileBrightness = 100;
  
  light_column(11, 3, 5, tileColour, tileBrightness);
  light_column(11, 3, 12, tileColour, tileBrightness);
  light_row(5, 12, 2, tileColour, tileBrightness);
  light_row(5, 12, 12, tileColour, tileBrightness);
  
  
  FastLED.show();
}
