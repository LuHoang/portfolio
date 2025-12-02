#include <Wire.h>
#include "INA226.h"
#include <U8glib.h>
#include <EEPROM.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 13

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;


//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
// U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI

INA226 INA(0x40);

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;


float last_1min_mw = 0;

float last_30s_wh;
float last1min_wh;
float last_2min_wh;
float last_30min_wh;
float last_1hour_wh;
float last_12hour_wh;
float last_24hour_wh;


//BATTERY 
boolean charging = false;
float charge_effciency = 0.9497; //@ 55W
float bat_voltage;
const long InternalReferenceVoltage = 1088L;
float mAh;
float mAh_compute;  //the different between mW output from i2c vs compute
float mWh;
float mWh_compute;      //the different between mW output from i2c vs compute
float Wh = 0;           // limit between 0 and real Wh
float mWh_nolimit = 0;  // limit at 0 but not at the top end, it will never goes pass zero, but will goes pass Wh_real during charge
float charge_Wh = 0;
float cal_discharge_Wh = 0;
float default_Wh = 266;
//float compute_voltage = 16.8;

boolean bat_cal = false;
boolean last_bat_cal = false;
float bat_cal_current = 20;  // mA
int bat_percent;
int progress = 0;  // battery progress bar
boolean bat_int_cal = false;
boolean last_bat_int_cal = false;

boolean screen_update = 0;


boolean display_on = 1;
int screen_timeout = 30000;

float total;
unsigned long count;
unsigned long last_timer1;
unsigned long last_timer2;

const unsigned long INTERVAL = 60000;  // one minute



unsigned long duration;
unsigned long last_millis = 0;

//TEMP 

//debug button

int button_up = 3;
int button_down = 6;
int button_ok = 7;
int button_back = 8;


// byte pic = 1;
int max_page = 15;


int lastButton3State;     // the previous state of button
int currentButton3State;  // the current state of button
int button1state = 0;
int button2state = 0;

int button3state = 0;
int button4state = 0;

boolean currentState = false;
boolean fan_state = false;
int fanSpeed = 100;
boolean mosfet_state = false;

int mosfet = 4;
boolean fan_mode = 1;
// Pins D9 and D10 - 30 Hz


float temp1 = 0;
float temp1F = 0;

//screen
boolean selection = false;
boolean flashing = true;
float screen_update_time = 1000;
int screen_last_pic = 1000;
int screen_power_on = 0;
int screen_last_title = 1000;



int bat_x = 88;
int bat_y = 0;
int bat_width = 40;
int bat_height = 8;
int bat_ver_padding = 2;
int bat_hor_padding = 2;
int bat_width_max = bat_width - bat_hor_padding * 2;



int fan = 9;

int childs = 0;

int last_button3state = 0;


int button3count = 0;


int pic = 1000;
int maxPics_L1 = 5;
int maxPics_L2 = 3;

// int button_brd = 12;

// OneButton button(button_brd,true);

long lastmillis = 0;
long maxtime = 20000;

int last_pic = 0;
int current_pic = 0;

unsigned long last_screen_update = 0;
#include <ezButton.h>

ezButton button1(button_up);
ezButton button2(button_down);
ezButton button3(button_ok);
ezButton button4(button_back);




int resolution = 11;
unsigned long lastTempRequest = 0;
int delayInMillis = 3000;
float temperature = 0.0;
int idle = 0;
//eeprom

struct MyData {
  int ini; 
  int fan_temp_on;
  int fan_power_on;
  int fan_speed;
  float mosfet_voltage_cutoff;
  float real_Wh;
  float bat_cycle;
  float min_voltage;
  float max_voltage; 
  float total_discharge;
  float temp_offset;

};
//defaul WH
int d_ini = 1;
int d_fan_temp_on = 50;
int d_fan_power_on = 10;
int d_fan_speed = 255;
float d_mosfet_voltage_cutoff = 12.4;
int d_real_Wh = 250;
float d_bat_cycle = 0;
float d_min_voltage = 12.4;   //12.4
float d_max_voltage = 16.64; 
float d_total_discharge = 0.000;
float d_temp_offset = -2.53;

