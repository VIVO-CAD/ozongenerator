/* Steuerung Ozongenerator für Institut Dr. Rilling                          */
/*            Hr. Jochen Seipel                                              */
/*            Hr. Chistian Mack                                              */
/* (Prozessor 18F2420)                                                       */
/*                                                                           */
/* Autor:      Dr.-Ing. Peer Dahl - IND, Pforzheim                           */
/* Version:    2.1 - 09. Juni 2023                                           */
/*                                                                           */
/* Geschichte: 1.0 - 07. Juli 2022    : Grundversion                         */
/*             1.0 - 07. Juli 2022    : 1. Release                           */
/*             2.0 - 16. März 2023    : Auslieferung                         */
/*             2.1 - 09. Juni 2023    : Leistungsstufe 4 nicht mehr via      */
/*                                      Extra-LED LED 5 (Pink) angezeigt,    */
/*                                      sondern durch Leuchten aller         */
/*                                      Stufen 1-3                           */
/*             3.0 - 26. Februar 2024 : Anbindung des Druckschalters an AN4  */   
/*                                      PIN 7                                */
/*             3.1 - 05. März 2024    : Überarbeitung Version 3.0            */  
/*             3.2 - 10. April 2024   : 3-Sekunden-Ein/Aus-Taster            */
/*                                      Rote LED nicht mehr permanent an,    */
/*                                      sondern nur ohne Ozonanforderung in  */
/*                                      Ruhe.                                */
/*             3.3 - 04. Mai 2024     : Abssenkung d. Druck-ADC-Schwelle 3.5V*/
/*             3.4 - 20. Juni 2024    : 3-Sekunden-Ein/Aus-Taster            */
/*                                      nicht mehr als Extra-Taster, sondern */
/*                                      kombiniert mit Folientaster          */
/*             3.5 - 29. Juni 2024    : Druck-ADC-Messung mit MAV-Filter     */
/*             3.6 - 24. Juli 2024    : Pegelumpolung d. Generatorschalt-FETs*/
/*                                      Druck-ADC-Messung mit exüonentiellem */
/*                                      Filter, anstatt mit MAV-Filter.      */

/** I N C L U D E S **********************************************************/ 
#include <p18cxxx.h> 
#include "delays.h"                        // für die Warteschleife 

#include <stdlib.h>
#include <timers.h>


//#include <usart.h>

/* Globele Variablen */
unsigned uDruckschalterWertADC = 0;      // Wird in Funktion LeseADC() gesetzt
//extern union USART USART_Status;

  
/** Configuration ************************************************************/ 
#pragma config OSC = HS      // CPU=20 MHz 
#pragma config PWRT = ON 
//#pragma config BOR = OFF 
#pragma config WDT = OFF     // War OFF,Watchdog Timer 
#pragma config LVP = OFF     // Low Voltage ICSP 
   
#pragma config PBADEN = OFF  // AD-Wandler des Port B abschalten. Geschieht 
                             // gleich noch mal detaillierter.
//#pragma config ICS = PGx3    // Neue Zeile, um den Debugger mit Pickit zu verbinden.

#pragma config DEBUG = ON     // Experiment 20.4.2024, fürs Debugging, das Bit
                              // wird beim Debugging-Compile sowieso gesetzt und
                             // stört wegen statisch das normale Build

char cPORTA_Out=0;           // Bei Port A und C gibt es Ausgänge, daher den 
char cPORTB_Out=0;           // LATch-Inhalt merken
char cPORTC_Out=0;           // LATch-Inhalt merken
char BIT0=1;
char BIT1=2;
char BIT2=4;
char BIT3=8;
char BIT4=16;
char BIT5=32;
char BIT6=64;
char BIT7=128;




