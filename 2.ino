bool state[13];
boolean num[12][7]{1,1,1,1,1,1,0,
            0,1,1,0,0,0,0,
                  1,1,0,1,1,0,1,
                  1,1,1,1,0,0,1,
                  0,1,1,0,0,1,1,
                  1,0,1,1,0,1,1,
                  1,0,1,1,1,1,1,
                  1,1,1,0,0,1,0,
                  1,1,1,1,1,1,1,
                  1,1,1,1,0,1,1,
                  1,0,0,0,0,0,1,
                  0,0,0,1,1,1,0};
int randomNo;
unsigned int time[13];
int check11;
int check12;
int n=1;
void setup() {
  Serial.begin(9600);
  for(int i=2;i<=8;i++)
  pinMode(i ,OUTPUT);
  for(int i=11;i<=12;i++)
  pinMode(i ,INPUT);
  randomSeed(analogRead(A0));
  randomNo= random(1, 10);
}

void loop() {
  Serial.println(randomNo);
   check11=digitalRead(11);
  check12=digitalRead(12);
  if(!check11&&millis()-time[1]>=300){
    time[1]=millis();
    n++;
    }
  else if(check11)
  {
    if(n>0&&n<=9)
    for(int i=2;i<=8;i++){
  pinMode(i ,num[n][i-2]);
   }
    if(n>9)
    { n=1;
  }
    if(!check12)
    {if(n==randomNo){
     for(int i=2;i<=8;i++&&millis()-time[0]>=500){
      time[0]=millis();
  pinMode(i ,num[0][i-2]);
     n=1;
     }
     else if(n>randomNo){
     for(int i=2;i<=8;i++&&millis()-time[2]>=500){
      time[2]=millis();
  pinMode(i ,num[10][i-2]);
     n=1;
     }
     }
    }
  }
  }
}