int ini = 1;
int fan_temp_on = 50;
int fan_power_on = 10;
int fan_speed = 255;
float mosfet_voltage_cutoff = 12.4;
int real_Wh = 250;
float bat_cycle = 0;
float min_voltage = 12.4;   //12.4
float max_voltage = 16.64; 
float total_discharge = 0.000;
float temp_offset = -2.53;

   MyData myData;
  int address = 0; // Starting address in EEPROM


void saveData(const MyData& data, int address) {
    myData.ini = ini;
    myData.fan_temp_on = fan_temp_on;
    myData.fan_power_on = fan_power_on;
    myData.fan_speed = fan_speed;
    myData. mosfet_voltage_cutoff = mosfet_voltage_cutoff;
    myData.real_Wh = real_Wh;
    myData.bat_cycle = bat_cycle;
    myData.min_voltage = min_voltage;   //12.4
    myData.max_voltage = max_voltage; 
    myData.total_discharge = total_discharge;
    myData.temp_offset = temp_offset;
    EEPROM.put(address, data);
}

void loadData(MyData& data, int address) {
  EEPROM.get(address, data);
}
bool anyNaN(const MyData& data) {
  return isnan(data.ini) || isnan(data.fan_temp_on) || isnan(data.fan_power_on) || isnan(data.fan_speed) || isnan(data.mosfet_voltage_cutoff) || isnan(data.real_Wh) || isnan(data.bat_cycle)
  || isnan(data.min_voltage) || isnan(data.max_voltage)|| isnan(data.total_discharge) || isnan(data.temp_offset);

}
void defaultValues(){
    myData.ini = d_ini;
    myData.fan_temp_on = d_fan_temp_on;
    myData.fan_power_on = d_fan_power_on;
    myData.fan_speed = d_fan_speed;
    myData. mosfet_voltage_cutoff = d_mosfet_voltage_cutoff;
    myData.real_Wh = d_real_Wh;
    myData.bat_cycle = d_bat_cycle;
    myData.min_voltage = d_min_voltage;   //12.4
    myData.max_voltage = d_max_voltage; 
    myData.total_discharge = d_total_discharge;
    myData.temp_offset = d_temp_offset;

}



