#include <LiquidCrystal.h>

const byte start_step=1;  //initial step; 1-> possibility to choose test number, 2-> test numer is constant, defined by start_test constant
const byte start_test=1;  //initial test;
   
int stp=start_step;       //step counter;

byte t_no=start_test;     //test number (1 2 3)
byte c_no=1;              //cycle number (1=50  2=75 3=100)
byte o_no=1;              //option number (1=start 2=stop)
int c_counter=1;          //cycle_counter
int secs=0;

byte cor=0;               //correction of bracket position

//digital input PINs definition
byte c_switch=13;         //controll switch
byte s_switch=10;         //start switch
byte b_switch=9;          //break switch
byte dc_input=0;          //110 DCV sensor
byte piezo_pin=1;
//byte dc_input=10;         //do wykasownia

//digital out PINs definition, relays controll
byte PR1_pin=6;
byte PR2_pin=7;
byte PR3_pin=8;
byte PR=PR1_pin;          //helpfully for output control 
byte pPIN;                //temporary variable

byte test_array[3][6]=    //[test_no][count_of_points_in_test,delay_in_s, , , , ]
{
  {5,2,2,7,1,2},          // 5 intervals, 2 sec delay after start, 2 sec delay, 7 sec delay, 1 s, 1 s
  {2,2,20,0,0,0},         // 2 intervals, 2 sec delay after start, 20 sec delay
  {3,2,7,10,0,0},         // 3 intervals, 2 sec delay after start, 7 sec delay, 10 sec delay
};

byte int_step=1;                  //number of internal step 
int s_push=100;                   //short push time in ms
int b_push=1000;                  //break push time in ms
unsigned long sDelay=millis();    //switch delay
unsigned long inDelay=millis();   //internal delay
unsigned long secDelay=millis();  //counting one second delay
unsigned long buzzDelay=millis();
int buzzTime=500;
int secTime=1000;                 //1000ms=1s

bool bt_down=false;               // working together refresh_display
bool refresh_disp=true;           //anti blink protection
bool test_begin=false;            //if true, means test running (in progress)
bool testFail=false;              
bool buzz=false;
//lcd 
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//reset all variables in case of end, or break test
void end_test(){
  test_begin=false; 
  int_step=1;
  c_counter=1;
  c_no=1;
  secs=0;
  digitalWrite(PR1_pin,LOW);
  digitalWrite(PR2_pin,LOW);
  digitalWrite(PR3_pin,LOW);
  testFail=false;
  cor=0;
  //sound procedure
}

