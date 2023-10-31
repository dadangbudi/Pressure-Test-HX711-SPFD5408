#include <HX711_ADC.h>

#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define YP A3 //A3 
#define XM A2
#define YM 8 //8 
#define XP 9 //7




#define SENSITIVITY 300
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define TS_MINX 125
#define TS_MINY 85
#define TS_MAXX 965
#define TS_MAXY 905

#define sVersion "(V4s239v1)"

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, SENSITIVITY);

const int HX711_dout_1 = 14; //mcu > HX711 no 1 dout pin
const int HX711_sck_1 = 15; //mcu > HX711 no 1 sck pin
const int HX711_dout_2 = 16; //mcu > HX711 no 2 dout pin
const int HX711_sck_2 = 17; //mcu > HX711 no 2 sck pin
const int HX711_dout_3 = 18; //mcu > HX711 no 3 dout pin
const int HX711_sck_3 = 19; //mcu > HX711 no 3 sck pin
const int HX711_dout_4 = 20; //mcu > HX711 no 4 dout pin
const int HX711_sck_4 = 21; //mcu > HX711 no 4 sck pin


int LED1 = 31; // LED GREEN
int LED2 = 33; // LED RED
int LED3 = 35; // LED BLUE
int RELAY = 37; // OK SIGNAL
int piezo = 39;
int iOK = 0;
int iNG = 0;
int iCx = 0;
int iCy = 0;
int iCz = 0;

float a = 0;
float b = 0;
float c = 0;
float d = 0;
int Sa = 0;
int Sb = 0;
int Sc = 0;
int Sd = 0;

int iRelay = 0;
int portRelay = 53;


const int minNewton = 200;
const int waitTime = 1450;
unsigned long tStart = 0;
float maxA = 0;
float maxB = 0;
float maxC = 0;
float maxD = 0;

int laststatus = 5;
int currstatus = 0;
TSPoint p;

HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1); //HX711 1
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2); //HX711 2
HX711_ADC LoadCell_3(HX711_dout_3, HX711_sck_3); //HX711 3
HX711_ADC LoadCell_4(HX711_dout_4, HX711_sck_4); //HX711 4

unsigned long t = 0;

