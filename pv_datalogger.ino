/*
 * <<< Del Pierro Code >>>
 * PV Datalogger Project 
 * Group 2
 * 
 */

#include <Wire.h>
#include <Adafruit_INA219.h> 
#include <SPI.h>
#include <SD.h>
#include <elapsedMillis.h>
#include <LiquidCrystal.h>
#include <MyRealTimeClock.h>
#define timeLimit 300

/* 
 *  SD card attached to SPI bus as follows;
 *  MOSI to Pin 11 
 *  MISO to pin 12 
 *  CLK to pin 13  
 *  CS to pin 10
 */

Adafruit_INA219 ina219; // Declare and instance of INA219

elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs

LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
MyRealTimeClock myRTC(2, 9, 14);
 
const int ChipSelect = 10;

float volt; 
float curr;
float power;
double ld_resistance;
double lux;

byte p1[8] ={
0b10000, 0b01000,
0b00010, 0b00001,
0b00010, 0b00100,
0b01000, 0b10000
};


byte p2[8] ={
0b00001, 0b00010,
0b01000, 0b10000,
0b01000, 0b00100,
0b00010, 0b00001

};


void setup() {
  // put your setup code here, to run once:

  delay(100);

  // create animation character
  lcd.createChar(1, p1);  

  lcd.createChar(2, p2);

  // Begin the lcd and ina219
  lcd.begin(20, 4);
  ina219.begin();


  // Display welcome note 
  welcome_note();

  // Setting the necessary pin configuration 
  pinMode(14, INPUT);
  
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);


 // chipSelect pin for sd card
  pinMode(ChipSelect, OUTPUT);

digitalWrite(A1, LOW);
digitalWrite(A2, LOW);
digitalWrite(A3, LOW);


/* To set the current time and date in specific format 
 *  uncomment the following lines below
| Second 05 | Minute 06 | Hour 04| Day 24|  Month 05 | Year 2019 |
*/
//  myRTC.setDS1302Time(00, 41, 03, 3, 19, 06, 2019); 

 animate_message("Initializing the SD card...", 0);

delay(500);

lcd.setCursor(0, 1);

if (!SD.begin(ChipSelect)){
  
  lcd.print("Initializing failed!");
  lcd.setCursor(0, 2);
  lcd.print("Please try again");
  // do not perform any action 
  while(1);
}

// create file 'data.csv' and the neccesary headers in it 
  create_header("PVdata.csv");
  
}




void loop() {
   // read three sensors and append to the string:
  


  ld_resistance = ohmmeter();
  lux = ldr_lux(ld_resistance);
 voltage_current(volt, curr, power);
  

 String days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
lcd.clear();

 myRTC.updateTime();
 
 lcd.setCursor(0, 0);
 lcd.print("Date: ");
 
lcd.print(myRTC.dayofmonth); 
lcd.print("/"); 
lcd.print(myRTC.month); 
lcd.print("/");
lcd.print(myRTC.year); 
lcd.print("  ");
lcd.print(days[myRTC.dayofweek-1]); 

 
 lcd.setCursor(0, 1);
 lcd.print("Time: ");

 if (myRTC.hours < 10){
  lcd.print("0");
  }
 
 lcd.print(myRTC.hours);
lcd.print(":");

if (myRTC.minutes < 10){
  lcd.print("0");
  }
  
lcd.print(myRTC.minutes); 
lcd.print(":");

if (myRTC.seconds < 10){
  lcd.print("0");
  }
  
lcd.print(myRTC.seconds); 
 
 lcd.setCursor(0, 2);
 lcd.print("V: ");
 lcd.print(volt, 1);
  lcd.print("V");
 
 lcd.setCursor(10, 2);
 lcd.print("P: ");
 
 if(power > 100)
{
  
 lcd.print(power/1000, 1);
  lcd.setCursor(18, 2);
   lcd.print("W");
    
}else{
  lcd.print(power, 2);
   lcd.setCursor(18, 2);
   lcd.print("mW"); 
}
 

 lcd.setCursor(0, 3);

lcd.print("I: ");
if(curr > 100)
{
 lcd.print(curr/1000, 1);
   lcd.print("A");
    
}else{
  lcd.print(curr, 1);
   lcd.print("mA"); 
}
 
 lcd.setCursor(10, 3);
 //lcd.print("       ");
 lcd.print("L: ");
 lcd.print(lux, 1);
  lcd.setCursor(18, 3);
  lcd.print("Lx");





// Logs data every one hour
if ((timeElapsed/1000) >= timeLimit){
 save_data("PVdata.csv", curr, volt, lux, power);
  timeElapsed = 0;
}


delay(800);

}


/* --------   Methods Declaration ----------- */
/* ========================================== */


/* -----------------------------------------------------
 *  function for creating header of the table 
*  where the data is recorded
*  ======================================================
 */



