#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "pitches.h"

Servo myservo;                                        // 서보모터 선언

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
 
#define HEAD_SR04_TRIG   8    // 트리거 핀 선언
#define HEAD_SR04_ECHO   9    // 에코 핀 선언
#define BODY_SR04_TRIG  10    // 트리거 핀 선언
#define BODY_SR04_ECHO  11    // 에코 핀 선언

#define SERVO_PIN 6                                       // 모터 제어를 위해 6번핀(PWM) 으로 선언
#define POS_MOVE_MAX 180

#define MAX_HEAD_LENGTH   1000      // 뚜껑위 물체 인식 거리 설정값
#define FULL_BODY_LENGTH   600      // 몸통안 물체 인식 거리 설정값

int speakerpin = 3; //스피커가 연결된 디지털핀 설정
 
//음계 데이터 (4개)
int melody[] = {NOTE_C6,NOTE_E6,NOTE_G6,NOTE_C7,
                };
//음의길이, 4:4분음표, 2:2분음표
int noteDurations[] = {8,8,8,1};

int pos = 0;  
int pos_move = 0;

long head_distance, body_distance;    // 거리 측정을 위한 변수 선언

void setup()
{
   Serial.begin(9600);      // 통신속도 9600bps로 시리얼 통신 시작
   Serial.println("초음파 센서 시작");

   // 뚜껑 센서
   pinMode(HEAD_SR04_TRIG, OUTPUT);    // 트리거 핀 출력으로 선언
   pinMode(HEAD_SR04_ECHO, INPUT);     // 에코 핀 입력으로 선언

   // 통안 센서
   pinMode(BODY_SR04_TRIG, OUTPUT);    // 트리거 핀 출력으로 선언
   pinMode(BODY_SR04_ECHO, INPUT);     // 에코 핀 입력으로 선언

     pinMode (SERVO_PIN, OUTPUT);           // 모터 제어핀을 출력으로 설정
     myservo.attach(SERVO_PIN);                                // 모터의 신호선을 6번핀에 연결

     delay(200);
     
    // 센서값 초기화
     myservo.write(pos);

    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.backlight();

    lcd_display(0, "Hello.");
    lcd_display(1, "I'm Trashcan");

}

//라인출력 라이브러리 0: 첫번째, 1:두번째
void lcd_display(int line, char str[])
{
  lcd.setCursor(0,line);
  lcd.print(str);
}

void move_servo()
{
  if(pos_move != 0)
  {
    pos = pos + pos_move;   
  }

  if(pos < 0)
  {
    pos = 0;
    myservo.write(pos);
    pos_move = 0; 

    lcd_display(0, "Thank you!      ");
    lcd_display(1, ":)              ");

    delay(2000);
    lcd_display(0, "Hello.          ");
    lcd_display(1, "I'm Trashcan    ");

  }
  
  if(pos >= POS_MOVE_MAX)
  {
    pos = POS_MOVE_MAX;
    myservo.write(pos); 
    pos_move = pos_move * -1;
  }

  if(pos > 0 && pos < POS_MOVE_MAX)
  {
    myservo.write(pos); 
  }


  Serial.print("pos : ");
  Serial.print(pos);
  Serial.print(", move : ");
  Serial.println(pos_move);
  
}



long get_distance(int SR04_TRIG, int SR04_ECHO)
{
   long duration, distance;    // 거리 측정을 위한 변수 선언

   digitalWrite(SR04_TRIG, LOW);        // Trig 핀 Low
   delayMicroseconds(2);            // 2us 딜레이
   digitalWrite(SR04_TRIG, HIGH);    // Trig 핀 High
   delayMicroseconds(10);            // 10us 딜레이
   digitalWrite(SR04_TRIG, LOW);        // Trig 핀 Low

    /*
        에코핀에서 받은 펄스 값을 pulseIn함수를 호출하여
        펄스가 입력될 때까지의 시간을 us단위로 duration에 저장
        pulseln() 함수는 핀에서 펄스(HIGH or LOW)를 읽어서 마이크로초 단위로 반환
   */
   duration = pulseIn(SR04_ECHO, HIGH);  
   distance = duration * 170 / 1000;  
}

void buzzer_1(){
  for (int thisNote = 0; thisNote < 4; thisNote++)
  {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(speakerpin, melody[thisNote], noteDuration); //소리를 낸다.
    int pauseBetweenNotes = noteDuration * 1.30;      //delay 계산식
    delay(pauseBetweenNotes);                         //delay
    noTone(speakerpin);                               //대상핀 출력 중단
  }
}

void buzzer_2(){
 tone(3, 1047, 1000);
 delay(3000);
 noTone(3);
}

void loop()
{
   head_distance = get_distance(HEAD_SR04_TRIG, HEAD_SR04_ECHO);
   body_distance = get_distance(BODY_SR04_TRIG, BODY_SR04_ECHO);
 
   /*
        음파의 속도는 초당 340m, 왕복하였으니 나누기 2를하면 170m이고,
        mm단위로 바꾸면 170,000mm.
        duration에 저장된 값은 us단위이므로 1,000,000으로 나누어 주고,
        정리해서 distance에 저장 
   */
 
   Serial.print("뚜껑 거리: ");
   Serial.print(head_distance); // 거리를 시리얼 모니터에 출력
   Serial.print("mm, ");
   Serial.print("몸통 거리: ");
   Serial.print(body_distance); // 거리를 시리얼 모니터에 출력
   Serial.println("mm");

    // MAX_HEAD_LENGTH 거리안에 물체가 인식되면 모터를 욺직인다.
  if( head_distance < MAX_HEAD_LENGTH && pos_move == 0)
   
   {
    pos_move = 60;
    lcd_display(0, "Good job!    ");
    lcd_display(1, "Wait a second...");
    buzzer_1();
    
   }
   else if( body_distance < FULL_BODY_LENGTH)
  {
    buzzer_2();
    lcd_display(0, "I'm FULL!!     ");
    lcd_display(1, "               ");
    }
   move_servo();
   delay(100);
}
   
   
