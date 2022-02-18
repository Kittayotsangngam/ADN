int num[13]={63,6,91,79,102,109,125,39,127,111,61,56,0}; //10=L 11=G 12=Blank
int randomNum=0;
int idx=1;
unsigned long bounce[6];
void setup() {
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  for(int i=0;i<8;i++)
    pinMode(i+6,OUTPUT);
  randomSeed(analogRead(0));
}
void printnum(int n){
  for(int i=0;i<8;i++)
    digitalWrite(i+6,1-bitRead(num[n],i));
}
void krapib(int n){
  for(int i=0;i<5;i++){
      printnum(n);
      delay(200);
      printnum(12);
      delay(200);
  }
}
void loop() {
  if(!randomNum) randomNum=random(1,9);
  if(!digitalRead(4)&&millis()-bounce[4]>=300){
    idx++;
    if(idx>=10) idx=1;
    bounce[4]=millis();
  }
  if(!digitalRead(5)&&millis()-bounce[5]>=300){
    if(idx<randomNum){
      digitalWrite(2,1);
      krapib(11);
      digitalWrite(2,0);
     }
    else if(idx>randomNum){
      digitalWrite(2,1);
      krapib(10);
      digitalWrite(2,0);
    }
    else{
      digitalWrite(3,1);
      krapib(0);
      digitalWrite(3,0);
      idx=1;
      randomSeed(analogRead(0));
      randomNum=0;
    }
    bounce[5]=millis();
  }
  else
    printnum(idx);
}