// test_step(test_number)
void test_step(byte iT){
  
      //countown every one second
      if ((millis()-secDelay)>=secTime) {
        secs++;
        secDelay=millis();
      }
      //controll sub step delay
      if ((millis()-inDelay)>(test_array[iT-1][int_step]*secTime) ) {
          int_step++;
          if (int_step>test_array[iT-1][0]){
             int_step=2; //reset of internal step counter, 2 because in test_array  0 element is number of steps, 
                         //                                                         1st element is delay after push start btn
                         //                                                         2nd and next elements are delay after another steps
             testFail=false;   
             c_counter++;//counter of cycles in test
             if (c_counter>((c_no+1)*25)) {//end of test  c_no= 1..3 -> 25..75
                end_test();
                stp=6;
                bt_down=true;
             }
          }
          inDelay=millis();
          secs=0;            
      }

    //relays controll

    //prawdopodobnie ten switch obsluguje wszytskie trzy testy, do sprawdzenia, 
    //nalezy podpiac na wyjscia diody, w celu zasugnalizowania wysokich stanow na wyjsciu
    //oraz zmostkowac do masy input 0

    // poniższe już obsłużone, w kroku testowania 110 do zmiennej testFail jest przepisywana zanegowany stan pinu czytajacego, 
    // jezeli przez 1000ms nie pojawi sie wysoki sygnal to testFail bedzie true, i to spowoduje wywalenie procedury, sprawdzic czy dziala
    
    // zasanowic się czy nie dawac tam jakiegos opoznienia przy czekaniu na impuls, albo zrobic 
    //po PR3 petle while czekajaca na sygnal przez 1 sek, albo przez czas zadany w przedostatniej pozycji tablicy, 
    //czyliuzyc impulsu opozniajacego, jezeli w ciagu tego czasu nie pojawi sie sygnal to uwalic,
    //taka konstrukcja zabezpieczny przed zdesynchronizowana dostawa impulsu
    
    //switch (iT){
    //  case 1:
          pPIN=PR+(int_step-2);
          if (!testFail)
            testFail=!digitalRead(dc_input);// if dc_input == HIGH-> OK, dc_input == LOW -> fail -> testFail=TRUE 
                                            
          if (int_step==5){          //step after 110test, before next loop
              if (testFail){
                end_test();
                stp=7;
                bt_down=true;
                
                //Serial.println(int_step);
              }
          }
          if ((pPIN>=PR1_pin)&&(pPIN<=PR3_pin)){
            //if ((test_array[t_no-1][int_step]-secs)==0) //change coming in last second of substep
            digitalWrite(pPIN,HIGH); //int_step-2 = > 2-2=0, 3-2=1, 4-2=2 -> PR1, PR2, PR3
            //Serial.println(pPIN);
          }  
          //
          if ( (int_step==test_array[t_no-1][0])&&
               ((test_array[t_no-1][int_step]-secs)<=1) 
               ){
                  digitalWrite(PR1_pin,LOW);         
                  digitalWrite(PR2_pin,LOW);         
                  digitalWrite(PR3_pin,LOW);                     
          }
    //    break;
          
    //  case 2:
    //    break;

    //  case 3:
    //    break;    
    //}


}


void setup() {
  //Serial.begin(9600);
  
  pinMode(c_switch, INPUT);
  pinMode(s_switch, INPUT);
  pinMode(b_switch, INPUT);
  pinMode(dc_input, INPUT);
  pinMode(PR1_pin,OUTPUT);
  pinMode(PR2_pin,OUTPUT);
  pinMode(PR3_pin,OUTPUT);
  pinMode(piezo_pin,OUTPUT);
  
  lcd.begin(16, 2);

  digitalWrite(PR1_pin,LOW);         
  digitalWrite(PR2_pin,LOW);         
  digitalWrite(PR3_pin,LOW);         
  
}


