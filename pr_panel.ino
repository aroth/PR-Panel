#include <Servo.h>
#include <LiquidCrystal.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <SPI.h>
#include <String.h>

//
// Networking
//

byte mac[]     = { 0x90, 0xA2, 0xDA, 0x00, 0x73, 0xC1 };  
byte ip[]      = { 192, 168, 3, 13 };    
byte gateway[] = { 192, 168, 3, 1 };
byte subnet[]  = { 255, 255, 255, 240 };

// Strings & Constants
String lcd_default_str = "PR Panel 1.0";
String song;

//
// Pins
//
int lcdBacklightPin = 2;

int ledPin          = A0;
int chalkButtonPin  = A1;
int bellServoPin    = A2; 
int chalkServoPin   = A3; 
int prButtonPin     = A4;
int bellButtonPin   = A5; 

//
// Servos
//
Servo chalkServo;
Servo bellServo;

//
// LCD 
//

int RS = 7;
int EN = 8;
int D4 = 6;
int D5 = 5;
int D6 = 3;
int D7 = 9;

// Init Objects
EthernetServer server = EthernetServer(777);
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7 );

void setup(){
  Serial.begin(9600);

  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();

  lcd_default();
  
  pinMode(lcdBacklightPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(chalkButtonPin, INPUT);
  pinMode(bellButtonPin, INPUT);
  pinMode(bellServoPin, OUTPUT);
  pinMode(chalkServoPin, OUTPUT);
  pinMode(prButtonPin, INPUT);
}



void loop(){

  led_off();
  lcd_off();

//  Serial.print("[D] BELL BUTTON: ");
//  Serial.println( digitalRead( bellButtonPin ) );
//
//  Serial.print("[D] CHALK BUTTON: ");
//  Serial.println( digitalRead( chalkButtonPin ) );
//
//  Serial.print("[D] SONG BUTTON: ");
//  Serial.println( digitalRead( prButtonPin ) );
//
//
//  Serial.print("[A] BELL BUTTON: ");
//  Serial.println( analogRead( bellButtonPin ) );
//
//  Serial.print("[A] CHALK BUTTON: ");
//  Serial.println( analogRead( chalkButtonPin ) );
//
//  Serial.print("[A] SONG BUTTON: ");
//  Serial.println( analogRead( prButtonPin ) );
//  
  
  // No client connected  
  if( digitalRead( chalkButtonPin ) == HIGH ){
    warn("A");
    dispense_chalk();
  }
  
  if( digitalRead( bellButtonPin) == HIGH ){
    warn("B");
    ring_pr_bell(); 
  }

  if( digitalRead( prButtonPin ) == HIGH ){
    warn("C");
    no_client_play_track(); 
  }
  
  delay(100); // DEBOUNCE

  EthernetClient client = server.available();
  if (client) {

    while( client.connected() ){
      lcd_on();
      led_on();

      // If client data has been recieved
      if( client.available() ){
        String data = recv_data( client );

        if( data == "HELLO" ){
          lcd_print("Connected...");
          delay(1500);
          lcd_default();
        }
        else if( data == "POWER" ){
          lcd_print("PERSONAL RECORD!");
          delay(1500);
        }else if( data == "CHALK" ){
          dispense_chalk();
        }else if( data == "BELL" ){
          ring_pr_bell();
        }
        else{
          // anything else is assumed a song
          song = data;
          lcd_print(song); 
        }
        Serial.println(data);
      }

      // Detect song button
      if( digitalRead(prButtonPin) == HIGH ){
        int now = millis();
        int held_for = 0;
        while( digitalRead(prButtonPin) == HIGH ){
          Serial.println("HELD...");  
          delay(10);
          held_for += millis() - now;
          now = millis();
          if( held_for > 1000 ){
            break;
          }
        }

        Serial.println("RELEASED, HELD FOR: ");
        Serial.println( held_for );

        if( held_for > 1000 ){
          client.println("POWER");
        }
        else{      
          client.println("PLAY");
        }
        delay(150);
      }


      // Client connected  
      if( digitalRead( chalkButtonPin ) == HIGH ){
        dispense_chalk();
      }
      
      if( digitalRead( bellButtonPin) == HIGH ){
        ring_pr_bell(); 
      }

      delay(100); // DEBOUNCE
    }
  }
}


String recv_data(EthernetClient client){
  String data;
  char c = client.read();
  while( c != '\n' ){
    data += c;
    c = client.read();
  }
  return data;
}

void lcd_on(){
  digitalWrite(lcdBacklightPin, HIGH);      // LCD ON
}

void lcd_off(){
  digitalWrite(lcdBacklightPin, LOW);      // LCD OFF
}

void lcd_clear(){
  lcd_print("");
}

void dispense_chalk(){
  warn("dispense_chalk()");
  chalkServo.attach(chalkServoPin);
  lcd_on();
  
  lcd_print("Dispensing chalk|in 3...");
  delay(1000);
  lcd_print("Dispensing chalk|in 2...");
  delay(1000);
  lcd_print("Dispensing chalk|in 1...");
  delay(1000);  
  lcd_print("Dispensing chalk|now!");
  delay(750);
  
  chalkServo.write(152);
  delay(150);
  chalkServo.write(93);
  delay(1200);
  
  lcd_print("   GET SWOLE   | LFT HVY ASS W8");
  delay(3000);
  lcd_default();
  lcd_off();
  chalkServo.detach();      
}

void ring_pr_bell(){
  warn("ring_pr_bell");
  int x=0;
  bellServo.attach(bellServoPin);

  lcd_on();
  lcd_print("PERSONAL RECORD!");

  for( x=0; x<3; x++){
    bellServo.write(0);
    delay(1000);
    bellServo.write(180);
    delay(800);
    bellServo.write(0);
    delay(800);
    bellServo.write(180);
    delay(800);
  }

  lcd_default();
  lcd_off();

  bellServo.detach();
}

void no_client_play_track(){
  lcd_print("No client connected.");
  lcd_on();
  delay(1000);
  lcd_off();
  lcd_default();
}


void lcd_default(){
  lcd_print( lcd_default_str );  
}

void lcd_print(String data){
  String line1;
  String line2;
  int index;

  index = data.indexOf('|');
  line1 = data.substring(0, index);
  line2 = data.substring(index+1);

  line1 = line1.substring(0,16);
  line2 = line2.substring(0,16);

  lcd.begin(16,2);

  lcd.setCursor(0,0);
  lcd.print(line1);
  if( index != -1 ){
    lcd.setCursor(0,1);
    lcd.print(line2);
  }
}

void led_off(){
  analogWrite(ledPin,0);
}

void led_on(){
  analogWrite(ledPin,128);
}

void warn(String str){
  Serial.println(str);
}

