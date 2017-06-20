#include <ArduinoJson.h>
#include <UTFT.h>
#include <WiFiEsp.h>

char ssid[] = "xxxxxx";
char pwd[] = "xxxxxx";
char server[] = "xxxxx";
WiFiEspClient client;

UTFT tft(ITDB32S,38,39,40,41);

extern uint8_t BigFont[];

const int MODE_ON_OFF = 0;
const int MODE_SELECT = 1;
const int SELECT_STATUS = 0;
const int ON_STATUS = 1;

int bar_width = 0;
int mode = MODE_SELECT;

int selected_color[3] = {240,240,30};

// selected, on/off, red, green, blue
int light_status[6][5] = {
 {1,1,240,240,30},
 {1,1,0,155,0},
 {1,1,240,240,30},
 {0,0,240,240,30},
 {1,0,240,240,30},
 {1,1,240,240,30}
};

String light_map[6] = {
  "5", //light 0
  "4", //light 1
  "1", //light 2
  "6", //light 3
  "2", //light 4
  "3", //light 5
};

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));

  setupTFT();
  setpuWifi();
  setupColorScreen();
}

void setupTFT() {
  tft.InitLCD(PORTRAIT);
  tft.clrScr();
  tft.setFont(BigFont);
  
  bar_width = tft.getDisplayXSize()/10;
}

void setpuWifi() {
  Serial1.begin(115200); // ESP
  WiFi.init(&Serial1);
  WiFi.begin(ssid, pwd);
}

/*
 * INIT SETUP FOR SCREEN
 */
void setupColorScreen() {
  printColorLabels();
  printColorStrokes();
  printColorScreenButtons();
  setColorScreenToNewColor();
}

void printColorLabels() {
  tft.setBackColor(VGA_BLACK);
  tft.setColor(VGA_WHITE);
  
  tft.print(F("R"), bar_width-22, 10);
  tft.print(F("E"), bar_width-22, 25);
  tft.print(F("D"), bar_width-22, 40);
  tft.print(F("G"), bar_width*3-22, 10);
  tft.print(F("R"), bar_width*3-22, 25);
  tft.print(F("E"), bar_width*3-22, 40);
  tft.print(F("E"), bar_width*3-22, 55);
  tft.print(F("N"), bar_width*3-22, 70);
  tft.print(F("B"), bar_width*5-22, 10);
  tft.print(F("L"), bar_width*5-22, 25);
  tft.print(F("U"), bar_width*5-22, 40);
  tft.print(F("E"), bar_width*5-22, 55);
}

void printColorScreenButtons() {
  int scene_top = 10;
  int mode_top = 80;
  int light_top = 300;
  
  // button & label
  tft.setBackColor(VGA_BLACK);
  tft.setColor(VGA_WHITE);
  
  tft.fillRoundRect(160, scene_top, 230, scene_top+40);
  tft.fillRoundRect(160, mode_top, 230, mode_top+40);
  tft.print(F("scene"), 158, scene_top+45);
  tft.print(F("mode"), 164, mode_top+45);
  tft.print(F("light"), 158, light_top);
  
  // button text
  tft.setBackColor(VGA_WHITE);
  tft.setColor(VGA_BLACK);
  
  tft.print(F(">"), 190, scene_top+12);

  if (mode == MODE_ON_OFF) {
    tft.print(F("0/1"), 173, mode_top+12);
  } else {
    tft.print(F("set"), 173, mode_top+12);
  }
}

void printColorStrokes() {
  // bars
  tft.setColor(VGA_WHITE);
  tft.fillRect(bar_width-2-2, 265+2, bar_width*2+2-2, 7);
  tft.fillRect(bar_width*3-2-2, 265+2, bar_width*4+2-2, 7);
  tft.fillRect(bar_width*5-2-2, 265+2, bar_width*6+2-2, 7);

  // color text
  int pad_top = 12;
  int box_height = 35;

  tft.setColor(VGA_WHITE);
  tft.fillRect(bar_width-2-2, 265+(pad_top-2), bar_width*6+2-2, 265+pad_top+box_height+2);
}

/*
 * USER ACTIONS
 */
void selectColor(int r, int g, int b) {
  selected_color[0] = r;
  selected_color[1] = g;
  selected_color[2] = b;
  
  for (int i = 0; i < 6; i++) {
    if (light_status[i][SELECT_STATUS]) {
      light_status[i][2] = selected_color[0];
      light_status[i][3] = selected_color[1];
      light_status[i][4] = selected_color[2];
      
      if (!light_status[i][ON_STATUS]) {
        // also turn the light on
        light_status[i][ON_STATUS] = 1;
      }
    }
  }
  
  setColorScreenToNewColor();
}