void setup() {

  TCCR1A = 0b00000001; // 8bit
  TCCR1B = 0b00000101; // x1024 phase correct
  // TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)

   loadData(myData, address);

  // Print loaded data
   if (anyNaN(myData)) {
      //memset(&myData, 0, sizeof(myData));

      defaultValues();    // and initialize your data
      saveData(myData, address);
      loadData(myData, address);
      Serial.println("Non good data in EEPROM, load default");

  } else {
    //real_Wh = EEPROM.get(0, real_Wh);
        Serial.println("REAL DATA:");

  }

    ini = myData.ini;
    fan_temp_on = myData.fan_temp_on;
    fan_power_on = myData.fan_power_on;
    fan_speed= myData.fan_speed;
    mosfet_voltage_cutoff=myData.mosfet_voltage_cutoff;
    real_Wh= myData.real_Wh;
    bat_cycle =myData.bat_cycle;
    min_voltage =myData.min_voltage;   //12.4
    max_voltage=myData.max_voltage ; 
    total_discharge=myData.total_discharge;
    temp_offset =myData.temp_offset ;


  Serial.begin(9600);
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  // delayInMillis = 750 / (1 << (12 - resolution));
  lastTempRequest = millis();


  Wire.begin();
  if (!INA.begin()) {
    Serial.println("could not connect. Fix and Reboot");
  }
  INA.setMaxCurrentShunt(10, 0.002, false);
  INA.setAverage(5);
  // Serial.print("LSB:\t");
  // Serial.println(INA.getCurrentLSB(), 10);
  // Serial.print("LSB_uA:\t");
  // Serial.println(INA.getCurrentLSB_uA(), 3);
  // Serial.print("shunt:\t");
  // Serial.println(INA.getShunt(), 3);
  // Serial.print("maxCur:\t");
  // Serial.println(INA.getMaxCurrent(), 3);
  // Serial.print("get average:\t");
  // Serial.println(INA.getAverage());
  // Serial.println();



  // real_Wh = EEPROM.get(0, real_Wh);

  // // if (isnan(real_Wh)) {
  // //   real_Wh = default_Wh;
  // // } else {
  // //   //real_Wh = EEPROM.get(0, real_Wh);
  // // }


  // pinMode(button1, INPUT_PULLUP);
  // pinMode(button2, INPUT_PULLUP);
  // pinMode(button3, INPUT_PULLUP);
  // pinMode(button4, INPUT_PULLUP);

  pinMode(fan, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(mosfet, LOW);
  digitalWrite(fan, LOW);

  last_timer1 = millis();
}
//------------------------------------------------------------------------

void ina226_raw() {
  busvoltage = INA.getBusVoltage();
  shuntvoltage = INA.getShuntVoltage_mV();
  current_mA = INA.getCurrent_mA();
  power_mW = busvoltage * current_mA;
}

//-------------------------------------------------------------------------------

void mAh_mWh(unsigned long interval_time) {
  if (power_mW < 0 && mWh <= 0) {
    mWh = 0;

  } else if (mWh >= (real_Wh * 1000) && power_mW > 0) {
    mWh = (real_Wh * 1000);
  } else {
    mWh += power_mW * ((float)interval_time / 3600000);  // in millis, depends on the interval


    // mWh_compute += loadvoltage * current_mA * ((float)interval_time / 3600000);
  }
  // for no limit mWh
  if (power_mW < 0 && mWh_nolimit <= 0) {

    mWh_nolimit = 0;

  } else {
    mWh_nolimit += power_mW * ((float)interval_time / 3600000);  // in millis, depends on the interval
  }
}

//-----------------------------------------------------------------------------------
void batteryCalibration(unsigned long duration) {



  if (busvoltage >= max_voltage && current_mA <= bat_cal_current) {

    bat_cal = true;

  }

  if (busvoltage >= max_voltage && current_mA <= 50 && current_mA >= 1) {
    mWh = real_Wh * 1000;
  }


  if (bat_cal == true && current_mA <= 100) {

    cal_discharge_Wh += power_mW * ((float)duration / 3600000) / 1000;

  } else if (bat_cal == true && current_mA > 100) {  // set back calibration to zero if there is a charge in above x_ma during calibration
    bat_cal = false;
    cal_discharge_Wh = 0;
  }

  if (busvoltage <= min_voltage && bat_cal == true && current_mA >= (-100)) {

    bat_cal = false;
    real_Wh = abs(cal_discharge_Wh);
    Serial.print(real_Wh);
    EEPROM.put(0, real_Wh);

    cal_discharge_Wh = 0;
    mWh = 0;
    mWh_nolimit = 0;
  }
  if (busvoltage <= min_voltage && bat_cal == false && current_mA >= -100) {

    mWh = 0;
    mWh_nolimit = 0;
  }
}
//-----------------------------


//----------------------------
void loop() {
  //--------------------------------------------------------------------------------





  //-------
  unsigned long start = millis();

  //How many childs of each main menu slection
  static int main_menu[6] = { 5, 2, 8, 1, 1 };

  button1.loop();
  button2.loop();
  button3.loop();
  button4.loop();

  // button3count = button3.getCount();
  //  if (button1.getState() == LOW && button1state == 0) button1state = 1;
  // if (button2.getState() == LOW && button2state == 0) button2state = 1;
  // if (button3.getState() == LOW && button3state == 0) button3state = 1;
  // if (button4.getState() == LOW && button4state == 0) button4state = 1;


if (display_on == 1) { 

  //button1

  


  if (button1.isPressed()) {  //click up
      Serial.println(pic);

    screen_update = 1;
    button1state = 0;
    lastmillis = millis();
    if (pic < 10) {  //(pic >= -1 && pic < 10) {
      pic--;
      if (pic <= 0) { pic = maxPics_L1; }
    }



    if (pic >= 1000) {
      //MainMenu
      if (pic >= 1002) {
        pic = 1000;
      } else if (pic < 1010){
        pic++;
      }
    }


    if (pic >= 10 && pic < 100) {
      pic = pic - 10;
      if (pic == ((pic % 10))) {  //(pic = ((pic%10)*childs+10+last_pic)) {//  1*10*main menu child+10+lastpic example = 41.   11 =
        pic = (childs * 10) + (pic % 10);
        
        // 31 = 1+ (3*10)  = 31
        // 21 = 1 + (2*10) = 21
        // 32 = 2 + (3*10) = 32
      }
    }
  }
  //button2
  if (button2.isPressed()) {  //click down
    if (pic >= 1000) { mosfet_state = !mosfet_state; }
    screen_update = 1;

    button2state = 0;
    lastmillis = millis();


    if (pic >= 1 && pic < 10) {
      //MainMenu
      if (pic >= maxPics_L1) {
        pic = 1;
      } else if (pic < maxPics_L1) {
        pic++;
      }
    }

    if (pic >= 10 && pic < 100) {
      pic = pic + 10;

      if (pic == (childs * 10 + 10 + last_pic)) {
    
        //Goal 41, 42, etc.  //lastpic 1 pic%10 1  childs 3 // childs * 10 + 10 + last_pic = 41
        pic = last_pic + 10;
        //Goal 11 , 12 , 13 , etc. For 12: lastpic + 10
      }
    }
  }


  if (button3.isPressed()) {  //longPressStart-old not work  //more like button3
                              //  if (pic >=1000) {fan_state = !mosfet_state;}
    screen_update = 1;

    lastmillis = millis();



    if (pic <= 10) {
      last_pic = pic;

      if (last_pic == 1) {
        childs = main_menu[0];
      }
      if (last_pic == 2) {
        childs = main_menu[1];
      }
      if (last_pic == 3) {
        childs = main_menu[2];
      }
      if (last_pic == 4) {
        childs = main_menu[3];
      }
      if (last_pic == 5) {
        childs = main_menu[4];
      }
    }
    if (pic > 0 & pic < 10) {
      pic = pic + 10;
    } else {
      if (pic == 11) { fan_mode = !fan_mode; }
      if (pic == 21) {
        fan_mode = 0;
        fan_state = !fan_state;
      }
      if (pic == 12) { mosfet_state = !mosfet_state; }
      if (pic == 22) {
        mosfet_voltage_cutoff += 0.1;
        if (mosfet_voltage_cutoff >= 13) mosfet_voltage_cutoff = 11.8;
      }
  
    }
    button3state = 0;
  }

  if (button3.getState() == LOW){
        screen_update = 1;

    if (pic == 51) {
        fan_speed += 5;
        if(fan_speed > 255) fan_speed = 0;
      }

  }

  if (button4.isPressed()) {  //doubleclick
    screen_update = 1;


    if(pic >=1000){
      screen_last_title = pic;
    }

    button4state = 0;
    lastmillis = millis();
    if (pic < 10) {
      pic = screen_last_title;
    } else {
      pic = 1;
    }
  }

}
else {
  if (button1.isPressed() || button2.isPressed() || button3.isPressed() || button4.isPressed()) {
        lastmillis = millis();

  }
}
////////////////////////////////////////


  //Serial.print("end 1:");
  //Serial.println(millis()-start);

  //---------------------------------
  // get_temp();
  //Serial.print("end 2:");
  //Serial.println(millis()-start);
  ina226_raw();
  //Serial.print("end 3:");
  //Serial.println(millis()-start);

  batteryCalibration(duration);
  //Serial.print("end 4:");
  //Serial.println(millis()-start);

  mAh_mWh(duration);
  //Serial.print("end 5:");
  //Serial.println(millis()-start);


  if (millis() - last_timer2 >= 100) {
   if(power_mW < 0) total -= current_mA;
    count++;
    last_timer2 = millis();
  }

  unsigned long delta_timer1 = millis() - last_timer1;
  if (delta_timer1>= 15000)  // is a minute up?
  {

    last_1min_mw = total / count;
    Serial.print("total: ");
    Serial.println(total);
    Serial.print("count");
    Serial.println(count);
    Serial.print("lastmin: ");
    Serial.println(last_1min_mw);
    Serial.print("lastmin_A: ");
    float printwh = last_1min_mw / 1000;
    Serial.println(printwh);
    Serial.print("powerMW: ");
    Serial.println(power_mW );
    Serial.println("     ");

    total = 0;
    count = 0;
    last_timer1 = millis();
  }














//TEMP READING

  if (millis() - lastTempRequest >= delayInMillis)  // waited long enough??
  {
    // Serial.print(" Temperature: ");
    // temperature = sensors.getTempCByIndex(0);
    temp1 = sensors.getTempCByIndex(0) + temp_offset;
    temp1F = (temp1 * 9 / 5) + 32;
    // Serial.println(temp1, resolution - 8);
    // Serial.print("  Resolution: ");
    // Serial.println(resolution);
    // Serial.print("Idle counter: ");
    // Serial.println(idle);
    // Serial.println();
    idle = 0;
    //   immediately after fetching the temperature we request a new sample
    //  in the async modus
    //   for the demo we let the resolution change to show differences
    //   resolution++;
    //   if (resolution > 12) resolution = 9;

    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.requestTemperatures();
    // delayInMillis = 750 / (1 << (12 - resolution));
    lastTempRequest = millis();
  }
  // we can do usefull things here
  // for the demo we just count the idle time in millis
  idle++;

  // Serial.print("Celsius temperature: ");
  // // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  // Serial.print(sensors.getTempCByIndex(0));
  // Serial.print(" - Fahrenheit temperature: ");
  // Serial.println(sensors.getTempFByIndex(0));



  if (busvoltage <= mosfet_voltage_cutoff) {
    mosfet_state = 0;
  }

  if (mosfet_state == 1) {
    digitalWrite(mosfet, HIGH);
  } else {
    digitalWrite(mosfet, LOW);
  }



  if (fan_mode == 1) {
    if ((power_mW / 1000) > fan_power_on || (power_mW / 1000) < (-fan_power_on) || temp1 >= fan_temp_on) {
      fan_state = 1;
    } else {
      fan_state = 0;
    }
  }

  if((power_mW / 1000) > screen_power_on || (power_mW / 1000) < -2 ){
        lastmillis = millis();
  } 
   
  if(power_mW > 0) {
    charging =true;
  } else {
    charging = false;
  }


  if (fan_state == 1) {
    analogWrite(fan, fan_speed);
  } else {
    digitalWrite(fan, LOW);
  }


  //Serial.print("end 6:");
  //Serial.println(millis()-start);

  if (bat_int_cal == false && last_bat_int_cal == false) {
    float x = busvoltage / 4;

    if (4.1 <= x && x <= 4.3) {
      mWh = real_Wh * 1000 * 0.95;
    }
    if (4.0 <= x && x <= 4.1) {
      mWh = real_Wh * 1000 * 0.84;
    }
    if (3.9 <= x && x <= 4.0) {
      mWh = real_Wh * 1000 * 0.73;
    }
    if (3.8 <= x && x <= 3.9) {
      mWh = real_Wh * 1000 * 0.60;
    }
    if (3.7 <= x && x <= 3.8) {
      mWh = real_Wh * 1000 * 0.51;
    }
    if (3.6 <= x && x <= 3.7) {
      mWh = real_Wh * 1000 * 0.37;
    }
    if (3.5 <= x && x <= 3.6) {
      mWh = real_Wh * 1000 * 0.18;
    }
    if (3.4 <= x && x <= 3.5) {
      mWh = real_Wh * 1000 * 0.125;
    }
    if (3.3 <= x && x <= 3.4) {
      mWh = real_Wh * 1000 * 0.02;
    }
    if (3.3 <= x && x <= 3.4) {
      mWh = real_Wh * 1000 * 0.02;
    }

    if (0 <= x && x <= 3.3) {
      mWh = real_Wh * 1000 * 0;
    }
    mWh_nolimit = mWh;
    last_bat_int_cal = true;
  }

  float current = millis();
  float second = current / 1000 / 60;
  duration = millis() - last_millis;
  last_millis = millis();







  Wh = mWh / 1000;
  if (Wh > real_Wh) {
    bat_percent = 100;
  } else {
    bat_percent = Wh / real_Wh * 100;
  }
  progress = bat_percent * bat_width_max / 100;



  //Serial.print("end 7:");
  //Serial.println(millis()-start);
  //------------------------------------------------------------------------------
  //screeen update
  unsigned long current_screen_update = millis();


  if ((current_screen_update - last_screen_update) > screen_update_time || screen_update == 1) {
    screen_update = 0;

    if(charging == true){
    flashing = !flashing;} else{
      flashing = false; 
    }
     Serial.print("charing: "); 
     Serial.println(charging);

    last_screen_update = millis();
    u8g.firstPage();
    do {
      draw_display();
    } while (u8g.nextPage());
  }

  //------------------------------------------------------------------------------

  //JUMP TO DEFAULT IF NO CLICK IS DETECTED
  // if (millis() >= (lastmillis + maxtime)) {
  if (millis() >= (lastmillis + screen_timeout)) {
    
    if (display_on ==1 ){
    
    u8g.sleepOn();
    display_on = 0;
    Serial.println("screen off");
    }
  } else {

    if (display_on == 0)  {  
          u8g.sleepOff();
          display_on = 1;
          Serial.println("screen on");

    }


  }



  //Serial.print("end 8:");
  //Serial.println(millis()-start);
  // around 78 without the temp call
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void header() {
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  u8g.setPrintPos(22, 0);
  u8g.print("rg-electronics");
  // display.drawLine (0,9,128,9, WHITE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_display() {


  // byte button4State = digitalRead(button4);
  int front_width = 5;
  int front_height = 6;
  int front_height_vert_pad = front_height + 2;
  int begy = 6;
  int delta_timer1 = (millis()-(float)last_timer1)/1000;
  int nav_pos = (pic - last_pic)/10;  //cal the pos of the cursor base on the current pic and last main page
  int menu_nav_pos = nav_pos;
  if(nav_pos > 5) menu_nav_pos -= 5; //if it goes out of bound, menu only allow 5 items 

   if(0 <= pic && pic <= 10){
      u8g.setPrintPos(2, 9*pic + 11 );
      u8g.print(">");


      u8g.setPrintPos(92, begy);
      u8g.print(String(power_mW / 1000) + "W");
      u8g.setPrintPos(0, 11);
      u8g.print("SETTING");

      u8g.setPrintPos(0, 20);
      u8g.print("  FAN");
      u8g.setPrintPos(0, 29);
      u8g.print("  MOSFET");
      u8g.setPrintPos(0, 38);
      u8g.print("  BATTERY");
      u8g.setPrintPos(0, 47);
      u8g.print("  MISC");
      u8g.setPrintPos(0, 56);
      u8g.print("  PROJECT");
      u8g.setPrintPos(111, begy + front_height_vert_pad * 7);
      u8g.print(String(pic));
   }

  if(pic == 11 || pic == 21 || pic == 31|| pic == 41|| pic == 51){
      
      u8g.setPrintPos(2, 9*nav_pos + 11 );
      u8g.print(">");

      u8g.setPrintPos(92, begy);
      u8g.print(String(power_mW / 1000) + "W");

      u8g.setPrintPos(0, 11);
      u8g.print("FAN");
      u8g.setPrintPos(0, 20);
      u8g.print("  MODE:");
      u8g.setPrintPos(90, 20);
      if (fan_mode == 1) {
        u8g.print("AUTO");
      } else {
        u8g.print("MANUAL");
      }

      u8g.setPrintPos(0, 29);
      u8g.print("  ON/OFF:");
      u8g.setPrintPos(90, 29);
      u8g.print(fan_state);

      u8g.setPrintPos(0, 38);
      u8g.print("  TEMP ON:");
      u8g.setPrintPos(90, 38);
      u8g.print(fan_temp_on);

      u8g.setPrintPos(0, 47);
      u8g.print("  POWER ON:");
      u8g.setPrintPos(90, 47);
      u8g.print(fan_power_on);

            u8g.setPrintPos(0, 56);
      u8g.print("  FAN SPEED:");
      u8g.setPrintPos(90, 56);
      u8g.print(fan_speed);


      u8g.setPrintPos(111, begy + front_height_vert_pad * 7);
      u8g.print(String(pic));
  }

  if(pic == 12 || pic == 22 || pic == 32|| pic == 42|| pic == 52)
  {
      u8g.setPrintPos(2, 9*nav_pos + 11 );
      u8g.print(">");


      u8g.setPrintPos(0, 11);
      u8g.print("MOSFET");
      u8g.setPrintPos(0, 20);
      u8g.print("  ON/OFF:");
      u8g.setPrintPos(90, 20);
      u8g.print(mosfet_state);

      u8g.setPrintPos(0, 29);
      u8g.print("  VOLTAGE CUT:");
      u8g.setPrintPos(90, 29);
      u8g.print(mosfet_voltage_cutoff);

      u8g.setPrintPos(111, begy + front_height_vert_pad * 7);
      u8g.print(String(pic));

  }


  if(pic == 13|| pic == 23 || pic == 33|| pic == 43|| pic == 53 || pic == 63 || pic == 73|| pic == 83|| pic == 93){
      
      u8g.setPrintPos(2, 9*menu_nav_pos + 11 );
      u8g.print(">");

      u8g.setPrintPos(92, begy);
      u8g.print(String(power_mW / 1000) + "W");

      u8g.setPrintPos(0, 11);
      u8g.print("BATTERY");

     if(nav_pos <=5 ){
      u8g.setPrintPos(0, 20);
      u8g.print("  REM CAPACITY:");
      u8g.setPrintPos(90, 20);
      u8g.print(String(Wh));
    

      u8g.setPrintPos(0, 29);
      u8g.print("  MAX CAPACITY:");
      u8g.setPrintPos(90, 29);
      u8g.print(String(real_Wh));

      u8g.setPrintPos(0, 38);
      u8g.print("  CAL:");
      u8g.setPrintPos(90, 38);
      u8g.print(bat_cal);

      u8g.setPrintPos(0, 47);
      u8g.print("  MIN BAT CAL:");
      u8g.setPrintPos(90, 47);
      u8g.print(min_voltage);

            u8g.setPrintPos(0, 56);
      u8g.print("  MAX BAT CAL:");
      u8g.setPrintPos(90, 56);
      u8g.print(max_voltage);
     } else {
      u8g.setPrintPos(0, 20);
      u8g.print("  CHARGE(WH):");
      u8g.setPrintPos(90, 20);
      u8g.print(String(mWh_nolimit/1000));
    

      u8g.setPrintPos(0, 29);
      u8g.print("  ALL DISCHA.(KWH)");
      u8g.setPrintPos(90, 29);
      u8g.print(total_discharge);

      u8g.setPrintPos(0, 38);
      u8g.print("  BATTERY CYCLE:");
      u8g.setPrintPos(90, 38);
      u8g.print(bat_cal);
     }


      u8g.setPrintPos(111, begy + front_height_vert_pad * 7);
      u8g.print(String(pic));
  }
 



//selection

 int nav_select_menu[5][4] = {{90, 20-5,30 , 5},
 {90, 29-5,30 , 5},
 {90, 38-5,30 , 5},
 {90, 47-5,30 , 5},
 {90, 56-5,30 , 5}};


      // u8g.setColorIndex(0);
      // u8g.drawBox(nav_select_menu[0][0], nav_select_menu[0][1] , nav_select_menu[0][2], nav_select_menu[0][3]);


      // u8g.setColorIndex(1);

  if(selection == 1){
  //u8g.drawBox(bat_x + bat_hor_padding, bat_y + bat_ver_padding, progress, bat_height - bat_ver_padding * 2);

  }


    if(pic == 1000){

      u8g.setPrintPos(128 - 28, front_height + 8);
      if (flashing == false) {
        u8g.print(String(bat_percent) + "%");
          } else {
            //empty
          }
      u8g.drawFrame(bat_x, bat_y, bat_width, bat_height);
      u8g.drawBox(bat_x + bat_hor_padding, bat_y + bat_ver_padding, progress, bat_height - bat_ver_padding * 2);


     
   
      u8g.setPrintPos(0, begy);
      u8g.print(String(Wh) + "/" + String(real_Wh) + "Wh");
      u8g.setPrintPos(0, begy + front_height_vert_pad);
      u8g.print(String(busvoltage) + "V " + String(current_mA) + "mA");
      u8g.setPrintPos(0, begy + front_height_vert_pad * 2);
      u8g.print(String(power_mW / 1000) + "W");
      u8g.setPrintPos(0, begy + front_height_vert_pad * 3);
      u8g.print(String(temp1) + " oC " + String(temp1F) + " oF  ");
      u8g.setPrintPos(0, begy + front_height_vert_pad * 4);
      u8g.print("CAL: " + String(bat_cal) + " DIS: " + String(cal_discharge_Wh) + " (NL): " + String(mWh_nolimit / 1000));
      u8g.setPrintPos(0, begy + front_height_vert_pad * 5);
      // u8g.print("Wh(nolimit): " + String(mWh_nolimit / 1000));
      u8g.print("MIN(mW): " + String(last_1min_mw) + " S COUNT: "+ String(delta_timer1) );

      u8g.setPrintPos(0, begy + front_height_vert_pad * 6);
     
      u8g.print("FAN : ");
          if (fan_mode == 1) {
        u8g.print("A ");
      } else {
        u8g.print("M ");
      }
      u8g.print(String(fan_state) + " MOSFET : " + String(mosfet_state));
    
}






if(pic == 1001){
      // u8g.setPrintPos(128 - 28, front_height + 8);
      // u8g.print(String(bat_percent) + "%");
      // u8g.drawFrame(bat_x, bat_y, bat_width, bat_height);
      // u8g.drawBox(bat_x + bat_hor_padding, bat_y + bat_ver_padding, progress, bat_height - bat_ver_padding * 2);

      u8g.setFont(u8g_font_freedoomr25n); 
      // u8g.setFont(u8g_font_10x20r);  //w7h12

    //   u8g.setColorIndex(1);
    //  u8g.drawBox(10, 12, 20, 30);  
    //  u8g.setColorIndex(0);


      u8g.setScale2x2();
      // u8g.setPrintPos(25, 22);
      u8g.setPrintPos(10, 30);
      u8g.print(String(bat_percent));
      u8g.setFont(u8g_font_04b_03b);  //w5h6
      if (flashing == false) {
            u8g.print("%");
          } else {
            //empty
          }
        
      u8g.undoScale();
}


  u8g.setFont(u8g_font_04b_03b);  //w5h6
  switch (pic) {
  

    case 14:
      u8g.setPrintPos(2, 9*nav_pos + 11 );
      u8g.print(">");

      u8g.setPrintPos(92, begy);
      u8g.print(String(power_mW / 1000) + "W");

      u8g.setPrintPos(0, 11);
      u8g.print("MISC");
      u8g.setPrintPos(0, 20);
      u8g.print("  EEPROM SAVE:");
      u8g.setPrintPos(90, 20);
      if (fan_mode == 1) {
        u8g.print("AUTO");
      } else {
        u8g.print("MANUAL");
      }

      u8g.setPrintPos(0, 29);
      u8g.print("  TEMP OFFSET:");
      u8g.setPrintPos(90, 29);
      u8g.print(fan_state);
      // u8g.setPrintPos(0, 38);
      // u8g.print("  TEMP ON:");
      // u8g.setPrintPos(90, 38);
      // u8g.print(fan_temp_on);

      // u8g.setPrintPos(0, 47);
      // u8g.print("  POWER ON:");
      // u8g.setPrintPos(90, 47);
      // u8g.print(fan_power_on);

      //       u8g.setPrintPos(0, 56);
      // u8g.print("  FAN SPEED:");
      // u8g.setPrintPos(90, 56);
      // u8g.print(fan_speed);
      u8g.setPrintPos(111, begy + front_height_vert_pad * 7);
      u8g.print(String(pic));


      break;


    case 15:
    

      u8g.setPrintPos(0, 11);
      u8g.print("PROJECT INFO");
      u8g.setPrintPos(0, 20);
      u8g.print("NAME:");
      u8g.setPrintPos(50, 20);
      u8g.print("POWER BANK");
      u8g.setPrintPos(0, 29);
      u8g.print("DATE:");
      u8g.setPrintPos(50, 29);
      u8g.print("");
      u8g.setPrintPos(0, 38);
      u8g.print("BY:");
      u8g.setPrintPos(50, 38);
      u8g.print("LUONG HOANG");
      u8g.setPrintPos(111, begy + front_height_vert_pad * 7);

      u8g.print(String(pic));


      break;



    case 1002:

      u8g.setPrintPos(128 - 28, front_height + 8);
      u8g.print(String(bat_percent) + "%");
      u8g.drawFrame(bat_x, bat_y, bat_width, bat_height);
      u8g.drawBox(bat_x + bat_hor_padding, bat_y + bat_ver_padding, progress, bat_height - bat_ver_padding * 2);


      u8g.setScale2x2();
      u8g.setPrintPos(0, 8);
      u8g.print(String(busvoltage) + "V ");
      u8g.setPrintPos(0, 18);
      u8g.print(String(power_mW / 1000) + "W");
      u8g.setPrintPos(0, 28);
      u8g.print(String(current_mA) + "mA");

      u8g.undoScale();
      // u8g.setFont(u8g_font_04b_03b);  //w5h6

      // u8g.setPrintPos(111, begy + front_height_vert_pad * 7);
      // u8g.print(String(pic) + "/" + String(max_page));

      break;

   

  }
}



//------------------------------------------------------------------------
