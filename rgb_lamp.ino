/* 
  RGB Lamp
  by Bithead942

  Using an IKEA Dudero as a base, wrap an RGB LED light strand around the center post and attach the stack below to control it.
   
Arduino Uno +
Ethernet Shield +
RGB LED Shield

   The Arduio sketch provides a web server which can be accessed via any browser on the network.  The browser-based HTML interfce makes it easy to make light changes to the lamp from any kind of connected device.


Pins used:
    Ethernet Shield
      SCK                          13
      MISO                         12
      MOSI                         11
      SS for Ethernet Controller   10
      SS for SD Card                4
    
    RGB Shield
      Red:    3
      Green:  5
      Blue:   6

 */

#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

int red = 3;    // RED LED connected to PWM pin 3
int green = 5;    // GREEN LED connected to PWM pin 5
int blue = 6;    // BLUE LED connected to PWM pin 6
int r=50; int g=100; int b=150;
int rup; int gup; int bup;
int fader=0;
int inc=10;
String inString = String(50);
char buffer[160]; // make sure this is large enough for the largest string it must hold

byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x1D, 0x01 };
byte ip[] = { 192,168,1,194 };

prog_char string_0[] PROGMEM  = "<html><head><title>Ethan's Lamp</title></head><script language=\"javascript\"> var alphaStr = \"0123456789ABCDEF\";";
prog_char string_1[] PROGMEM  = "var alphaArr = [\"0\",\"1\",\"2\",\"3\",\"4\",\"5\",\"6\",\"7\",\"8\",\"9\",\"A\",\"B\",\"C\",\"D\",\"E\",\"F\"]; var RGB = [";
prog_char string_2[] PROGMEM  = "];function pulsRED(){document.bgColor='#FF0000';RGB = [255,0,0]}function pulsGRE(){document.bgColor='#00FF00';RGB = [0,255,0]}";
prog_char string_3[] PROGMEM  = "function pulsBLU(){document.bgColor='#0000FF';RGB = [0,0,255]}function pulsYEL(){document.bgColor='#FFFF00';RGB = [255,255,0]}";
prog_char string_4[] PROGMEM  = "function pulsPIN(){document.bgColor='#FF00FF';RGB = [255,0,255]}function pulsCEL(){document.bgColor='#00FFFF';RGB = [0,255,255]}";
prog_char string_5[] PROGMEM  = "function pulsWHI(){document.bgColor='#FFFFFF';RGB = [255,255,255]}";
prog_char string_6[] PROGMEM  = " ";
prog_char string_7[] PROGMEM  = "function HEX_from_RGB(){document.bgColor = '#' + DEC_to_HEX(RGB[0]) + DEC_to_HEX(RGB[1]) + DEC_to_HEX(RGB[2]);}";
prog_char string_8[] PROGMEM  = "function adjRED(incr){	RGB[0] += incr;	if (RGB[0] > 255) RGB[0] = 255;	if (RGB[0] < 0) RGB[0] = 0;}";
prog_char string_9[] PROGMEM  = "function adjGRN(incr)	{RGB[1] += incr;if (RGB[1] > 255) RGB[1] = 255;	if (RGB[1] < 0) RGB[1] = 0;}";
prog_char string_10[] PROGMEM  = "function adjBLU(incr)	{RGB[2] += incr;if (RGB[2] > 255) RGB[2] = 255;	if (RGB[2] < 0) RGB[2] = 0;}";
prog_char string_11[] PROGMEM = "function setRED(r) { RGB[0] = r; } function setGRN(g) { RGB[1] = g; } function setBLU(b) { RGB[2] = b; }";
prog_char string_12[] PROGMEM = "function newRGB() { HEX_from_RGB(); } </script> <script language=\"javascript\"> var i_a; ";
prog_char string_13[] PROGMEM = "function DEC_to_HEX(dec) {var n_ = Math.floor(dec / 16); var _n = dec - n_ * 16; return alphaArr[n_] + alphaArr[_n];	}";
prog_char string_14[] PROGMEM = "function HEX_to_DEC(hex){var n_ = alphaStr.indexOf(hex.substring(0,1)); var _n = alphaStr.indexOf(hex.substring(1,2)); return n_ * 16 + _n;	}";
prog_char string_15[] PROGMEM = "function updateFields(){var d = document.colForm;d.r.value = RGB[0];	d.g.value = RGB[1];	d.b.value = RGB[2];	}";
prog_char string_16[] PROGMEM = "function setRGB(){var d = document.colForm; var R = d.r.value;	var G = d.g.value;	var B = d.b.value;";
prog_char string_17[] PROGMEM = "setRED(parseInt(R));	setGRN(parseInt(G));	setBLU(parseInt(B));	newRGB(); updateFields();} </script>";
prog_char string_18[] PROGMEM = "</head><body><center><form method=GET, name=\"colForm\"><h1>ETHAN'S RGB LAMP</h1> <table> <tr> <td valign=\"top\"> <input type=\"text\" name=\"r\"> ";
prog_char string_19[] PROGMEM = "<input type=\"text\" name=\"g\"> <input type=\"text\" name=\"b\"> <input type=\"submit\" value=\"Calculate\" onclick=\"setRGB();\">";
prog_char string_20[] PROGMEM = " </td></tr> <tr><td>";
prog_char string_21[] PROGMEM = "<CENTER><input type=\"submit\"  value=\"Red\" style=\" width:72;height:44px;background-color:FF0000;\" onclick=\"pulsRED();updateFields();\";>";
prog_char string_22[] PROGMEM = "<input type=\"submit\" value=\"Green\" style=\" width:72;height:44px;background-color:00FF00;\" onclick=\"pulsGRE();updateFields();\";>";
prog_char string_23[] PROGMEM = "<input type=\"submit\" value=\"Blue\" style=\" width:72;height:44px;background-color:0000FF;\" onclick=\"pulsBLU();updateFields();\";>";
prog_char string_24[] PROGMEM = "<input type=\"submit\" value=\"Yellow\" style=\" width:72;height:44px;background-color:FFFF00;\" onclick=\"pulsYEL();updateFields();\";>";
prog_char string_25[] PROGMEM = "<input type=\"submit\" value=\"Purple\" style=\" width:72;height:44px;background-color:FF00FF;\" onclick=\"pulsPIN();updateFields();\";>";
prog_char string_26[] PROGMEM = "<input type=\"submit\" value=\"Teal\" style=\" width:72;height:44px;background-color:00FFFF;\" onclick=\"pulsCEL();updateFields();\";>";
prog_char string_27[] PROGMEM = "<input type=\"submit\" value=\"White\" style=\" width:72;height:44px;background-color:FFFFFF;\" onclick=\"pulsWHI();updateFields();\";></CENTER>";
prog_char string_28[] PROGMEM = "</table> </form> <script language=\"javascript\">newRGB(); updateFields(); ";
prog_char string_29[] PROGMEM = "i_a = parseInt(document.colForm.inc.options[document.colForm.inc.selectedIndex].value);</script>";
prog_char string_30[] PROGMEM  = "<br></center></body></html>";