void loop() {
  
  //display control
  //refresh_disp is depend on bt_down
  if (refresh_disp || test_begin){
    cor=0;
    switch (stp){
      case 1:
          lcd.setCursor(0,0);        
          lcd.print("Test:");
          lcd.setCursor(0,1);        
          lcd.print(" 1  2  3 ");  
          lcd.setCursor((t_no+1)*3-6,1);  
            lcd.print("[");
          lcd.setCursor((t_no+1)*3-4,1);
            lcd.print("]");
        break;

      case 2:
        lcd.setCursor(0,0); 
        lcd.print("T:");
        lcd.print(t_no);       
        lcd.print(" Cycles:");
        lcd.setCursor(0,1);        
        lcd.print(" 50  75  100 ");  //0 4 8
        lcd.setCursor((c_no-1)*4,1);  
          lcd.print("[");
        if (c_no==3) cor=1;  //correction of bracket position 
        lcd.setCursor((c_no-1)*4+3+cor,1);
          lcd.print("]");       
        break;    

      case 3:
          lcd.setCursor(0,1);        
          lcd.print(" Start  Cancel ");   
          lcd.setCursor((o_no-1)*7,1);  //0 7
            lcd.print("[");
          if (o_no==2) cor=2; //correction of bracket position 
          lcd.setCursor((o_no)*6+cor,1);    //6 14
            lcd.print("]");       
        break;  

      case 4:
          //show test number
          lcd.setCursor(0,0); lcd.print("T:");
          lcd.setCursor(2,0); lcd.print(t_no);
          
          //show cycle_counter/choosen_cycles
          lcd.setCursor(4,0);   lcd.print("C:");
          lcd.setCursor(9,0);  lcd.print("/");   
          lcd.setCursor(10,0);  lcd.print((c_no+1)*25); 
          cor=0;       
          if (c_counter<100) cor=1;  //controll of shift the digit
          if (c_counter<10)  cor=2;  //controll of shift the digit
          lcd.setCursor(6+cor,0);   lcd.print(c_counter);   

          //show internal_step/count_of_internal_steps 
          lcd.setCursor(0,1);   lcd.print(int_step);
          lcd.setCursor(1,1);   lcd.print("/");
          lcd.setCursor(2,1);   lcd.print(test_array[t_no-1][0]);  

          //show duration of internal current step
          cor=0;
          if (test_array[t_no-1][int_step]-secs<10){   //shift digit right 
              lcd.setCursor(7,1);   
              lcd.print(" ");
              cor=1;
          }
          lcd.setCursor(7+cor,1);   lcd.print(test_array[t_no-1][int_step]-secs);             
          lcd.setCursor(10,1);   lcd.print("sek.");      

        break; 

      case 5:
          lcd.setCursor(0,0);
          lcd.print("  Test stopped  ");
          lcd.setCursor(0,1);
          lcd.print("      [OK]      "); 
          buzz=true;        
        break;   

      case 6:
          lcd.setCursor(0,1);
          lcd.print("  Test passed.  ");
          buzz=true;
        break;
        
      case 7:
          lcd.setCursor(0,0);
          lcd.print("  Test failed.  ");
          lcd.setCursor(0,1);
          lcd.print("      [OK]      ");         
          buzz=true;
        break;    
    }
    refresh_disp=false;
  }

//logic control
  //break switch
  if ((digitalRead(b_switch)==HIGH)&& test_begin){
    if ((millis()-sDelay)>=b_push){
      end_test();
      stp=5;        // for information -> show "test stopped"message
      bt_down=true; // to refresh display
     
    }
  } else
  //start switch press 
  if ((digitalRead(s_switch)==HIGH)&&(!test_begin)){         //!test_begin is blocking start switch during test  
     if ( ( (millis()-sDelay)>=s_push) && (!bt_down) ) { 
        bt_down=true;
        if (stp==3 and o_no==2) {
          //cancel option, go to start
          stp=start_step-1;//have to be -1 because of inc in next line
        }
        stp++;
        if (stp>4) stp=start_step;
        lcd.clear(); 
        if (stp==4)   {
          test_begin=true; 
          buzz=true;
        } 
        //reset of starting points
        inDelay=millis();
        secDelay=inDelay;          
     }
  } 
  //control switch press
  else if ((digitalRead(c_switch)==HIGH)&&(!test_begin)){  //!test_begin is blocking controll switch during test 
      if ( ( (millis()-sDelay)>=s_push) && (!bt_down) ) {        
        bt_down=true;
        switch (stp) {
          case 1:  //test choice
              t_no++;  
              if (t_no>3) t_no=1;
              break;
              
          case 2:  //cycle amount
              c_no++;
              if (c_no>3) c_no=1;
              break;

          case 3:  //confirmation 
              o_no++;
              if (o_no>2) o_no=1;
              break;   

          case 4: //test step
              //a tu sie nic nie dzieje, bo to jest obsluzone przez procedure test_step;
              break;            
        }
      }   
  } else{
    sDelay=millis();
    bt_down=false;
  }

  if (test_begin) test_step(t_no);

  refresh_disp=bt_down; //musi byc na końcu, ze wzgledu na bt_down eksplatowany w test_step()

  if  (buzz){
    tone(piezo_pin, 1000);
    buzzDelay=millis();
    buzz=false;
  }
  if ((millis()-buzzDelay)>buzzTime)
    noTone(piezo_pin);
  
}






