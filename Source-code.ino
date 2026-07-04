#define BLYNK_TEMPLATE_ID "TMPL62DBXr7-M"
#define BLYNK_TEMPLATE_NAME "Smart Door Lock"
#define BLYNK_AUTH_TOKEN "YcxMVKIJU5Zyddd-UI0TrkcsrTGfXdiF"
#include <Keypad.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>




////////////////////////////////////////
const char* ssid="GJU_STAFF";
const char* password="GJUstaff";
///////////////////////////////////////



////////////////////////////////////
const int RELAY_PIN=15;
const int BUZZER_PIN=4;
byte rowPins[4]={13, 12, 14, 27};
byte colPins[4]={26, 25, 33, 32};
///////////////////////////////////





///////////////////////////////////////////////////////////////////////////
const int rows=4;
const int colunms=4;
char keys[4][4]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,rows,colunms);
/////////////////////////////////////////////////////////////////////////








/////////////////////////////////////////////////////////////////////////
BlynkTimer timer;
WidgetTerminal terminal(V2);

String correctPass;
String entered;
String newPass;
String accessLog;
int  wrongCount=0;
bool lockedOut=false;
unsigned long lockTime=0;
int state=0;
///////////////////////////////////////////////////////////////////////////












String getTime() {
  struct tm t;
  if (!getLocalTime(&t)) return "--:--:--";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
  return String(buf);
}





void log(String msg) {
  accessLog = "[" + getTime() + "] " + msg + "<br>" + accessLog;
  if (accessLog.length() > 2000) 
  accessLog = accessLog.substring(0, 2000);
  Serial.println(msg);
  terminal.println("[" + getTime() + "] " + msg);
  terminal.flush();
}




///////////////////////////////////////////////////////////////////////////////////////
void beep(int ms){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}
/////////////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////////////
void unlock(){
  digitalWrite(RELAY_PIN,LOW);
  Blynk.virtualWrite(V1,"UNLOCKED");
  beep(100);
  delay(80);
  beep(100);
  log("Access Granted - Door Unlocked");
  delay(5000);
  digitalWrite(RELAY_PIN, HIGH);
  Blynk.virtualWrite(V1, "LOCKED");
  log("Door Locked");
}
///////////////////////////////////////////////////////////////////////////////////////







//////////////////////////////////////////////////////////////////////////////////////
void updateWrongCount(){
Blynk.virtualWrite(V3, String(wrongCount) + " / 3");
}
////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////
void updateLockoutLabel(bool locked){
  if (locked){
  Blynk.virtualWrite(V4, "LOCKED OUT");
  }else{
  Blynk.virtualWrite(V4, "NORMAL");
}
}
/////////////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////////////
void savePass(String p){
EEPROM.write(0, p.length());
for (int i=0;i<(int)p.length();i++) 
EEPROM.write(i+1,p[i]);
EEPROM.commit();
}
//////////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////////
String loadPass(){
  int len=EEPROM.read(0);
  String p;
  if (len==0 || len>10 || len==255){ 
  return "1234";}

  for (int i=0; i<len;i++) 
  p=p+(char)EEPROM.read(i+1);
  return p;
}
//////////////////////////////////////////////////////////////////////////////////









///////////////////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V5){
 String blynkEntered=param.asStr();


 if(lockedOut){
 log("Locked Out! Cannot enter password.");
 return;
}


  if(blynkEntered==correctPass){
  wrongCount = 0;
  updateWrongCount();
  log("Correct password from Blynk");
  unlock();
  }else{
  wrongCount++;
  updateWrongCount();
  log("Wrong Password (" + String(wrongCount) + "/3)");
  if (wrongCount >= 3) {
  lockedOut = true;
  lockTime  = millis();
  updateLockoutLabel(true);
  log("LOCKOUT - Wait 60 seconds");
}}}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////











//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleKey(char key){
  beep(30);


  if (lockedOut){
  beep(500);
  return; 
  }