/** F U N K T I O N E N ******************************************************/ 

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion setzt ein Bit im Ausgangsport A und verhindert, daß */
/* dieses Setzen Einfluss auf die anderen Bits hat.                          */
void SetzeA(char cBit)
   {
   cPORTA_Out = cPORTA_Out | cBit;
   PORTA = cPORTA_Out;
   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion löscht ein Bit im Ausgangsport A und verhindert, daß*/
/* dieses Setzen Einfluss auf die anderen Bits hat.                          */
void LoescheA(char cBit)
   {
   cPORTA_Out = cPORTA_Out & (0xFF-cBit);
   PORTA = cPORTA_Out;
   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion setzt ein Bit im Ausgangsport B und verhindert, daß */
/* dieses Setzen Einfluss auf die anderen Bits hat.                          */
void SetzeB(char cBit)
   {
   cPORTB_Out = cPORTB_Out | cBit;
   PORTB = cPORTB_Out;
   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion löscht ein Bit im Ausgangsport B und verhindert, daß*/
/* dieses Setzen Einfluss auf die anderen Bits hat.                          */
void LoescheB(char cBit)
   {
   cPORTB_Out = cPORTB_Out & (0xFF-cBit);
   PORTB = cPORTB_Out;
   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion führt ein Delay aus, dass die gesamte Bitlänge incl.*/
/* Setzen der Bits auf genau 4 us festlegt                                   */
void DelayPort(void)
{
Nop();

Nop();
Nop();
Nop();
Nop();
Nop();
Nop();
Nop();
Nop();
Nop();
Nop();

return;
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die Funktion läßt eine LED so cZaehler-oft blinken.                       */ 
void blinker(unsigned char ucZaehler, char cImmer)
{
int i;
char cEndlos;

cEndlos = 1;

while(cEndlos)
   {
   SetzeA(BIT1);                        // LED aus
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   if (ucZaehler>0)
     { 
     for (i=0; i<ucZaehler; i++)
       {
       LoescheA(BIT1);                // LED An
       Delay10KTCYx(100);
       SetzeA(BIT1);                  // LED aus
       Delay10KTCYx(100);
       }
     }
     else
     {
     LoescheA(BIT1);                  // LED An
     Delay10KTCYx(10);
     }
   if (cImmer==0) cEndlos = 0;
   }
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion setzt ein Bit im Ausgangsport C und verhindert, daß */
/* dieses Setzen Einfluss auf die anderen Bits hat.                          */
void SetzeC(char cBit)
   {
   cPORTC_Out = cPORTC_Out | cBit;
   PORTC = cPORTC_Out;
   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion löscht ein Bit im Ausgangsport C und verhindert, daß*/
/* dieses Setzen Einfluss auf die anderen Bits hat.                          */
void LoescheC(char cBit)
   {
   cPORTC_Out = cPORTC_Out & (0xFF-cBit);
   PORTC = cPORTC_Out;
   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion überwacht AD-Eingang AN0 und AN4 und veranlasst in  */
/* größeren Zeitabständen die Aktualisierung des Pegelwerts.                 */
/* Da es nur einen AD-Wandler gibt, werden wechselseitig AN0 und AN4         */
/* abgefragt. Der neue Druckschalter-Wert AN4 wird global übergeben.         */
unsigned LeseADWandler(void)
   {
    

   static char cWandlerPingPong = 0; // Welcher AN-Eingang wird abgefragt?
   static unsigned suZaehler=0;      // Der Poti-Wert, der hier abgefragt wird,
                                     // ändert sich nicht so schnell. Daher
                                     // kann man den Spannungswert ab und zu
                                     // einmal updaten.
   static unsigned suPegelPT100=1024;// Ansonsten gibt man einen gemerkten 
                                     // Wert zurück. Hier PT100.
   static unsigned suPegelDruck=1024;// Ansonsten gibt man einen gemerkten 
                                     // Wert zurück. Hier Druck.


   // PT100 abfragen
   if (cWandlerPingPong == 0)
       {
   
       if (ADCON0bits.GO==0)
          {
          suPegelPT100 = ADRES;
          suPegelPT100 = suPegelPT100 & 0x3FF; 
          }

       }
   
   // Druckschalter abfragen
   if (cWandlerPingPong == 1)
       {
   
       if (ADCON0bits.GO==0)
          {
          suPegelDruck = ADRES;
          uDruckschalterWertADC = suPegelDruck & 0x3FF; 
          }
       else
          { 
          uDruckschalterWertADC = suPegelDruck; // ADC-Wert für den Druck zurückgeben
          }
        }   
   
    suZaehler++;
    if (suZaehler>0x003F)    
       {
       suZaehler = 0;
       
       if (cWandlerPingPong == 0)    // Wandlereingang wechseln
           {
           cWandlerPingPong = 1;
           
           ADCON0bits.CHS0=0;         // AN4 einstellen, siehe Kapitel 19.1, 
           ADCON0bits.CHS1=0;         // Seite 225   
           ADCON0bits.CHS2=1;       
           ADCON0bits.CHS3=0;
           }
           else 
           {
           cWandlerPingPong = 0;
           
           ADCON0bits.CHS0=0;            // AN0 einstellen, siehe Kapitel 19.1, 
           ADCON0bits.CHS1=0;            // Seite 225 
           ADCON0bits.CHS2=0;         
           ADCON0bits.CHS3=0; 
           }
       
       ADCON0bits.GO=1;              // AD-Wandlung neu starten
       }

   return suPegelPT100; // ADC-Wert für PT100 Temperaturmessung zurückgeben
 
   }  
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion liest einen Port als Tastendruck ein und gibt eine  */
/* 1 aus, wenn die Taste stabil gedrückt wurde (Low).                        */
int LeseTastePortB_Bit4_Taste_Up(void)
   {
   unsigned long ulDauerzaehler;
   char cPortBit;
   
   cPortBit = PORTBbits.RB4;

   ulDauerzaehler = 0;
   
   if (cPortBit == 0)                 // Taste gedrückt
        {
        while(1)                      // Spikes vermeiden 
            {
            cPortBit = PORTBbits.RB4;

            if (cPortBit == 0)        // Taste noch immer gedrückt
                {
                ulDauerzaehler++;
                if(ulDauerzaehler >1000)
                    {
                    return 1;          // Taste stabil gedrückt
                    }
                }
                else
                {
                return 0;
                }
            }
        }
        else
        {
        return 0;    
        }    
            
    }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion liest einen Port als Tastendruck ein und gibt eine  */
/* 1 aus, wenn die Taste neu gedrückt wurde (Low).                               */
int LeseTastePortB_Bit5_Taste_Down(void)
   {
   static int iTasteGedrueckt = 0; 
   unsigned long ulDauerzaehler;
   char cPortBit; 
   
   cPortBit = PORTBbits.RB5;




   if ((iTasteGedrueckt == 1)&&(cPortBit ==0)) // Taste war schon gedrückt
      {
      return 0;   // Keinen erneuten Tastendruck mehr ausgeben, auch wenn gedrückt.
      }

   ulDauerzaehler = 0;

   while(1)                      // Spikes vermeiden 
      {
      cPortBit = PORTBbits.RB5;

      if (cPortBit ==0)          // Taste gedrückt
          {
          ulDauerzaehler++;
          if(ulDauerzaehler >1000)
              {
              iTasteGedrueckt = 1;
              return 1;
              }
          }
          else
          {
          iTasteGedrueckt = 0;
          return 0;
          }
       }
    }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion liest einen RB5 und RC0 ein und gibt eine           */
/* 1 aus, wenn es eione Druckanforderung gibt.                               */
int LeseTastePortB_Bit5_PortC_Bit0_Druck(void)
   {
   char cPortBitRB5;
   char cPortBitRC0;
    
   cPortBitRB5 = PORTBbits.RB5;      // Taste Druck-Simulation, aktiv Low
   cPortBitRC0 = PORTCbits.RC0;      // Drucksensor, aktiv High  

   if ((cPortBitRB5 == 0) || (cPortBitRC0 != 0))  
      {
      return 1;   // Druckanforerung
      }
      else
      {
      return 0;   // Keine Druckanforderung
      }

   }
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion liest RB5 und prüft den ADC-Schwellwert für den    */ 
/* Druck-Schalter und gibt eine 1 aus, wenn es eine Druckanforderung gibt.                               */
int LeseTastePortB_Bit5_CheckeDruckADCWert(unsigned uDruckschalterWertADC)
   {
   #define SKALIERUNG 10                // Der ADC-Wert wird exponentiell
                                        // gefiltert. Um keine Float-Variable zu
                                        // brauchen, wird mit diesem Faktor der
                                        // Zahlenraum vergrößert.
   #define FILTERPARAMETER_1 0.95       // Diese beiden Filterparameter müssen 
   #define FILTERPARAMETER_2 0.05       // in der Summe 1.0 ergeben.
                                        // Ein neuer Filterwert ergibt sich aus
                                        // aus einem Teil des altem Filterwerts 
                                        // und einem Teil des meuen Messwerts. 
                                        // je größer FILTERPARAMETER_1 ist, 
                                        // desto weniger wird gemittelt und 
                                        // schneller ist die Reaktion.
    
   static unsigned uADCFilterWert = 0;  // Speicher für den aktuellen bzw. 
                                        // letzten Filterwert.
            
   static int iInitFilter = 1;          // Filter beim Start vorblegen
     
   char cPortBitRB5;
   int i;
   
   /* HIER WIRD DIE SCHWELLE EINGESTELLT, AB DER EIN UNTERDRUCK FESTGESTELLT */
   /* WIRD: Z.B. 3.5 Volt (AUS) -> 0 Volt (EIN)                              */
   /* Der  ADC bezieht sich auf die Betriebsspannung von 5 Volt.             */
   /* Der Faktor 1.0 ist experimentell ermittelt, da der AD-Wandler nicht    */
   /* so genau ist.    
   /* Es gibt für die Hysterese einen Einschaltwert und einen Ausschaltwert. */
/*   
   static unsigned uSchwellWertADC_On  = 
         SKALIERUNG * 1024 * 3.0 * 1.0 / 5; // Schwelle für 3.0 Volt (Ozon An)
   static unsigned uSchwellWertADC_Off = 
         SKALIERUNG * 1024 * 3.5 * 1.0 / 5; // Schwelle für 3.5 Volt (Ozon Aus)
*/
   static unsigned uSchwellWertADC_On  = 
         SKALIERUNG * 1024 * 1.15 * 1.0 / 5; // Schwelle für 1.20 Volt (Ozon An)
   static unsigned uSchwellWertADC_Off = 
         SKALIERUNG * 1024 * 1.18 * 1.0 / 5; // Schwelle für 1.22 Volt (Ozon Aus)

   static unsigned uOzonEinOderAus = 0;          // 1: Ozonanfoderung, 0: keine 

   if(iInitFilter == 1)                          // Beim Start Filter vorbelegen
       {
       uADCFilterWert = uDruckschalterWertADC * SKALIERUNG;
       iInitFilter = 0;
       }
   
   // Ein neuer Filterwert besteht zum größten Teil ais dem alten Filterwert und 
   // zu einem geringen Teil aus dem neuen Messwert:
   uADCFilterWert = FILTERPARAMETER_1 * uADCFilterWert + 
                    FILTERPARAMETER_2 * (uDruckschalterWertADC * SKALIERUNG);
   
 
   cPortBitRB5 = PORTBbits.RB5;      // Taste Druck-Simulation, aktiv Low !
   
   if (cPortBitRB5 == 0)             // OZON-Anforderung über den Testschalter
       {
       uOzonEinOderAus = 1;          // Druckanforderung
       return uOzonEinOderAus;       // Gleich abbrechen, ADC nicht beachten.
       }

   if(uOzonEinOderAus == 1)    
       {
       if(uADCFilterWert > uSchwellWertADC_Off) 
           {
           uOzonEinOderAus = 0;          // Druckanforderung beenden
           }
       }
       else
       {
       if(uADCFilterWert < uSchwellWertADC_On) 
           {
           uOzonEinOderAus = 1;          // Druckanforderung beginnen
           }      
       }
       
return uOzonEinOderAus;                  // Ozon-Erzeugung ein- oder ausschalten   
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion liest RB3 (EinAus-Geschaltety-Taste)und gibt eine 1 */
/* aus, wenn die Taste gedrückt ist, sonst 0.                                */
int LeseTastePortB_Bit3_TasteEinAusgeschaltetX(void)
   {
   char cPortBitRB3;

  
   cPortBitRB3 = PORTBbits.RB3;      // Taste Standby, aktiv Low
   

   if (cPortBitRB3 == 0)  
      {
      return 1;   // EinAusgeschaltet-Anforderung
      }
      else
      {
      return 0;   // Keine EinAusgeschaltet-Anforderung
      }

   }
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* Die folgende Funktion zeigt die Ozonstufen-LEDs an oder löscht sie.       */
/* Wenn cLEDEin == 0, dann leuchtet keine LED.                               */
void LED_Anzeige_Ozonstufe(char cOzonstufe, char cLEDEin)
{
if (cLEDEin == 1)
   {
   switch (cOzonstufe)
       {
       case 1: 
           LoescheA(BIT2); // Blaue LED an
           SetzeA(BIT3);   // Grüne LED aus
           SetzeA(BIT4);   // Gelbe LED aus
       break;

       case 2: 
           SetzeA(BIT2);   // Blaue LED aus
           LoescheA(BIT3); // Grüne LED an
           SetzeA(BIT4);   // Gelbe LED aus
       break;

       case 3: 
           SetzeA(BIT2);   // Blaue LED aus
           SetzeA(BIT3);   // Grüne LED aus
           LoescheA(BIT4); // Gelbe LED an
       break;

       case 4: 
           LoescheA(BIT2);   // Blaue LED an
           LoescheA(BIT3);   // Grüne LED an
           LoescheA(BIT4);   // Gelbe LED an
       break;
       }
   }
   else
   {
   SetzeA(BIT2);   // Blaue LED aus
   SetzeA(BIT3);   // Grüne LED aus
   SetzeA(BIT4);   // Gelbe LED aus
   }   
   
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion schaltet den Lüfter ein und aus.                    */
void LuefterEin_Aus(char cLuefterEinAus)
{
if (cLuefterEinAus == 1)
   {
   SetzeC(BIT5);     // Lüfter ein
   }
   else
   {
   LoescheC(BIT5);   // Lüfter aus
   }
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion schaltet den Ozongenerator ein und aus.             */
void OzonEin_Aus(char cEinAus)
{
if (cEinAus == 0)
   {
   LoescheC(BIT6);   // Generator ein   
   }
   else
   {
   SetzeC(BIT6);     // Generator aus
   }
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die folgende Funktion schaltet die Standby-LED ein und aus.               */
/* 1: LED an, 0: LED aus                                                     */
void StandbyLEDEin_Aus(char cEinAus)
{
if (cEinAus == 0)
   {
   SetzeC(BIT4);     // LED aus
   }
   else
   {
   LoescheC(BIT4);   // LED ein
   }
}
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* Die Funktion schreibt ein Zeichen in das EEPROM                           */
void SchreibeInEEPROM(char cAdresse, char cWert)
{
  EECON1bits.EEPGD = 0;  /* WRITE step #1 */
  EECON1bits.CFGS = 0;   /* WRITE step #1B n.Doku S.87,bei18F1320 nicht nötig*/   
  EECON1bits.WREN = 1;   /* WRITE step #2 */
  EEADR = cAdresse;      /* WRITE step #3 */
  EEDATA = cWert;        /* WRITE step #4 */
  EECON2 = 0x55;         /* WRITE step #5 */
  EECON2 = 0xaa;         /* WRITE step #6 */
  EECON1bits.WR = 1;     /* WRITE step #7 */
  while (!PIR2bits.EEIF) /* WRITE step #8 */
    ;
  PIR2bits.EEIF = 0;     /* WRITE step #9 */

  /* clear the interrupt flag */
  INTCONbits.INT0IF = 0;
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Die Funktion liest ein Zeichen aus dem EEPROM                             */
char LeseAusEEPROM(char cAdresse)
{
char cWert;

  EECON1bits.EEPGD = 0;  /* READ step #1 */
  EECON1bits.CFGS = 0;   /* READ step #1B n.Doku S.87,bei 18F1320 nicht nötig*/
  EEADR =  cAdresse;     /* READ step #2 */
  EECON1bits.RD = 1;     /* READ step #3 */
  cWert = EEDATA;        /* READ step #4 */

return cWert;
}
/* ------------------------------------------------------------------------- */

//#define MAXANZ_KANAELE 1//512//90 // Mehr Kanäle führen bei 18F1320 zur Fehlfunktion


//#pragma udata buffer_scn
//static char buffer[MAXANZ_KANAELE];
//#pragma udata

// #define EEPROM_KANALZAHL 253   // 255 Byte - 3 Infobytes (Modus, Kanalzahl, Intervallzeit) 
/** D E C L A R A T I O N S **************************************************/ 
// #pragma code 



void main(void) 
{ 
unsigned char ucModus;    // 0: Idle , 1: Dworking, 2: IND-Sequenz, 3: IND-Kanal
unsigned char ucLetzterModus;
char *pcKanaele;
unsigned int uiKanalzahl;
unsigned int uiKanalzaehler;
unsigned int uiKanal;
unsigned char ucKanal;
unsigned int uiKanalSpeicherGrenze;

unsigned int i;
char cToggle;
unsigned char ucData;
char cAdresse;
char cZeitintervall;
char cAbschnitt;
unsigned int uiTimer;
unsigned int uiAbschnittTimer;

/* Variablen für Ozongenerator */
char cOzonstufe = 1;                 // Ozonstufe 1 bis 4
unsigned uPT100WertADC;              // ADC-Wert PT100 Temperaturmessung
float fMaxTemperaturLuefter = 25;    // Schwelle für Lüftereinschaltung (25 Grad)
float fMaxTemperaturAbschaltung = 30;// Schwelle für Gerätabschaltung (30 Grad)
char ucUebertemperatur = 0;          // 1, wenn Gerätetemperatur unzulässig hoch  
float fTemperatur;
float fPT100Widerstand;              // Widerstandswert PT100    
float fPT100SpannungsteilerR = 1000; // 1 kOhm Widerstand vor PT100  
int iStandByModus = 0;               // 1, wenn das Gerät ausgeschalet ist.  
int iFolienTasteBereitsGedrueckt=0;  // Merkt sich, ob die Taste losgelassen
                                     // wurde.
int iFolienTastenDruckIgnorieren =0; // Nach StandBy-Einschalten nicht die 
                                     // Ozon-Stufe erhöhen.



cToggle = 0;
cAbschnitt = 0;
ucModus = 0;         // Idle-Modus: Keine Ausgabe
   
cPORTB_Out = 0;
uiAbschnittTimer=0;

// Initialisierung des Port A:
// Belegung:
// Pin2 - AN0/RA0 - Eingang ADC für PT 100 Widerstand
// Pin3 - RA1     - Ausgang für LED1 Diagnose
// Pin4 - RA2     - Ausgang für LED2 Blaue Stufe 1
// Pin5 - RA3     - Ausgang für LED3 Grüne Stufe 2
// Pin6 - RA4     - Ausgang für LED4 Gelb Stufe 3
// Pin7 - RA5     - Ausgang für LED5 Pink Stufe 4  - ab Version 2.1  nicht mehr versendet.
  

 
//  LATA =  0x00;  
TRISA = 0xE1;                // Bit 1 bis Bit 4 sind Ausgänge (1=Input, 0=Output) 
                             // Bit0 ist ein Eingang (AD-Wandler PT100) 
                             // Bit5 ist ein Eingang (AD-Wandler Druckschalter) 

cPORTA_Out = 0x1E;           // Alle LEDs Aus, weil Leitungen auf High   
PORTA = 0x1E;

  ADCON0bits.CHS0=0;         // Nach Kapitel 19.1, Seite 225 
  ADCON0bits.CHS1=0;         // Vorinitialisierung AD-Wandler AN0 
  ADCON0bits.CHS2=0;         
  ADCON0bits.CHS3=0;  

  ADCON0bits.ADON=1;         // AD-Wandler einschalten.
  ADCON0bits.GO=0;           // Er läuft aber noch nicht
  

  ADCON1bits.PCFG0 = 0;      // PA0=AN0= analog, digital sonst 
  ADCON1bits.PCFG1 = 1;      // (siehe Doku Kapitel 19-2, Seite 224)  
  ADCON1bits.PCFG2 = 1; 
  ADCON1bits.PCFG3 = 1; 
  
  ADCON2bits.ADCS0 = 0;      // A/D-Clock FOSC/16 
  ADCON2bits.ADCS1 = 1; 
  ADCON2bits.ADCS2 = 0; 
  
  ADCON2bits.ACQT0 = 0;      // 8 TAD
  ADCON2bits.ACQT1 = 0; 
  ADCON2bits.ACQT2 = 1; 

  ADCON2bits.ADFM  = 1;      // Right justified 
  

// Initialisierung des Port B:
// Belegung:
// Pin25 - RB4    - Eingang für Taste UP
// Pin26 - RB5    - Eingang für Taste Down NEU Drucksimulation 
// Pin27 - RB6    - Eingang für Taste Druck-Simulation NEU D für Programmierung
// Pin28 - RB7    - Eingang für Taste Druck-Sensor NEU PSG für Programmierung

  TRISB = 0xFF;             // Keine Ausgänge

// Initialisierung des Port C:
// Belegung:
// Pin11 - RC0    - Eingang Drucksensor  
// Pin15 - RC4    - Eingang Standby-Taster  
// Pin16 - RC5    - Ausgang für Lüfter
// Pin17 - RC6    - Ausgang für Ozongenerator

  TRISC = 0x8F;             // Bit 4, 5 und Bit 6 sind Ausgänge (1=Input, 0=Output), sonst Eingänge 


// Timer-Initialisierung:
// configure timer0 - längster Zähltakt ist 51,2us (1/(5 MHz * 256))
OpenTimer0( TIMER_INT_OFF &
T0_SOURCE_INT &
T0_16BIT &
T0_PS_1_256 );
WriteTimer0( 0 ); // restart timer


/* Lampentest */

//while(1)
    {
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100); 

   LoescheA(2);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   SetzeA(2);


   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   LoescheA(4);
    
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100); 
   SetzeA(4);

   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   LoescheA(8);
   
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   SetzeA(8);

   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   LoescheA(16);
    
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   SetzeA(16);

   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   LuefterEin_Aus(1); // Lüfter an

   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   LuefterEin_Aus(0); // Lüfter aus

   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   OzonEin_Aus(1); // Generator an

   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   Delay10KTCYx(100);
   OzonEin_Aus(0); // Generator aus
   }

cAdresse = 0; 
ucData = LeseAusEEPROM(cAdresse);   // Letzte Onzonstufe lesen
cOzonstufe = ucData;
if (cOzonstufe > 4) cOzonstufe = 4;
if (cOzonstufe < 1) cOzonstufe = 1;

blinker (2,0);      // Zwei Mal blinken bevor es losgeht.



while(1) 
   {
   // Lüfter mit Hysterese steuern
   uPT100WertADC = LeseADWandler();      // Temperatur-Spannung einlesen  
   fPT100Widerstand = uPT100WertADC * (fPT100SpannungsteilerR /(1024 - uPT100WertADC));
   fTemperatur = (fPT100Widerstand - 100) * 2.597 * 0.716; // 0.716 = Schaltungskompensation
   if (fTemperatur > fMaxTemperaturLuefter)
       {
       LuefterEin_Aus(1); // Lüfter an
       }
   if (fTemperatur < (fMaxTemperaturLuefter -1))
       {
       LuefterEin_Aus(0); // Lüfter aus
       }

   if (fTemperatur > fMaxTemperaturAbschaltung)
       {
       ucUebertemperatur = 1;          // 1, wenn Gerätetemperatur unzulässig hoch  
       }
   if (fTemperatur < (fMaxTemperaturAbschaltung - 5))
       {
       ucUebertemperatur = 0;          // 0, wenn Gerätetemperatur o.k. 
       }

   uiTimer = ReadTimer0();                    // read timer
   if(LeseTastePortB_Bit4_Taste_Up() == 0)    // Kein Tastendruck
      {
      // Sekundentakt für nachfolgende Ozon-Timer bereitstellen. 
      if (uiTimer>19531)           // 1 Sekunden verstrichen (genau: 19531,25)
          {
          WriteTimer0( 0 ); // restart timer
          LED_Anzeige_Ozonstufe(cOzonstufe, 0);    // LEDs-löschen
          } 
      }
      else
      { 
      iFolienTasteBereitsGedrueckt = 1;                       // Taste gedrückt
      // StandBy-Modus toggeln ?: 
      if (uiTimer>19531*3)           // 3 Sekunden verstrichen (genau: 3 * 19531,25)
          {
   //       iFolienTasteBereitsGedrueckt = 0; // Tastedruck nicht zählen, da StandBy
          
          if (iStandByModus  == 1)
              {
              iStandByModus  = 0;
              
              iFolienTastenDruckIgnorieren =1; // Nach StandBy-Ende nicht die 
                                               // Ozon-Stufe mit dem Druck
                                               // erhöhen.
              }
              else
              iStandByModus  = 1;
          
          WriteTimer0( 0 );         // restart timer
          } 
      }
   
   // Der Folientaster steuert die Ozon-Stufe und Standby. Die Entkopplung 
   // erfordert ein paar Bedingungen.
   // Ozon-Stufenänderung nur, wenn ... 
   if ((iStandByModus == 0) &&                 // ... überhaupt eingeschaltet  
       (iFolienTasteBereitsGedrueckt == 1) &&  // ... und beim Loslassen der 
       (LeseTastePortB_Bit4_Taste_Up() == 0))  // Taste (kein Tastendruck mehr)
       {
       iFolienTasteBereitsGedrueckt = 0;       // Taste wurde losgelassen
       if (iFolienTastenDruckIgnorieren == 0)  // Und nach StandBy-Änderung nicht 
                                               // reagieren.
           {
           cOzonstufe ++;
           if (cOzonstufe > 4) cOzonstufe = 1; // Stufen im Kreis verlaufen lassen
           cAdresse = 0;
           ucData =  cOzonstufe; // Ozonstufe speichern
           SchreibeInEEPROM(cAdresse,ucData);
           LED_Anzeige_Ozonstufe(cOzonstufe, 1);
           WriteTimer0( 0 );     // restart timer
           }
       else
           {
           iFolienTastenDruckIgnorieren =0;     // Nur einmal nach Standby 
                                                // ignorieren.
           }
        }
    
    
   if(iStandByModus  == 1)
       {
       StandbyLEDEin_Aus(0);                   // Keine Standby-LED zeigen
       LED_Anzeige_Ozonstufe(cOzonstufe, 0);   // Keine LEDs zeigen.
       OzonEin_Aus(0);                         // Generator aus
       }
   
   // Gab es eine Ozon-Anforderung via ADC-Drucksensor oder Taster?
   if(LeseTastePortB_Bit5_CheckeDruckADCWert(uDruckschalterWertADC) && 
      (ucUebertemperatur == 0) &&
      (iStandByModus  == 0))
   //   if(LeseTastePortB_Bit5_PortC_Bit0_Druck() && (ucUebertemperatur == 0))
       {
       LED_Anzeige_Ozonstufe(cOzonstufe, 1);    // LEDs zeigen.
       StandbyLEDEin_Aus(0);                    // Standby-LED ausschalten

       switch (cOzonstufe)
           {
           case 1: // Stufe 1 (25 %)
                   if (uiTimer< (19531 * 0.25))  
                      {
                      OzonEin_Aus(1); // Generator ein
                      }
                      else
                      {
                      OzonEin_Aus(0); // Generator aus
                      }
           break;
           
           case 2: // Stufe 1 (50 %)
                   if (uiTimer< (19531 * 0.50))  
                      {
                      OzonEin_Aus(1); // Generator ein
                      }
                      else
                      {
                      OzonEin_Aus(0); // Generator aus
                      }
           break;

           case 3: // Stufe 1 (75 %)
                   if (uiTimer< (19531 * 0.75))  
                      {
                      OzonEin_Aus(1); // Generator ein
                      }
                      else
                      {
                      OzonEin_Aus(0); // Generator aus
                      }
           break;

           case 4: // Stufe 1 (100 %)                       
                      OzonEin_Aus(1); // Generator ein                   
           break;

           } // ... switch() ...

       } // ... if(LeseTastePortB_Bit5_PortC_Bit0_Druck()) ...
       else
       {
       // Keine Druckanforderung
       LED_Anzeige_Ozonstufe(cOzonstufe, 0);    // LEDs-löschen
       OzonEin_Aus(0);                          // Generator aus
       if(iStandByModus  == 0)
           StandbyLEDEin_Aus(1);                // Standby-LED einschalten
       }
  
   if (ucUebertemperatur == 1)
       {
       if (uiTimer>9765)        // 0,5 Sekunden verstrichen (genau: 9765,5)
           {
           LED_Anzeige_Ozonstufe(1, 1); // Die Stufen-LEDs als Fehlermeldung blinken lassen.
           LED_Anzeige_Ozonstufe(2, 1);
           LED_Anzeige_Ozonstufe(3, 1);
           LED_Anzeige_Ozonstufe(4, 1);
           }
       if (uiTimer>19531)        // 1 Sekunden verstrichen (genau: 19531,25)
           {
           WriteTimer0( 0 ); // restart timer
           LED_Anzeige_Ozonstufe(cOzonstufe, 0);    // LEDs-löschen
           }
       }

   } // ... while(1) ... 
   

}//end main 