PROGMEM const char *string_table[] = // change "string_table" name to suit
{
string_0,
string_1,
string_2,
string_3,
string_4,
string_5,
string_6,
string_7,
string_8,
string_9,
string_10,
string_11,
string_12,
string_13,
string_14,
string_15,
string_16,
string_17,
string_18,
string_19,
string_20,
string_21,
string_22,
string_23,
string_24,
string_25,
string_26,
string_27,
string_28,
string_29,
string_30
};


// Initialize the Ethernet server library
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{
  // start the Ethernet connection and the server:
      Serial.begin(9600);
      Ethernet.begin(mac, ip);
      server.begin();
      Serial.println("Serial READY");
      Serial.println("Ethernet READY");
      Serial.println("Server READY");
      r = EEPROM.read(1);
      g = EEPROM.read(2);
      b = EEPROM.read(3);
      inc = EEPROM.read(4);
      fader = EEPROM.read(5);
} 

void loop()  {
      EthernetClient client = server.available();

      if (client) {
        Serial.println("client");
        // an http request ends with a blank line
        boolean current_line_is_blank = true;
        while (client.connected()) {

          if (client.available()) {

            char c = client.read();
            if (inString.length() < 50) {
            inString.concat(c);

            } 

            if (c == '\n' && current_line_is_blank) {
                char colorArr[5];
                String temp="";
                Serial.print("inString: ");
                Serial.println(inString);

                int Pos_r = inString.indexOf("r");
                int Pos_g = inString.indexOf("g");
                int Pos_b = inString.indexOf("b");
                //int Pos_i = inString.indexOf("inc");
                //int Pos_f = inString.indexOf("fad");
                int End = inString.indexOf("HTTP");
                /*Serial.print("Pos_r: ");
                Serial.println(Pos_r);
                Serial.print("Pos_g: ");
                Serial.println(Pos_g);
                Serial.print("Pos_b: ");
                Serial.println(Pos_b);
                Serial.print("Pos_i: ");
                Serial.println(Pos_i);
                Serial.print("Pos_f: ");
                Serial.println(Pos_f);
                Serial.print("End: ");
                Serial.println(End); */

                if(Pos_r>=0){
                   temp=inString.substring((Pos_r+2), (Pos_g-1));
                   temp.toCharArray(colorArr, 5);
                   r=(atoi(colorArr));
                   //Serial.print("red: ");
                   //Serial.println(r);
                   EEPROM.write(1, r);
                }
                if(Pos_g>=0){
                   temp=inString.substring((Pos_g+2), (Pos_b-1));
                   temp.toCharArray(colorArr, 5);
                   g=(atoi(colorArr));
                   //Serial.print("green: ");
                   //Serial.println(g);
                   EEPROM.write(2, g);
                }         

                if(Pos_b>=0){
                   temp=inString.substring((Pos_b+2), (End-1));
                   temp.toCharArray(colorArr, 5);
                   b=(atoi(colorArr));
                   //Serial.print("blue: ");
                   //Serial.println(b);
                   EEPROM.write(3, b);
                }      

               if ((Pos_r>=0)&&(Pos_g>=0)&&(Pos_b>=0)) {
                 rgb(r,g,b);
               }
            printWebPage( &client);
            break;
            }
            if (c == '\n') {
            // we're starting a new line
            current_line_is_blank = true;
            } else if (c != '\r') {
            // we've gotten a character on the current line
            current_line_is_blank = false;
            }
          }
        }
        // give the web browser time to receive the data
        delay(1);

        inString = "";
        client.stop();
      }

}

void printWebPage(Client *client)
{
      int tmp=0;
      Serial.println("printWebPage");
      // send a standard http response header
      client->println("HTTP/1.1 200 OK");
      client->println("Content-Type: text/html");
      client->println();

      strcpy_P(buffer, (char*)pgm_read_word(&(string_table[0]))); // Necessary casts and dereferencing, just copy.
      client->println( buffer );
      //Serial.println( buffer );
      for (int i = 1; i <= 30; i++)
      {
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table[i]))); // Necessary casts and dereferencing, just copy.
          client->println( buffer );
          if (i==1)
          {
            client->print( r );
            client->print( "," );
            client->print( g );
            client->print( "," );
            client->print( b );
          }
          delay(30);
      }

}

void rgb(int r, int g, int b)
{

  Serial.print("RGB: ");
  Serial.print(r);
  Serial.print(" ");
  Serial.print(g);
  Serial.print(" ");
  Serial.print(b);
  if (r>255) r=255;
  if (g>255) g=255;
  if (b>255) b=255;
  if (r<0) r=0;
  if (g<0) g=0;
  if (b<0) b=0;

  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}
