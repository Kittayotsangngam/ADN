struct State{
  unsigned long ST_Out;
  unsigned long Time;
  unsigned long Next[8];
};
/*
state Explain: 
0= goNorth, 1=waitNorth, 
2= goEast, 3=waitEast,
4= goWalk, 5=waitWalk
Button Explain:
button pin 11 = North
button pin 12 = East
button pin 13 = Walk
000 001 010 011 100 101 110 111
*/
State FSM[6]={  
  {B10100001,2000,{0,0,1,1,1,1,1,1}},  // <-st0
  {B10100010,1000,{2,2,2,2,4,4,2,2}}, //<- st1
  {B10001100,2000,{2,3,2,3,3,3,3,3}}, //<- st2
  {B10010100,1000,{4,0,4,0,4,4,4,4}}, //<- st3
  {B01100100,2000,{4,5,5,5,4,5,5,5}}, //<- st4
  {B01100100,300,{0,0,2,0,0,0,2,0}} //<- st5
};
unsigned long currentState=0;
void setup() {
  for(int i=2;i<=9;i++)
    pinMode(i,OUTPUT);
  for(int i=10;i<=12;i++)
    pinMode(i,INPUT_PULLUP);
}
void loop() {
  for(int i=2;i<=9;i++){
    digitalWrite(i,FSM[currentState].ST_Out & 1<<(i-2));
  }
  if(currentState==5){
    for(int i=0;i<6;i++){
      digitalWrite(8,1-digitalRead(8));
      delay(FSM[currentState].Time);
    }
  }
  else delay(FSM[currentState].Time);
  int input = (1-digitalRead(12))*4+(1-digitalRead(11))*2+(1-digitalRead(10));
  currentState = FSM[currentState].Next[input];
}