void create_header(String filename){
  
  //create instance of the file 
  File datafile = SD.open(filename, FILE_WRITE);
  // create array of days 
  
  String days[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", 
                  "Friday", "Saturday", "Sunday"};
                      
  
  // if the file is available, write to it
  if (datafile) {
     datafile.println(",   ,   ,   ,   ,");
    datafile.println("Standalone Datalogger for photovoltaic system");
    datafile.println("PV  :,  Group 2 project ");
    datafile.print("Date : ,");
    
    myRTC.updateTime();

    datafile.print(myRTC.dayofmonth);
    datafile.print('/');
    datafile.print(myRTC.month);
    datafile.print('/');
    datafile.print (myRTC.year);
    datafile.println (",");

    datafile.print("Day : ,");
    datafile.println(days[myRTC.dayofweek - 1]);
    
    datafile.println(",   ,   ,   ,   ,");
    datafile.println("TIME,VOLTAGE (V),CURRENT (mA), LUX (lx), POWER (mW)");
    lcd.setCursor(0, 0);
    delay(100);
    datafile.close();
    
      // Display animations
animate_message(" Card accessed! ", 1);  
  
lcd.clear();
    
  }
    // if the file isn't open, pop up an error:
  else {
    lcd.setCursor(0, 1);
  lcd.print("error opening file!");
  
  // do not perform any action 
  while(1);
    
  }
}



/*  -----------------------------------------------
 *   for calculating ldr lux from measured resistance 
 *  ===============================================
 */


double ldr_lux(double R1){
  /*
   lux = R^m * 10^b;
   from sunrom;
   lux = 10 R = 9k;
   lux = 1000 R = 400
   therefore m = -1.479 b = 6.849

  */
  
 double lux = (7.06 * pow(10, 6))*pow(R1, -1.479);
 return lux;
  
}



/*  ---------------------------------------------------
 *  A method for determining the resistance of the ldr 
 *  auto ranging
 *  ==================================================
 */
 
double ohmmeter()
{
  
double Vout;  
double R2;
double Vin = 5.00;
double R13 = 5100;

 
digitalWrite(A3, HIGH);

delay(100);

int V1 = analogRead(A7);



Vout = V1 * 0.004888;
R2 = Vout/((Vin-0.63-Vout)/R13);

 
  return R2;
  
  }



/*
 *  -----------------------------------------------------
 *   Method for measuring voltage and current using  ina219
 *  =====================================================
 */

 
void voltage_current(float &loadVoltage, float &current, float &power){

  float  busVoltage = ina219.getBusVoltage_V(); // voltage measured in volts 
  current = ina219.getCurrent_mA(); // current measured in mA

  
  if (current < 0.8){
    current = 0.0;
  }

  
float shuntvoltage = ina219.getShuntVoltage_mV();  // voltage across shunt in mV
loadVoltage = busVoltage + (shuntvoltage / 1000);

  power = ina219.getPower_mW();
 
}



/*  ------------------------------------------
 *  A method for saving the data to the sd card
 *  ==========================================
 */
 

void save_data(String filename, float current, float volt, double lux, float power)
{
  File loadfile = SD.open(filename, FILE_WRITE);
  if (loadfile)
  {
    
    
    if (myRTC.hours < 10)
    {
      loadfile.print("0");
    }
    loadfile.print(myRTC.hours);
    loadfile.print(':');
    
    if (myRTC.minutes < 10)
    {
      loadfile.print("0");
    }
    loadfile.print(myRTC.minutes);
    loadfile.print(':');
    
    if (myRTC.seconds < 10)
    {
      loadfile.print("0");
    }
    loadfile.print(myRTC.seconds);
    loadfile.print (",");
    loadfile.print(volt);
    loadfile.print (",");
    loadfile.print(current);
    loadfile.print (",");
    loadfile.print(lux);
    loadfile.print (",");
    loadfile.print(power);
    loadfile.println (",");


    delay(100);

    loadfile.close();

// display animation
lcd.clear();

   for (int a = 1; a<3; ++a)
{
  lcd.setCursor(a, 1);
  lcd.write(2);
}
  lcd.setCursor(4, 1);

lcd.print("STORING DATA");

lcd.setCursor(0, 2);
 for (int a = 17; a<19; ++a)
{
  lcd.setCursor(a, 1);
  lcd.write(1);
}



for (int i = 0; i<20; i=i+2)
{

  lcd.setCursor(i-1, 2);
  lcd.print("  ");
lcd.write(1);
lcd.write(1);
delay(100);
}

lcd.clear();

  }
}




/*  ---------------------------------------
 *  A method for displaying the welcome note
 *  =======================================
 */


void welcome_note(){
  for (int a = 1; a<4; ++a)
{
  lcd.setCursor(a, 1);
  lcd.write(2);
}


  lcd.setCursor(5, 1);
lcd.print("Loading");

int timing = 1;

lcd.setCursor(0, 2);

 for (int a = 16; a<19; ++a)
{
  lcd.setCursor(a, 1);
  lcd.write(1);
}

for (int i = 0; i<20; ++i)
{
  if(timing <=3){
    lcd.setCursor(11+timing, 1);
    lcd.print(".");
    delay(200);
    timing = timing + 1;
    
  }

  if(timing >3){
    lcd.setCursor(12, 1);
    lcd.print("   ");
    delay(100);
    timing = 1;
  }
  
 
  lcd.setCursor(i-1, 2);
  lcd.print(" ");
lcd.write(1);
}

lcd.clear();
}


/*
 *  Display animation message
 */
 
void animate_message(String message, int line){
  
  lcd.clear();
  lcd.setCursor(0, 1);
lcd.print(message);
lcd.setCursor(0, 3);
for (int i = 1; i<18; ++i){

  lcd.setCursor(i-1, 2);
  if(line == 1) lcd.print("-");
  lcd.print(" ");
lcd.write(1);

if(line == 0) lcd.write(1);

delay(200);
}

lcd.clear();

}