switch(state){



////////////////////////////////////////////////////////////////////////////////////////////////    
  case 0:




    switch (key) {


      //**********************************************************************************
      case '#':
        if (entered == correctPass) {
        wrongCount = 0;
        updateWrongCount();
        unlock();
        }else{
        wrongCount++;
        updateWrongCount();
        log("Wrong Password (" + String(wrongCount) + "/3)");
        beep(600);
        if (wrongCount >= 3) {
        lockedOut = true;
        lockTime  = millis();
        updateLockoutLabel(true);
        log("LOCKOUT - Wait 60 seconds");
        }
        }
        entered = "";
       break;
      //**********************************************************************************






      //**********************************************************************************
        case '*':
          entered = "";
          break;

        case 'A':
          state = 1; 
          entered = ""; 
          log("Change password mode - Enter old password");
          break;
      //********************************************************************************





      //********************************************************************************
        default:
          if (entered.length() < 10) {
            entered += key;
          }
          break;
      //**********************************************************************************



      }
      break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  case 1: 
    switch (key) {

      //***************************************************************************
        case '#':
          if (entered == correctPass) { 
            state = 2; 
            entered = ""; 
            log("Enter new password"); 
          } else { 
            beep(600); 
            state = 0; 
            entered = ""; 
            log("Wrong old password"); 
          }
          break;
      //*****************************************************************************




      //*****************************************************************************
        case '*':
          state = 0; 
          entered = "";
          break;
      //*****************************************************************************





      //*****************************************************************************
        default: 
          if (entered.length() < 10) {
            entered += key;
          }
          break;
      //******************************************************************************
      }
      break;
////////////////////////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////////////////////////
  case 2: 
    switch (key) {
    //*******************************************************************************************
        case '#':
          if (entered.length() < 4) {
            beep(600); 
            entered = ""; 
          } else { 
            newPass = entered; 
            entered = ""; 
            state = 3; 
            log("Confirm new password"); 
          }
          break;
    //*******************************************************************************************





    //********************************************************************************************
        case '*':
          state = 0; 
          entered = "";
          break;
    //********************************************************************************************






    //*******************************************************************************************
        default: 
          if (entered.length() < 10) {
            entered += key;
          }
          break;
    //*******************************************************************************************



      }
      break; 
////////////////////////////////////////////////////////////////////////////////////////////////////////








//////////////////////////////////////////////////////////////////////////////////////////////////////
    case 3: 
      switch (key) {
      //*******************************************************************************************
        case '#':
          if (entered == newPass) {
            correctPass = newPass;
            savePass(correctPass); 
            log("Password Changed");
            beep(100);
            delay(80);
            beep(100);
          } else {
            beep(600);
            log("Password Mismatch - Cancelled");
          }
          state = 0;
          entered = ""; 
          newPass = "";
          break;
      //********************************************************************************************





      //******************************************************************************************
        case '*':
          state = 0; 
          entered = ""; 
          newPass = ""; 
          break;
        //****************************************************************************************





        //***************************************************************************************
        default:
          if (entered.length() < 10) {
            entered += key;
          }
          break;
      //****************************************************************************************


      }
      break; 
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////////////////////////////
void timerCheck() {
  if (lockedOut && millis() - lockTime >= 60000) {
    lockedOut  = false;
    wrongCount = 0;
    updateWrongCount();
    updateLockoutLabel(false);
    log("Lockout ended");
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////









/////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  EEPROM.begin(64);
  correctPass = loadPass();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  beep(100); 
  delay(80); 
  beep(100);
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.print("\nIP: ");
    Serial.println(ip);
    configTime(10800, 0, "pool.ntp.org");
    log("System started - IP: " + ip);
  } else {
    log("Started offline");
  }

  Blynk.virtualWrite(V1, "LOCKED");
  Blynk.virtualWrite(V3, "0 / 3");
  updateLockoutLabel(false);
  timer.setInterval(1000L, timerCheck);
}
//////////////////////////////////////////////////////////////////////////////////////////








void loop() {
  Blynk.run();
  timer.run();

char key = keypad.getKey();
  if (key) 
  handleKey(key);
  delay(10);
}