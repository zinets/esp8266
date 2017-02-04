#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include "FastLED.h"

#define NUM_LEDS      149
#define LED_TYPE      WS2812
#define COLOR_ORDER   GRB
#define DATA_PIN      4

const char* ssid = "hamster-wifi2";
const char* password = "134679852";

ESP8266WebServer server(80);

CRGB leds[NUM_LEDS];

#define TWINKLE_SPEED 3
#define TWINKLE_DENSITY 5
CRGB gBackgroundColor = CRGB::Black; //CRGB(0,0,6);
#define SECONDS_PER_PALETTE  60
CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;
int currentBrightness = 200;
bool repeatMode = true;

unsigned long timeToSwitch;

typedef void (*TwoArgumentPattern)(uint8_t arg1, uint8_t arg2);
typedef struct {
  TwoArgumentPattern mPattern;
  uint8_t mArg1;
  uint8_t mArg2;
} TwoArgumentPatterWithArgumentValues;

void juggle_2(uint8_t arg1, uint8_t arg2);
void bpm_2(uint8_t arg1, uint8_t arg2);
void sinelon_2(uint8_t arg1, uint8_t arg2);
void confetti_2(uint8_t arg1, uint8_t arg2);
void rainbowWithGlitter_2(uint8_t arg1, uint8_t arg2);

TwoArgumentPatterWithArgumentValues gPatternsAndArguments[] = {
  {rainbowWithGlitter_2,  5 /*stripeDensity*/,  0 /*chanceOfGlitter*/},
  {rainbowWithGlitter_2, 10 /*stripeDensity*/, 80 /*chanceOfGlitter*/},

  {sinelon_2, 6 /*BPM*/, 10 /*fadeAmount*/ },
  {sinelon_2,  3 /*BPM*/, 1 /*fadeAmount*/ },

  {bpm_2,     5 /*BPM*/, 3 /*stripeWidth*/},
  {bpm_2,    10 /*BPM*/, 7 /*stripeWidth*/},
  {bpm_2,     15 /*BPM*/, 1 /*stripeWidth*/},

  {confetti_2, 96 /*colorVariation*/, 30/*fadeAmount*/},
  {confetti_2, 16 /*colorVariation*/,  3/*fadeAmount*/},

  {juggle_2,  30 /*numDots*/,  3 /*baseBpmSpeed*/},
  {juggle_2,  80 /*numDots*/, 6 /*baseBpmSpeed*/}
};

String modeNames[] = {
  "RainbowStripe",
  "RedGreenWhite",
  "RainbowStripe",
  "BlueWhite",
  "RainbowStripe",
  "PartyColors",
  "Rainbow",
  "Rainbow + Glitter",
  "Sinelon 1",
  "Sinelon 2",
  "BPM 1",
  "BPM 2",
  "BPM 3",
  "Confetti 1",
  "Confetti 2",
  "Juggle 1",
  "Juggle 2",
};

const int numOfModes = 17; // общее кол-во режимов, 6 + 11
String currentModeName = "RainbowStripe";
int currentMode = -1;
uint8_t currentPalette = -1;
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("xmas.tree")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", handleRoot());
  });

  server.on("/mode_next", []() {
    nextMode();
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", handleRoot());
  });

  server.on("/bright_more", []() {
    moreBright();
    Serial.println("Brightness: " + String(currentBrightness));
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", handleRoot());
  });

  server.on("/bright_less", []() {
    lessBright();
    Serial.println("Brightness: " + String(currentBrightness));
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", handleRoot());
  });

  server.on("/mode_repeat", switchRepeatMode);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip);

  chooseNextColorPalette(gTargetPalette);
  timeToSwitch = millis() + SECONDS_PER_PALETTE * 1000;
}

bool isReelMode () {
  return currentMode > 5; // первые 5 режимов - из twinkle, дальше - из demoreel
}

void loop(void){
  if (repeatMode && millis() > timeToSwitch) {
    Serial.println("Time to switch!");
    timeToSwitch = millis() + SECONDS_PER_PALETTE * 1000;
    if (isReelMode()) {
      nextPattern();
    } else {
      chooseNextColorPalette( gTargetPalette );
    }
    currentModeName = modeNames[currentMode];
  }

  if (isReelMode()) {
    gHue++; // slowly cycle the "base color" through the rainbow

    // Call the current pattern function once, updating the 'leds' array
    uint8_t arg1 = gPatternsAndArguments[ gCurrentPatternNumber ].mArg1;
    uint8_t arg2 = gPatternsAndArguments[ gCurrentPatternNumber ].mArg2;
    TwoArgumentPattern pat = gPatternsAndArguments[ gCurrentPatternNumber ].mPattern;

    pat(arg1, arg2);

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/120); // about sixty FPS
  } else {
    EVERY_N_MILLISECONDS( 10 ) {
      nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 12);
    }
    drawTwinkles( leds, NUM_LEDS);
    FastLED.show();
  }
  server.handleClient();
}

void nextMode() {
  int x = (currentMode + 1) % numOfModes;
  currentModeName = modeNames[x];
  timeToSwitch = millis();
  repeatMode = true;
}

inline int min(int x, int y) {
  return x < y ? x : y;
}

inline int max (int x, int y) {
  return x > y ? x : y;
}

void moreBright() {
  currentBrightness = min(255, currentBrightness + 10);
  FastLED.setBrightness(currentBrightness);
}

