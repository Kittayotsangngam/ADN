/*
  x 300 435
  y 290 430
  z 285 444
  pin 3,4,5,6 Mode/Enter,Select,Activate,Back
  Mode selection state : 0=Settime, 1=Save, 2=Alarm, 3=stopwatch
  ALL MODE: DISPLAY,SELECTMODE,ALARM,STOPWATCH
*/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <TimerOne.h>
#include <EEPROM.h>
static const unsigned char PROGMEM alarmclock[] = {
  0x00, 0x00, 0x38, 0x1C, 0x60, 0x06, 0x43, 0xC2, 0x0C, 0x30, 0x10, 0x08, 0x00, 0x00, 0x20, 0x04,
  0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x00, 0x04, 0x10, 0x08, 0x08, 0x10, 0x0F, 0xF0, 0x00, 0x00
};
int inputMode, inputRight, inputDown , inputBack;
String Mode = "DISPLAY";
int Select = 0;
unsigned long allsecond, hour, minute, second;
unsigned long allsecondTEMP;
unsigned long alarmsec;
int day, month, year, today;
String Day[7] = {"Sun ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sat "};
String Month[13] = {" ", "Jan ", "Feb ", "Mar ", "Apr ", "May ", "Jun ", "Jul ", "Aug ", "Sep ", "Oct ", "Nov ", "Dec "};
int dayTEMP , monthTEMP, yearTEMP;
int x, y, z;
bool flip = false , alarm = false;
unsigned long long blinkTimer, buttonBounce[4], stopwatchTimer, stopwatchEnd, alarmdelay;
unsigned long sum, summinutes, sumsecond, summillisecond , minussum;
bool stopwatch, firststop;
bool monthdate[13] = {0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1}; //false=30 true=31 except feb
void setup() {
  Serial.begin(9600);
  DDRD &= B10000111;
  PORTD |= B01111000;
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.display();

  EEPROM.get(0, allsecond);
  EEPROM.get(sizeof(unsigned long), today);
  EEPROM.get(sizeof(unsigned long) + sizeof(int), day);
  EEPROM.get(sizeof(unsigned long) + 2 * sizeof(int), month);
  EEPROM.get(sizeof(unsigned long) + 3 * sizeof(int), year);
  EEPROM.get(sizeof(unsigned long) + 4 * sizeof(int), alarmsec);
  if (allsecond <= 0) allsecond = 0;
  if (today < 0) today = 5;
  if (day <= 0) day = 1;
  if (month <= 0) month = 1;
  if (year <= 0) year = 2021; //default value;
  if (alarmsec <= 0) alarmsec = 90000;

  Timer1.initialize(1000000);
  Timer1.attachInterrupt(countSec);
}
void countSec() {
  allsecond++;
}
void BLINK(int x, int y, int width, int height) { // put this function under object that u want to blink
  if (millis() - blinkTimer >= 600) blinkTimer = millis();
  else if (millis() - blinkTimer >= 300) oled.fillRect(x, y, width, height, BLACK);
}
bool checkLeapYear(unsigned long i) {
  if (i % 4 == 0 && i % 100 != 0 || i % 400 == 0) return true;
  return false;
}
void UpdateDate() {
  if (month == 2) {
    if (checkLeapYear && day > 29) day = 1, month++;
    else if (!checkLeapYear && day > 28) day = 1, month++;
  }
  else if (day > 31 && monthdate[month]) {
    day = 1; month++;
    if (month == 13) month = 1, year++;
  }
  else if (day > 30 && !monthdate[month]) {
    day = 1; month++;
  }
}
void printDate() {
  oled.print(Day[today]);
  oled.print(String(day) + " ");
  oled.print(Month[month]);
  if (year / 1000 == 0) {
    oled.print("0");
    if ((year % 1000) / 100 == 0) {
      oled.print("0");
      if ((year % 100) / 10 == 0) oled.print("0");
    }
  }
  oled.print(String(year));
}
int dayofweek(int d, int m, int y) {
  static int t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  y -= m < 3;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}