void setup() {
  Serial.begin(9600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  tft.reset();
  tft.begin(0x9341);
  tft.fillScreen(BLACK);
  Serial.print(tft.width());
  Serial.print("/");
  Serial.println(tft.height());
  tft.setRotation(1); //0,1,2,3
  Serial.print(tft.width());
  Serial.print("/");
  Serial.println(tft.height());

  tft.fillScreen(BLUE);
  tft.drawRect(0,0,319,239,WHITE);    
  tft.setTextColor(RED);  tft.setTextSize(3);  tft.setCursor(30, 45);  tft.println("Pressure Tester");
  tft.setTextColor(GREEN);  tft.setTextSize(3);  tft.setCursor(30, 150);  tft.println("Callibrating..."); 
    

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(piezo, OUTPUT);
  pinMode(portRelay,OUTPUT);

  float calibrationValue_1; // calibration value load cell 1
  float calibrationValue_2; // calibration value load cell 2
  float calibrationValue_3; // calibration value load cell 2
  float calibrationValue_4; // calibration value load cell 2

  calibrationValue_1 = 733.0; // uncomment this if you want to set this value in the sketch
  calibrationValue_2 = 733.0; // uncomment this if you want to set this value in the sketch
  calibrationValue_3 = 733.0; // uncomment this if you want to set this value in the sketch
  calibrationValue_4 = 733.0; // uncomment this if you want to set this value in the sketch


  LoadCell_1.begin();
  LoadCell_2.begin();
  LoadCell_3.begin();
  LoadCell_4.begin();

  //LoadCell_1.setGain(32);
  //LoadCell_2.setGain(32);

  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  byte loadcell_3_rdy = 0;
  byte loadcell_4_rdy = 0;

  while ((loadcell_1_rdy + loadcell_2_rdy + loadcell_3_rdy + loadcell_4_rdy) < 4) { //run startup, stabilization and tare, both modules simultaniously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
    if (!loadcell_3_rdy) loadcell_3_rdy = LoadCell_3.startMultiple(stabilizingtime, _tare);
    if (!loadcell_4_rdy) loadcell_4_rdy = LoadCell_4.startMultiple(stabilizingtime, _tare);

  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
  }
  if (LoadCell_3.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.3 wiring and pin designations");
  }
  if (LoadCell_4.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.4 wiring and pin designations");
  }

  LoadCell_1.setCalFactor(calibrationValue_1); // user set calibration value (float)
  LoadCell_2.setCalFactor(calibrationValue_2); // user set calibration value (float)
  LoadCell_3.setCalFactor(calibrationValue_3); // user set calibration value (float)
  LoadCell_4.setCalFactor(calibrationValue_4); // user set calibration value (float)

  Serial.println("Startup is complete");
}

void loop() {

  static boolean newDataReady = 0;
  const int serialPrintInterval = 100; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  //if (LoadCell_1.update()) newDataReady = true;

  //LoadCell_2.update();
  //get smoothed value from data set
  //if ((newDataReady)) {
  if (millis() > t + serialPrintInterval) {
    LoadCell_1.update();
    LoadCell_2.update();
    LoadCell_3.update();
    LoadCell_4.update();

    a = LoadCell_1.getData();
    b = LoadCell_2.getData();
    c = LoadCell_3.getData();
    d = LoadCell_4.getData();


    
    newDataReady = 0;

    p = ts.getPoint();
    pinMode(XM, OUTPUT); //Pins configures again for TFT control
    pinMode(YP, OUTPUT);

    if (iCx != p.x)
    {
      tft.setTextColor(BLACK);  tft.setTextSize(1);  tft.setCursor(10, 130);  tft.print(iCx);
      tft.setTextColor(GREEN);  tft.setTextSize(1);  tft.setCursor(10, 130);  tft.print(p.x);
      iCx = p.x;
    }
    if (iCy != p.y)
    {
      tft.setTextColor(BLACK);  tft.setTextSize(1);  tft.setCursor(10, 145);  tft.print(iCy);
      tft.setTextColor(GREEN);  tft.setTextSize(1);  tft.setCursor(10, 145);  tft.print(p.y);
      iCy = p.y;
    }

    if (iCz != p.z)
    {
      tft.setTextColor(BLACK);  tft.setTextSize(1);  tft.setCursor(10, 160);  tft.print(iCz);
      tft.setTextColor(GREEN);  tft.setTextSize(1);  tft.setCursor(10, 160);  tft.print(p.z);
      iCz = p.z;
    }


    t = millis();
  }
  //}
  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
      LoadCell_3.tareNoDelay();
      LoadCell_4.tareNoDelay();

    }
  }
  //check if last tare operation is complete
  if (LoadCell_1.getTareStatus() == true) {
    Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare load cell 2 complete");
  }

  if (LoadCell_3.getTareStatus() == true) {
    Serial.println("Tare load cell 3 complete");
  }

  if (LoadCell_4.getTareStatus() == true) {
    Serial.println("Tare load cell 4 complete");
  }

  if (a < minNewton && b < minNewton && c < minNewton && d < minNewton)
  {
    currstatus = 0;
    maxA = 0;
    maxB = 0;
    maxC = 0;
    maxD = 0;

  }
  else if (a > minNewton || b > minNewton || c > minNewton || d > minNewton)
  {
    tStart = millis();
    maxA = a;
    maxB = b;
    maxC = c;
    maxD = d;
    while (millis() <= tStart + waitTime)
    {
      LoadCell_1.update();
      LoadCell_2.update();
      LoadCell_3.update();
      LoadCell_4.update();
      a = LoadCell_1.getData();
      b = LoadCell_2.getData();
      c = LoadCell_3.getData();
      d = LoadCell_4.getData();
      
      if (a > maxA)
        maxA = a;
      if (b > maxB)
        maxB = b;
      if (c > maxC)
        maxC = c;
      if (d > maxD)
        maxD = d;

      if (maxA > minNewton && maxB > minNewton  && maxC > minNewton  && maxD > minNewton)
        break;

    }


  }


  if (maxA < minNewton && maxB < minNewton && maxC < minNewton && maxD < minNewton)
    currstatus = 0;
  else if ((maxA > minNewton || maxB > minNewton || maxC > minNewton || maxD > minNewton) && (maxA < minNewton || maxB < minNewton || maxC < minNewton || maxD < minNewton))
    currstatus = 1;
  else if (maxA > minNewton || maxB > minNewton || maxC > minNewton || maxD > minNewton)
    currstatus = 3;
  else
    currstatus = 5;

 



  if (currstatus == 0 && currstatus != laststatus)
  {
    tft.fillScreen(BLACK);
    tft.drawRect(0, 0, 159, 119, WHITE);    tft.drawRect(160, 0, 159, 119, WHITE);
    tft.drawRect(160, 120, 159, 119, WHITE);    tft.drawRect(0, 120, 159, 119, WHITE);
    tft.setTextColor(GREEN);  tft.setTextSize(2);  tft.setCursor(5, 10);  tft.print(iOK);
    tft.setTextColor(RED);  tft.setTextSize(2);  tft.setCursor(5, 30);  tft.print(iNG);    
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 
    tft.setTextColor(YELLOW);  tft.setTextSize(3);  tft.setCursor(45, 15);  tft.print("H1");
    tft.setTextColor(GREEN);  tft.setTextSize(3);  tft.setCursor(25, 50);  tft.print("READY");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(25, 75);  tft.print(a);
    tft.setTextColor(YELLOW);  tft.setTextSize(3);  tft.setCursor(225, 15);  tft.print("H2");
    tft.setTextColor(GREEN);  tft.setTextSize(3);  tft.setCursor(205, 50);  tft.print("READY");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(205, 75);  tft.print(b);
    tft.setTextColor(YELLOW);  tft.setTextSize(3);  tft.setCursor(45, 130);  tft.print("H3");
    tft.setTextColor(GREEN);  tft.setTextSize(3);  tft.setCursor(25, 165);  tft.print("READY");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(25, 190);  tft.print(c);
    tft.setTextColor(YELLOW);  tft.setTextSize(3);  tft.setCursor(225, 130);  tft.print("H4");
    tft.setTextColor(GREEN);  tft.setTextSize(3);  tft.setCursor(205, 165);  tft.print("READY");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(205, 190);  tft.print(d);
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(170, 220);  tft.print(sVersion);
    Sa=0;
    Sb=0;
    Sc=0;
    Sd=0;
        
    Serial.print("M ");
    Serial.print("1,");
    //Serial.print(a);
    //Serial.print(",");
    Serial.print(maxA);
    Serial.print(",");
    Serial.print(Sa);   
    Serial.print(" 2,");
    //Serial.print(b);
    //Serial.print(",");
    Serial.print(maxB);
    Serial.print(",");
    Serial.print(Sb);        
    Serial.print(" 3,");
    //Serial.print(c);
    //Serial.print(",");
    Serial.print(maxC);
    Serial.print(",");
    Serial.print(Sc); 
    Serial.print(" 4,");
    //Serial.print(d);
    //Serial.print(",");
    Serial.print(maxD);
    Serial.print(",");
    Serial.println(Sd); 
    
    
    delay(10);
   // Serial.println(" 00 YELLOW LED ON");
    digitalWrite(LED1, HIGH); //RED LED On
    digitalWrite(LED2, HIGH); //RED LED On
    delay(10);
  //  Serial.println(" 00 YELLOW LED OFF");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    delay(10);
  }
  else if (currstatus == 1 && currstatus != laststatus)
  {
    if(maxA < minNewton)
    {
    tft.fillRect(0, 0, 159, 119, RED);
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(45, 15);  tft.print("H1");
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(25, 40);  tft.print("FAIL!");
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(25, 70);  tft.print(maxA);
    Sa = 2;
    }
    else
    {
    tft.fillRect(0, 0, 159, 119, GREEN);
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(45, 15);  tft.print("H1");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(25, 40);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(25, 70);  tft.print(maxA);
    Sa = 1;
    }    
    
    if(maxB < minNewton)
    {   
    tft.fillRect(160, 0, 159, 119, RED);
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(225, 15);  tft.print("H2");
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(205, 40);  tft.print("FAIL!");
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(205, 70);  tft.print(maxB);
    Sb = 2;
    }    
    else
    {   
    tft.fillRect(160, 0, 159, 119, GREEN);
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(225, 15);  tft.print("H2");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(205, 40);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(205, 70);  tft.print(maxB);
    Sb = 1;
    }

    if(maxC < minNewton)
    {       
    tft.fillRect(0, 120, 159, 119, RED);    
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(45, 130);  tft.print("H3");
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(25, 155);  tft.print("FAIL!");
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(25, 185);  tft.print(maxC);
    Sc = 2;
    }    
    else
    {      
    tft.fillRect(0, 120, 159, 119, GREEN);     
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(45, 130);  tft.print("H3");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(25, 155);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(25, 185);  tft.print(maxC);
    Sc = 1;
    }    

    if(maxD < minNewton)
    {
    tft.fillRect(160, 120, 159, 119, RED);    
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(225, 130);  tft.print("H4");
    tft.setTextColor(WHITE);  tft.setTextSize(3);  tft.setCursor(205, 155);  tft.print("FAIL!");
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(205, 185);  tft.print(maxD);
    Sd = 2;
    }
    else
    {
    tft.fillRect(160, 120, 159, 119, GREEN);    
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(225, 130);  tft.print("H4");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(205, 155);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(205, 185);  tft.print(maxD);
    Sd = 1;
    }    
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(170, 220);  tft.print(sVersion);
    iNG++;
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 10);  tft.print(iOK);
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 30);  tft.print(iNG); 
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 


    Serial.print("N ");
    Serial.print("1,");
    //Serial.print(a);
    //Serial.print(",");
    Serial.print(maxA);
    Serial.print(",");
    Serial.print(Sa);   
    Serial.print(" 2,");
    //Serial.print(b);
    //Serial.print(",");
    Serial.print(maxB);
    Serial.print(",");
    Serial.print(Sb);        
    Serial.print(" 3,");
    //Serial.print(c);
    //Serial.print(",");
    Serial.print(maxC);
    Serial.print(",");
    Serial.print(Sc); 
    Serial.print(" 4,");
    //Serial.print(d);
    //Serial.print(",");
    Serial.print(maxD);
    Serial.print(",");
    Serial.println(Sd);     
    
    delay(10);
  //  Serial.println(" 10 RED LED ON");
    digitalWrite(LED2, HIGH); //RED LED On
    delay(10);
   // Serial.println(" 10 RED LED OFF"); //RED LED OFF
    //digitalWrite(LED2, LOW);
    digitalWrite(piezo, 1);
    delay(1000);

    p = ts.getPoint();
    pinMode(XM, OUTPUT); //Pins configures again for TFT control
    pinMode(YP, OUTPUT);
    while (p.y <= 50)
    {
      p = ts.getPoint();
      pinMode(XM, OUTPUT); //Pins configures again for TFT control
      pinMode(YP, OUTPUT);
      delay(10);
    }
    digitalWrite(piezo, 0);
    delay(100);
  }
  else if (currstatus == 3 && currstatus != laststatus)
  {

    tft.fillRect(0, 0, 159, 119, GREEN);
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(45, 15);  tft.print("H1");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(25, 40);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(25, 70);  tft.print(maxA);
    tft.fillRect(160, 0, 159, 119, GREEN);
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(225, 15);  tft.print("H2");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(205, 40);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(205, 70);  tft.print(maxB);
    tft.fillRect(0, 120, 159, 119, GREEN);     
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(45, 130);  tft.print("H3");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(25, 155);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(25, 185);  tft.print(maxC);
    tft.fillRect(160, 120, 159, 119, GREEN);    
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(225, 130);  tft.print("H4");
    tft.setTextColor(BLUE);  tft.setTextSize(3);  tft.setCursor(205, 155);  tft.print("PASS!");
    tft.setTextColor(BLUE);  tft.setTextSize(2);  tft.setCursor(205, 185);  tft.print(maxD);

    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(170, 220);  tft.print(sVersion);
    iOK++;    
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 10);  tft.print(iOK);
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 30);  tft.print(iNG); 
    tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 
    Sa=1;
    Sb=1;
    Sc=1;
    Sd=1;

    Serial.print("O ");
    Serial.print("1,");
    //Serial.print(a);
    //Serial.print(",");
    Serial.print(maxA);
    Serial.print(",");
    Serial.print(Sa);   
    Serial.print(" 2,");
    //Serial.print(b);
    //Serial.print(",");
    Serial.print(maxB);
    Serial.print(",");
    Serial.print(Sb);        
    Serial.print(" 3,");
    //Serial.print(c);
    //Serial.print(",");
    Serial.print(maxC);
    Serial.print(",");
    Serial.print(Sc); 
    Serial.print(" 4,");
    //Serial.print(d);
    //Serial.print(",");
    Serial.print(maxD);
    Serial.print(",");
    Serial.println(Sd); 
    
    delay(10);
  //  Serial.println(" 11 Green LED ON");
    digitalWrite(LED1, HIGH); //Green LED On
    digitalWrite(RELAY, LOW); //SIGNAL On
    digitalWrite(piezo, 1);
    delay(200);
    digitalWrite(piezo, 0);
    delay(50);
    digitalWrite(piezo, 1);
    delay(200);
    digitalWrite(piezo, 0);
    delay(100);
    delay(2000);
    LoadCell_1.tare();
    LoadCell_2.tare();
    LoadCell_3.tare();
    LoadCell_4.tare();
 //   Serial.println(" 11 Green LED OFF");
    digitalWrite(LED1, LOW); //Green LED OFF
    digitalWrite(RELAY, LOW); //SIGNAL OFF
    delay(10);
  }
  else
  {
    if (currstatus != laststatus)
    {
      tft.fillScreen(BLACK);
      tft.drawRect(0, 0, 319, 239, YELLOW);
      tft.setTextColor(YELLOW);  tft.setTextSize(3);  tft.setCursor(30, 80);  tft.print("Not connected  ");
      tft.setTextColor(YELLOW);  tft.setTextSize(3);  tft.setCursor(30, 150);  tft.print(" to sensor!! ");
  //    Serial.println(" error - BLUE LED On");
      digitalWrite(LED3, HIGH); //BLUE LED On
      delay(10);
   //   Serial.println(" error - BLUE LED off");
      digitalWrite(LED3, LOW); //BLUE LED OFF

      delay(10);
    }
  }

    if(Serial.available()>0)
    {
        char inChar = (char)Serial.read();
        if (inChar=='N')
        {
              digitalWrite(portRelay, HIGH);
              tft.setTextColor(BLACK);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 
              iRelay = 1;
              tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 
              //Serial.println("ON");
              
        }
        if (inChar=='O')
        {
           digitalWrite(portRelay, LOW);
           tft.setTextColor(BLACK);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 
           iRelay = 0;
           tft.setTextColor(WHITE);  tft.setTextSize(2);  tft.setCursor(5, 50);  tft.print(iRelay); 
           //Serial.println("OFF");
        }
         //NNNNSerial.println(inChar);
         
    }


    delay(100);

    
  laststatus = currstatus;
}