void lessBright() {
  currentBrightness = max(100, currentBrightness - 10);
  FastLED.setBrightness(currentBrightness);
}

void switchRepeatMode() {
  repeatMode = !repeatMode;
  String state = repeatMode ? "on" : "off";
  Serial.println("Repeat mode: " + state);
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", handleRoot());
}

String handleRoot() {
  String res = "<html lang='en'><head>\
  	<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no, refresh='0;/'/>\
  	<title>Garaland interface</title>\
  	<style>	.c{text-align: center;}	div,input{padding:5px;font-size:1em;}	body{text-align: center;font-family:verdana;}	button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;}	.q{float: right;width: 64px;text-align: right;}	</style>\
    </head><body>	<div style='text-align:left;display:inline-block;min-width:260px;'><br/>\
      <h3>Garaland interface v. 0.3</h3>\
      <h4>Mode: " + currentModeName + " </h4>\
      <h4>Repeat mode: " + String(repeatMode ? "on" : "off") + "</h4>\
      <h4>Brightness: " + String(currentBrightness) + "</h4>\
      <h4>Local IP: " + WiFi.localIP().toString() + "</h4>\
      <form action='/mode_next' method='post'><button>Next mode</button></form>\
      <form action='/mode_repeat' method='post'><button>Repeat mode</button></form><br><br>\
      <form action='/bright_more' method='post'><button>Brightness +</button></form>\
      <form action='/bright_less' method='post'><button>Brightness -</button></form><br><br>\
  	</body></html>";
    return res;
}

void drawTwinkles( CRGB* L, uint16_t N) {
  uint16_t PRNG16 = 11337;
  uint32_t clock32 = millis();
  uint8_t backgroundBrightness = gBackgroundColor.getAverageLight();

  for( uint16_t i = 0; i < N; i++) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    CRGB c = computeOneTwinkle( myclock30, myunique8);
    if( c.getAverageLight() > backgroundBrightness) {
      L[i] = c;
    } else {
      L[i] = gBackgroundColor;
    }
  }
}

CRGB computeOneTwinkle( uint32_t ms, uint8_t salt) {
  uint16_t ticks = ms >> (8-TWINKLE_SPEED);
  uint8_t fastcycle8 = ticks;
  uint8_t slowcycle8 = (ticks >> 8) ^ salt;

  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < TWINKLE_DENSITY) {
    bright = triwave8( fastcycle8);
  }

  uint8_t hue = (slowcycle8 * 16) + salt;
  return ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
}

const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
   CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray,
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM =
{  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red,
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
   CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red,
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray };

const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM =
{  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
   CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray };

#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight,
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight,
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight,
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{  0x404040, 0x404040, 0x404040, 0x404040,
   0x404040, 0x404040, 0x404040, 0x404040,
   0x404040, 0x404040, 0x404040, 0x404040,
   0x404040, 0x404040, 0x404040, 0xFFFFFF };

const TProgmemRGBPalette16 RGB_p FL_PROGMEM = {
  CRGB::Red,   CRGB::Green, CRGB::Blue,  CRGB::Red,
  CRGB::Green, CRGB::Blue,  CRGB::Red,   CRGB::Green,
  CRGB::Blue,  CRGB::Red,   CRGB::Green, CRGB::Blue,
  CRGB::Red,   CRGB::Green, CRGB::Blue,  CRGB::Red,
};

const TProgmemRGBPalette16 White_p FL_PROGMEM = {
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray};

const TProgmemRGBPalette16* ActivePaletteList[] = {
  &RainbowStripeColors_p,
  &RedGreenWhite_p,
  &RainbowStripeColors_p,
  &BlueWhite_p,
  &RainbowStripeColors_p,
  &PartyColors_p,
};

void chooseNextColorPalette( CRGBPalette16& pal) {
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);

  currentPalette = addmod8(currentPalette, 1, numberOfPalettes);
  pal = *(ActivePaletteList[currentPalette]);

  currentMode = (currentMode + 1) % numOfModes;
  Serial.println("mode is " + String (currentMode));
}

void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  const int numberOfPatterns = sizeof(gPatternsAndArguments) / sizeof( gPatternsAndArguments[0]);
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
  currentMode = (currentMode + 1) % numOfModes;
}

void rainbowWithGlitter_2( uint8_t stripeDensity, uint8_t chanceOfGlitter) {
  // built-in FastLED rainbow, plus some random sparkly glitter
  fill_rainbow( leds, NUM_LEDS, gHue, stripeDensity);
  addGlitter(chanceOfGlitter);
}

void addGlitter( fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti_2( uint8_t colorVariation, uint8_t fadeAmount) {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, fadeAmount);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(colorVariation), 200, 255);
}

void sinelon_2( uint8_t bpmSpeed, uint8_t fadeAmount) {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, fadeAmount);
  int pos = beatsin16(bpmSpeed, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm_2( uint8_t bpmSpeed, uint8_t stripeWidth) {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = bpmSpeed;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*stripeWidth), beat);
  }
}

void juggle_2( uint8_t numDots, uint8_t baseBpmSpeed) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 100);
  byte dothue = 0;
  for( int i = 0; i < numDots; i++) {
    leds[beatsin16(i+baseBpmSpeed,0,NUM_LEDS)] |= CHSV(dothue, 255, 224);
    dothue += (256 / numDots);
  }
}
