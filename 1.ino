bool state[10];
unsigned long time[10];
int cou=4;
void setup()
{
  //Serial.begin(9600);
  for(int i=4;i<=6;i++){
    pinMode(i, OUTPUT);
    digitalWrite(i,LOW);
  }
  for(int i=7;i<=8;i++)
    pinMode(i, INPUT);
  pinMode(9,INPUT_PULLUP);

}
void loop()
{
  //Serial.print(time[8]);Serial.print(" ");Serial.println(millis());
  
  for(int i=7;i<=9;i++)
    state[i]=digitalRead(i);
  //pin 4,7
  if(state[7]&&millis()-time[7]>300){
    time[7]=millis();
    digitalWrite(4,1-digitalRead(4)); 
  }
  else if(!state[7]){
    if(millis()-time[7]>=3000)
      digitalWrite(4,LOW);
  }
  //pin 6,9
  if(!state[9]&&millis()-time[9]>300){
    time[9]=millis();
    if(digitalRead(4)||digitalRead(6))
      digitalWrite(6,LOW); 
    else
      digitalWrite(6,HIGH);
  }
  else if(state[9]){
    if(millis()-time[9]>=3000)
      digitalWrite(6,LOW);
  }
  //pin 5,8
  if(!state[8]&&millis()-time[8]>300){
    if(!digitalRead(4)&&!digitalRead(6)){
      cou=0;
    }
  }
  else if(state[8]){
    if(cou>=4)
      digitalWrite(5,LOW); 
    else if((millis()-time[8])>500){
      cou++;
      time[8]=millis();
      digitalWrite(5,1-digitalRead(5)); 
    }
  }
}