void selectCurrentColor() {
  // youd use this after selection to set all the newly selected lights to the current color
  selectColor(selected_color[0], selected_color[2], selected_color[2]);
}

void toggleMode() {
  if (mode == MODE_ON_OFF) {
    mode = MODE_SELECT;
  } else {
    mode = MODE_ON_OFF;
  }
  
  setupColorScreen();
}

void toggleLightClick(int light_num) {
  if (mode == MODE_ON_OFF) {
    if (light_status[light_num][ON_STATUS]) {
      light_status[light_num][ON_STATUS] = 0;
      light_status[light_num][SELECT_STATUS] = 0;
    } else {
      light_status[light_num][ON_STATUS] = 1;
      light_status[light_num][SELECT_STATUS] = 1;
    }
  } else {
    if (light_status[light_num][SELECT_STATUS]) {
      light_status[light_num][SELECT_STATUS] = 0;
    } else {
      light_status[light_num][SELECT_STATUS] = 1;
    }
  }
  
  printColorLights();
}

int getTouchTarget(int x, int y) {
  int touch_bounds[12][4] = {
     {0,0,50,270},      //red
     {51,0,100,270},    //green
     {101,0,150,270},   //blue
     {0,271,150,319},   //current color
     {151,0,239,75},    //scene button
     {151,76,239,150},  //mode button
     {151,151,197,200}, //light 0
     {198,151,239,200}, //light 1
     {151,201,197,245}, //light 2
     {198,201,239,245}, //light 3
     {151,246,197,290}, //light 4
     {198,246,239,290}  //light 5
  };
  
  for (int i = 0; i < 12; i++) {
     if (x >= touch_bounds[i][0] && y >= touch_bounds[i][1] && x <= touch_bounds[i][2] && y <= touch_bounds[i][3]) {
        return i;
     }
  }

  return -1;
}

void handleTouchTarget(int x, int y) {
  int set;
  int target = getTouchTarget(x, y);
  
  switch (target) {
    case 0:
      set = y-10;
      if (set > 255) { set = 255; }
      if (set < 0) { set = 0; }
      selectColor(255-set, selected_color[1], selected_color[2]);
      sendRequest();
      break;
    case 1:
      set = y-10;
      if (set > 255) { set = 255; }
      selectColor(selected_color[0], 255-set, selected_color[2]);
      sendRequest();
      break;
    case 2:
      set = y-10;
      if (set > 255) { set = 255; }
      selectColor(selected_color[0], selected_color[1], 255-set);
      sendRequest();
      break;
    case 3:
      selectCurrentColor();
      sendRequest();
      break;
    case 4:
      break;
    case 5:
      toggleMode();
      break;
    case 6:
      toggleLightClick(0);
      sendRequest();
      break;
    case 7:
      toggleLightClick(1);
      sendRequest();
      break;
    case 8:
      toggleLightClick(2);
      sendRequest();
      break;
    case 9:
      toggleLightClick(3);
      sendRequest();
      break;
    case 10:
      toggleLightClick(4);
      sendRequest();
      break;
    case 11:
      toggleLightClick(5);
      sendRequest();
      break;
  }
}

void sendRequest() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  
  JsonObject& light_0 = jsonBuffer.createObject();
  light_0["on"] = light_status[0][1];
  light_0["color"] = getHex(light_status[0][2], light_status[0][3], light_status[0][4]);
  light_0["bri"] = 100;
  data[light_map[0]] = light_0;
  
  JsonObject& light_1 = jsonBuffer.createObject();
  light_1["on"] = light_status[1][1];
  light_1["color"] = getHex(light_status[1][2], light_status[1][3], light_status[01][4]);
  light_1["bri"] = 100;
  data[light_map[1]] = light_1;
  
  JsonObject& light_2 = jsonBuffer.createObject();
  light_2["on"] = light_status[2][1];
  light_2["color"] = getHex(light_status[2][2], light_status[2][3], light_status[2][4]);
  light_2["bri"] = 100;
  data[light_map[2]] = light_2;
  
  JsonObject& light_3 = jsonBuffer.createObject();
  light_3["on"] = light_status[3][1];
  light_3["color"] = getHex(light_status[3][2], light_status[3][3], light_status[3][4]);
  light_3["bri"] = 100;
  data[light_map[3]] = light_3;
  
  JsonObject& light_4 = jsonBuffer.createObject();
  light_4["on"] = light_status[4][1];
  light_4["color"] = getHex(light_status[4][2], light_status[4][3], light_status[4][4]);
  light_4["bri"] = 100;
  data[light_map[4]] = light_4;
  
  JsonObject& light_5 = jsonBuffer.createObject();
  light_5["on"] = light_status[5][1];
  light_5["color"] = getHex(light_status[5][2], light_status[5][3], light_status[5][4]);
  light_5["bri"] = 100;
  data[light_map[5]] = light_5;

  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("send request");
    String content;

    data.printTo(content);
    client.println(F("POST /api/hue/scene HTTP/1.1"));
    client.println(F("Host: xxxxxxxxxxx"));
    client.println(F("Authorization: Basic xxxxxxxxxx"));
    client.println(F("Accept: */*"));
    client.println(F("Content-Type: application/json"));
    client.print("Content-Length: ");
    client.println(content.length());
    client.println();
    client.println(content);
  }
}