void alarmtone() {
  if (millis() - alarmdelay > 500) tone(8, 330, 1000 / 8);
}
void loop() {
  //**this is input pullup**
  inputMode = PIND & (1 << 3);
  inputRight = PIND & (1 << 4);
  inputDown = PIND & (1 << 5);
  inputBack = PIND & (1 << 6);
  x = analogRead(A0);
  if (x < 330) flip = true;
  else if (x > 415) flip = false;
  if (millis() - alarmdelay > 1000) {
    noTone(8);
    alarmdelay = millis();
  }
  if (alarmsec == allsecond) alarm = true;
  if (alarm) {
    alarmtone();
    if (!inputBack && millis() - buttonBounce[3] >= 300) {
      alarm = false;
      alarmsec = 90000;
      buttonBounce[3] = millis();
    }
  }
  //============Time Update===========
  if (allsecond >= 86400) {
    allsecond -= 86400;
    day++;
    today++;
    today %= 7;
    UpdateDate();
  }
  hour = allsecond / 3600;
  minute = (allsecond / 60) % 60;
  second = allsecond % 60;
  //=================================================================================
  if (Mode == "DISPLAY") {
    //===========Button Update==========
    if (!inputMode && millis() - buttonBounce[0] >= 300) {
      Mode = "SELECTMODE";
      Select = 0;
      buttonBounce[0] = millis();
    }
    //==================================
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.setTextSize(3);
    if (hour < 10) oled.print("0");
    oled.print(String(hour) + " ");
    oled.fillRect(42, 2, 4, 4, WHITE);
    oled.fillRect(42, 16, 4, 4, WHITE);
    BLINK(42, 2, 10, 22);
    if (minute < 10) oled.print("0");
    oled.println(String(minute));
    oled.setTextSize(1);
    printDate();
    if (alarmsec < 86500) oled.drawBitmap(96, 0, alarmclock, 16, 16, WHITE);
  }
  //=================================================================================
  else if (Mode == "SELECTMODE") {
    oled.setCursor(0, 0);
    oled.setTextSize(1);
    oled.println("Set Time");
    oled.println("Save Time");
    oled.println("Alarm");
    oled.setCursor(64, 0);
    oled.println("Stopwatch");
    if (!inputMode && millis() - buttonBounce[0] >= 300) {
      buttonBounce[0] = millis();
      if (Select == 0) {
        Select = 0;
        Mode = "SETTIME";
        allsecondTEMP = allsecond;
        dayTEMP = day, monthTEMP = month, yearTEMP = year;
      }
      else if (Select == 1) {
        Mode = "DISPLAY";
        EEPROM.put(0, allsecond);
        EEPROM.put(sizeof(unsigned long), today);
        EEPROM.put(sizeof(unsigned long) + sizeof(int), day);
        EEPROM.put(sizeof(unsigned long) + 2 * sizeof(int), month);
        EEPROM.put(sizeof(unsigned long) + 3 * sizeof(int), year);
        EEPROM.put(sizeof(unsigned long) + 4 * sizeof(int), alarmsec);
      }
      else if (Select == 2) {
        Select = 0;
        if (alarmsec < 86400) allsecondTEMP = alarmsec;
        else allsecondTEMP = allsecond;
        Mode = "ALARM";
      }
      else if (Select == 3) {
        stopwatch = false;
        firststop = true;
        stopwatchTimer = stopwatchEnd = 0;
        sum = summinutes = sumsecond = summillisecond = minussum = 0;
        Mode = "STOPWATCH";
      }
    }
    if (!inputRight && millis() - buttonBounce[1] >= 300) {
      Select = (Select + 1) % 4;
      buttonBounce[1] = millis();
    }
    if (!inputDown && millis() - buttonBounce[2] >= 300) {
      Select = (Select + 3) % 4;
      buttonBounce[2] = millis();
    }
    if (!inputBack && millis() - buttonBounce[3] >= 300) {
      Mode = "DISPLAY";
      Select = 0;
      buttonBounce[3] = millis();
    }
    if (Select == 0) BLINK(0, 0, 64, 8);
    else if (Select == 1) BLINK(0, 8, 64, 8);
    else if (Select == 2) BLINK(0, 16, 64, 8);
    else if (Select == 3) BLINK(64, 0, 64, 8);
  }
  //=================================================================================
  else if (Mode == "SETTIME") {
    if (!inputDown && millis() - buttonBounce[2] >= 300) {
      if (Select == 0) {
        if ((allsecondTEMP % 36000) / 3600 > 3) {
          if (allsecondTEMP / 36000 == 1) allsecondTEMP -= 36000;
          else allsecondTEMP += 36000;
        }
        else {
          if (allsecondTEMP / 36000 == 2) allsecondTEMP -= 36000 * 2;
          else allsecondTEMP += 36000;
        }
      }
      else if (Select == 1) {
        if (allsecondTEMP / 36000 == 2) {
          if ((allsecondTEMP % 36000) / 3600 == 3) allsecondTEMP -= 3 * 3600;
          else allsecondTEMP += 3600;
        }
        else {
          if ((allsecondTEMP % 36000) / 3600 == 9) allsecondTEMP -= 9 * 3600;
          else allsecondTEMP += 3600;
        }
      }
      else if (Select == 2) {
        if ((allsecondTEMP % 3600) / 600 == 5) allsecondTEMP -= 50 * 60;
        else allsecondTEMP += 10 * 60;
      }
      else if (Select == 3) {
        if ((allsecondTEMP / 60) % 10 == 9) allsecondTEMP -= 9 * 60;
        else allsecondTEMP += 60;
      }
      else if (Select == 4) {
        if (monthTEMP == 2) {
          if (dayTEMP / 10 == 2) dayTEMP -= 20;
          if (!checkLeapYear(yearTEMP) && dayTEMP % 10 == 8 && dayTEMP / 10 == 1) dayTEMP -= 10;
          else dayTEMP += 10;
        }
        else {
          if (dayTEMP / 10 == 3) dayTEMP -= 30;
          else if (!monthdate[monthTEMP] && dayTEMP % 10 != 0 && dayTEMP / 10 == 2) dayTEMP -= 20;
          else if (monthdate[monthTEMP] && !(dayTEMP % 10 == 0 || dayTEMP % 10 == 1) && dayTEMP / 10 == 2) dayTEMP -= 20;
          else dayTEMP += 10;
        }
        if (dayTEMP == 0) dayTEMP += 10;
      }
      else if (Select == 5) {
        if (monthTEMP == 2) {
          if (!checkLeapYear(yearTEMP) && dayTEMP % 10 == 8) dayTEMP -= 8;
          else if (dayTEMP % 10 == 9) dayTEMP -= 9;
          else dayTEMP++;
        }
        else if (!monthdate[monthTEMP]) {
          if (dayTEMP / 10 < 3 && dayTEMP % 10 == 9) dayTEMP -= 9;
          else if (dayTEMP / 10 < 3) dayTEMP++;
        }
        else if (monthdate[monthTEMP]) {
          if (dayTEMP / 10 < 3 && dayTEMP % 10 == 9) dayTEMP -= 9;
          else if (dayTEMP / 10 < 3) dayTEMP++;
          else if (dayTEMP / 10 == 3 && dayTEMP % 10 == 1) dayTEMP--;
          else dayTEMP++;
        }
        if (dayTEMP == 0) dayTEMP++;
      }
      else if (Select == 6) {
        monthTEMP = (monthTEMP % 12) + 1;
        if (monthTEMP == 0) monthTEMP++;
        if (monthTEMP == 2 && checkLeapYear(yearTEMP) && dayTEMP > 29) monthTEMP = 3;
        if (monthTEMP == 2 && !checkLeapYear(yearTEMP) && dayTEMP > 28) monthTEMP = 3;
        if (dayTEMP > 30 && !monthdate[monthTEMP]) while (!monthdate[monthTEMP]) monthTEMP = (monthTEMP % 12) + 1;
      }
      if (!(checkLeapYear(yearTEMP) && monthTEMP == 2 && dayTEMP == 29)) {
        if (Select == 7) {
          if (yearTEMP / 1000 == 9) yearTEMP -= 9000;
          else yearTEMP += 1000;
          if (yearTEMP == 0) yearTEMP += 1000;
        }
        else if (Select == 8) {
          if ((yearTEMP % 1000) / 100 == 9) yearTEMP -= 900;
          else yearTEMP += 100;
          if (yearTEMP == 0) yearTEMP += 100;
        }
        else if (Select == 9) {
          if ((yearTEMP % 100) / 10 == 9) yearTEMP -= 90;
          else yearTEMP += 10;
          if (yearTEMP == 0) yearTEMP += 10;
        }
        else if (Select == 10) {
          if (yearTEMP % 10 == 9) yearTEMP -= 9;
          else yearTEMP++;
          if (yearTEMP == 0) yearTEMP ++;
        }
      }
      buttonBounce[2] = millis();
    }
    if (!inputRight && millis() - buttonBounce[1] >= 300) {
      Select = (Select + 1) % 11;
      buttonBounce[1] = millis();
    }
    if (!inputMode && millis() - buttonBounce[0] >= 300) {
      Select = 0;
      Mode = "DISPLAY";
      allsecondTEMP -= allsecondTEMP % 60;
      allsecond = allsecondTEMP;
      day = dayTEMP, month = monthTEMP, year = yearTEMP;
      today = dayofweek(day, month, year);
      buttonBounce[0] = millis();
    }
    if (!inputBack && millis() - buttonBounce[3] >= 300) {
      Select = 0;
      Mode = "SELECTMODE";
      buttonBounce[3] = millis();
    }
    //===============================================================================
    unsigned long hourTEMP = allsecondTEMP / 3600;
    unsigned long minuteTEMP = (allsecondTEMP / 60) % 60;
    unsigned long secondTEMP = allsecondTEMP % 60;
    oled.setCursor(0, 0);
    oled.setTextSize(2);
    if (hourTEMP < 10) oled.print("0");
    oled.print(String(hourTEMP));
    oled.print(":");
    if (minuteTEMP < 10) oled.print("0");
    oled.println(String(minuteTEMP));
    oled.setTextSize(1);
    if (dayTEMP < 10) oled.print("0");
    oled.print(String(dayTEMP) + "/");
    if (monthTEMP < 10) oled.print("0");
    oled.print(String(monthTEMP) + "/");
    if (yearTEMP / 1000 == 0) {
      oled.print("0");
      if ((yearTEMP % 1000) / 100 == 0) {
        oled.print("0");
        if ((yearTEMP % 100) / 10 == 0) oled.print("0");
      }
    }
    oled.print(String(yearTEMP));
    if (Select == 0) BLINK(0, 0, 12, 16);
    else if (Select == 1) BLINK(12, 0, 12, 16);
    else if (Select == 2) BLINK(36, 0, 12, 16);
    else if (Select == 3) BLINK(48, 0, 12, 16);
    else if (Select == 4) BLINK(0 , 16, 6, 10);
    else if (Select == 5) BLINK(6 , 16, 6, 10);
    else if (Select == 6) BLINK(18 , 16, 12, 10);
    else if (Select == 7) BLINK(36 , 16, 6, 10);
    else if (Select == 8) BLINK(42 , 16, 6, 10);
    else if (Select == 9) BLINK(48 , 16, 6, 10);
    else if (Select == 10) BLINK(54 , 16, 6, 10);
  }
  //=================================================================================
  else if (Mode == "ALARM") {
    if (Select == 0) {
      if (!inputRight && millis() - buttonBounce[1] >= 300) {
        Select = 1;
        buttonBounce[1] = millis();
      }
      if (!inputDown && millis() - buttonBounce[2] >= 300) {
        if ((allsecondTEMP % 36000) / 3600 > 3) {
          if (allsecondTEMP / 36000 == 1) allsecondTEMP -= 36000;
          else allsecondTEMP += 36000;
        }
        else {
          if (allsecondTEMP / 36000 == 2) allsecondTEMP -= 36000 * 2;
          else allsecondTEMP += 36000;
        }
        buttonBounce[2] = millis();
      }
    }
    else if (Select == 1) {
      if (!inputRight && millis() - buttonBounce[1] >= 300) {
        Select = 2;
        buttonBounce[1] = millis();
      }
      if (!inputDown && millis() - buttonBounce[2] >= 300) {
        if (allsecondTEMP / 36000 == 2) {
          if ((allsecondTEMP % 36000) / 3600 == 3) allsecondTEMP -= 3 * 3600;
          else allsecondTEMP += 3600;
        }
        else {
          if ((allsecondTEMP % 36000) / 3600 == 9) allsecondTEMP -= 9 * 3600;
          else allsecondTEMP += 3600;
        }
        buttonBounce[2] = millis();
      }
    }
    else if (Select == 2) {
      if (!inputRight && millis() - buttonBounce[1] >= 300) {
        Select = 3;
        buttonBounce[1] = millis();
      }
      if (!inputDown && millis() - buttonBounce[2] >= 300) {
        if ((allsecondTEMP % 3600) / 600 == 5) allsecondTEMP -= 50 * 60;
        else allsecondTEMP += 10 * 60;
        buttonBounce[2] = millis();
      }
    }
    else if (Select == 3) {
      if (!inputRight && millis() - buttonBounce[1] >= 300) {
        Select = 4;
        buttonBounce[1] = millis();
      }
      if (!inputDown && millis() - buttonBounce[2] >= 300) {
        if ((allsecondTEMP / 60) % 10 == 9) allsecondTEMP -= 9 * 60;
        else allsecondTEMP += 60;
        buttonBounce[2] = millis();
      }
    }
    else if (Select == 4) {
      if (!inputRight && millis() - buttonBounce[1] >= 300) {
        Select = 0;
        buttonBounce[1] = millis();
      }
      if (!inputDown && millis() - buttonBounce[2] >= 300) {
        alarmsec = 90000;
        Mode = "DISPLAY";
        buttonBounce[2] = millis();
      }
    }
    if (!inputMode && millis() - buttonBounce[0] >= 300) {
      alarmsec = allsecondTEMP - allsecondTEMP % 60;
      Select = 0;
      Mode = "DISPLAY";
      buttonBounce[0] = millis();
    }
    if (!inputBack && millis() - buttonBounce[3] >= 300) {
      Select = 0;
      Mode = "SELECTMODE";
      buttonBounce[3] = millis();
    }
    unsigned long hourTEMP = allsecondTEMP / 3600;
    unsigned long minuteTEMP = (allsecondTEMP / 60) % 60;
    unsigned long secondTEMP = allsecondTEMP % 60;
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.setTextSize(2);
    if (hourTEMP < 10) oled.print("0");
    oled.print(String(hourTEMP));
    oled.print(":");
    if (minuteTEMP < 10) oled.print("0");
    oled.println(String(minuteTEMP));
    oled.setTextSize(1);
    oled.print("Cancal Alarm");
    if (Select == 0) BLINK(0, 0, 12, 16);
    else if (Select == 1) BLINK(12, 0, 12, 16);
    else if (Select == 2) BLINK(36, 0, 12, 16);
    else if (Select == 3) BLINK(48, 0, 12, 16);
    else if (Select == 4) BLINK(0 , 16, 80, 16);
  }
  //=================================================================================
  else if (Mode == "STOPWATCH") {
    if (!inputMode && millis() - buttonBounce[0] >= 300) {
      if (!stopwatch) {
        if (firststop) {
          stopwatchTimer = millis();
          firststop = false;
        }
        else minussum += millis() - stopwatchEnd;
        stopwatch = true;
      }
      else {

        stopwatch = false;
        stopwatchEnd = millis();
      }
      buttonBounce[0] = millis();
    }
    if (!inputDown && millis() - buttonBounce[2] >= 300) {
      if (!stopwatch) {
        stopwatchTimer = 0;
        stopwatchEnd = 0;
        sum = summinutes = sumsecond = summillisecond = minussum = 0;
      }
      buttonBounce[2] = millis();
    }
    if (!inputBack && millis() - buttonBounce[3] >= 300) {
      Select = 0;
      Mode = "SELECTMODE";
      buttonBounce[3] = millis();
    }
    oled.setCursor(0, 0);
    oled.setTextSize(3);
    if (stopwatch) {
      sum = millis() - stopwatchTimer - minussum;
      summinutes = sum / 60000, sumsecond = (sum % 60000) / 1000, summillisecond = sum % 1000;
    }
    else {
      if (stopwatchEnd > stopwatchTimer)
        sum = stopwatchEnd - stopwatchTimer - minussum;
      else sum = 0;
      summinutes = sum / 60000, sumsecond = (sum % 60000) / 1000, summillisecond = sum % 1000;
    }
    if (summinutes < 10) oled.print("0");
    oled.print(String(summinutes) + " ");
    oled.fillRect(42, 2, 4, 4, WHITE);
    oled.fillRect(42, 16, 4, 4, WHITE);
    if (sumsecond < 10) oled.print("0");
    oled.print(String(sumsecond));
    oled.setCursor(88, 14);
    oled.setTextSize(1);
    oled.print(".");
    if ((summillisecond % 1000) / 100 == 0) {
      oled.print("0");
      if ((summillisecond % 100) / 10 == 0) oled.print("0");
    }
    oled.print(String(summillisecond));
    oled.setCursor(0, 24);
    oled.print("Stopwatch Mode");
  }
  //=================================================================================
  if (flip) oled.setRotation(2);
  else if (!flip) oled.setRotation(0);
  oled.display();
  oled.clearDisplay();
}