String getHex(int r, int g, int b) {
  char color_buffer[8] = {0};
  sprintf(color_buffer,"#%02X%02X%02X", r, g, b);
  return String(color_buffer);
}

/*
 * REPAINTING
 */
void setColorScreenToNewColor() {
  printColorBoxes();
  printColorPointers();
  printColorText();
  printColorLights();
}

void printColorLights() {
  tft.setColor(VGA_BLACK);
  tft.fillRect(151,150,240,300);
  
  for (int i = 0; i < 6; i++) {
    drawLight(i);
  }
}

void drawLight(int light_num) {
  int x = 175;
  int row = light_num / 2;
  int y = 180 + (row * 45);
  
  if (light_num % 2) {
    x = 218;
  }

  if (light_status[light_num][SELECT_STATUS]) {
    //this light is selected
    tft.setColor(VGA_WHITE);
    tft.fillCircle(x, y, 20);
  }

  if (light_status[light_num][ON_STATUS]) {
    drawLightColor(light_num);
    tft.fillCircle(x, y, 15);
  } else {
    if (light_status[light_num][SELECT_STATUS]) {
      tft.setColor(VGA_BLACK);
      tft.fillCircle(x, y, 15);
    } else {
      tft.setColor(VGA_WHITE);
      tft.drawCircle(x, y, 15);
    }
  }
}

void drawLightColor(int light_num) {
  tft.setColor(light_status[light_num][2], light_status[light_num][3], light_status[light_num][4]);
}

void printColorBoxes() {
  int step_size = 256/32;
  for (int i = 0; i < 256; i+=step_size) {
    int bottom = 265-i;
    int top = 265-(i+step_size);
    
    tft.setColor(i, selected_color[1], selected_color[2]);
    tft.fillRect(bar_width-2,bottom,bar_width*2-2,top);
    tft.setColor(selected_color[0], i, selected_color[2]);
    tft.fillRect(bar_width*3-2,bottom,bar_width*4-2,top);
    tft.setColor(selected_color[0], selected_color[1], i);
    tft.fillRect(bar_width*5-2,bottom,bar_width*6-2,top);
  }
}

void printColorText() {
  int pad_top = 12;
  int box_height = 35;

  // background
  tft.setColor(selected_color[0], selected_color[1], selected_color[2]);
  tft.fillRect(bar_width-2, 265+pad_top, bar_width*6-2, 265+pad_top+box_height);

  // text
  tft.setColor(VGA_WHITE);
  tft.setBackColor(selected_color[0], selected_color[1], selected_color[2]);

  // the text is always 110 wide
  tft.print(getHex(selected_color[0], selected_color[1], selected_color[2]),
            bar_width-2+(((bar_width*5)-110)/2), 265+pad_top+10);
}

void printColorPointers() {
  tft.setColor(VGA_WHITE);
  tft.fillRect(bar_width-2,(265+2-selected_color[0]),(bar_width*2)-2,(265-2-selected_color[0]));
  tft.fillRect(bar_width*3-2,(265+2-selected_color[1]),(bar_width*4)-2,(265-2-selected_color[1]));
  tft.fillRect(bar_width*5-2,(265+2-selected_color[2]),(bar_width*6)-2,(265-2-selected_color[2]));
}

void loop() {
  int x = random(20, 219);
  int y = random(20, 299);
  
  // draw the touch target
  tft.setColor(VGA_WHITE);
  tft.fillCircle(x, y, 20);
  delay(1000);

  // need to do this to clear the touch marker
  tft.clrScr();
  setupColorScreen();

  //actually handle the touch
  handleTouchTarget(x,y);
  
  delay(5000);
}

