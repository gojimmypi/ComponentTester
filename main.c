/* ************************************************************************
 *
 *   main part
 *
 *   (c) 2012-2015 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz K�bbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define MAIN_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "HD44780.h"          /* HD44780 LCD module */
#include "functions.h"        /* external functions */


/*
 *  local variables
 */

/* programm control */
uint8_t        RunsPassed;              /* counter for successful measurements */
uint8_t        RunsMissed;              /* counter for failed/missed measurements */



/* ************************************************************************
 *   output found components
 * ************************************************************************ */


/*
 *  show pinout for semiconductor
 *
 *  required:
 *  - character for pin A
 *  - character for pin B
 *  - character for pin C
 */

void Show_SemiPinout(uint8_t A, uint8_t B, uint8_t C)
{
  uint8_t           i, j;     /* counter */
  unsigned char     Pin[3];   /* component pins */
  unsigned char     ID[3];    /* component pin IDs */

  /* copy probe pin numbers */
  Pin[0] = Semi.A;
  Pin[1] = Semi.B;
  Pin[2] = Semi.C;

  /* copy pin characters/IDs */
  ID[0] = A;
  ID[1] = B;
  ID[2] = C;

  /* display: 123 */
  for (i = 0; i <= 2; i++)
  {
    LCD_ProbeNumber(i);
  }

  /* display: = */
  LCD_Char('=');

  /* display pin IDs */
  for (i = 0; i <= 2; i++)         /* loop through probe pins */
  {
    for (j = 0; j <= 2; j++)       /* loop through component pins */
    {
      if (i == Pin[j])        /* probe pin matches */
      {
        LCD_Char(ID[j]);           /* show ID */
      }
    }
  }
}



/*
 *  show failed test
 */

void Show_Fail(void)
{
  /* display info */
  LCD_EEString(Failed1_str);            /* display: No component */
  LCD_NextLine_EEString(Failed2_str);   /* display: found!*/  

  /* display numbers of diodes found */
  if (Check.Diodes > 0)                 /* diodes found */
  {
    LCD_Space();                        /* display space */
    LCD_Char(Check.Diodes + '0');       /* display number of diodes found */
    LCD_Space();                        /* display space */
    LCD_EEString(Diode_AC_str);         /* display: -|>|- */    
  }

  RunsMissed++;               /* increase counter */
  RunsPassed = 0;             /* reset counter */
}



/*
 *  show error
 */

void Show_Error()
{
  if (Check.Type == TYPE_DISCHARGE)     /* discharge failed */
  {
    LCD_EEString(DischargeFailed_str);  /* display: Battery? */

    /* display probe number and remaining voltage */
    LCD_NextLine();
    LCD_ProbeNumber(Check.Probe);
    LCD_Char(':');
    LCD_Space();
    DisplayValue(Check.U, -3, 'V');
  }
}



/*
 *  show single (first) resistor
 *
 *  requires:
 *  - ID1 pin ID character
 *  - ID2 pin ID character
 */

void Show_SingleResistor(uint8_t ID1, uint8_t ID2)
{
  Resistor_Type     *Resistor;     /* pointer to resistor */

  Resistor = &Resistors[0];        /* pointer to first resistor */

  /* show pinout */
  LCD_Char(ID1);
  LCD_EEString(Resistor_str);
  LCD_Char(ID2); 

  /* show resistance value */
  LCD_Space();
  DisplayValue(Resistor->Value, Resistor->Scale, LCD_CHAR_OMEGA);
}



/*
 *  show resistor(s)
 */

void Show_Resistor(void)
{
  Resistor_Type     *R1;           /* pointer to resistor #1 */
  Resistor_Type     *R2;           /* pointer to resistor #2 */
  uint8_t           Pin;           /* ID of common pin */

  R1 = &Resistors[0];              /* pointer to first resistor */

  if (Check.Resistors == 1)        /* single resistor */
  {
    R2 = NULL;                     /* disable second resistor */
    Pin = R1->A;                   /* make B the first pin */
  }
  else                             /* multiple resistors */
  {
    R2 = R1;
    R2++;                          /* pointer to second resistor */

    if (Check.Resistors == 3)      /* three resistors */
    {
      Resistor_Type     *Rmax;     /* pointer to largest resistor */    

      /*
       *  3 resistors mean 2 single resistors and both resistors in series.
       *  So we have to single out that series resistor by finding the
       *  largest resistor.
       */

      Rmax = R1;                   /* starting point */
      for (Pin = 1; Pin <= 2; Pin++)
      {
        if (CmpValue(R2->Value, R2->Scale, Rmax->Value, Rmax->Scale) == 1)
        {
          Rmax = R2;          /* update largest one */
        }

        R2++;                 /* next one */
      }

      /* get the two smaller resistors */
      if (R1 == Rmax) R1++;
      R2 = R1;
      R2++;
      if (R2 == Rmax) R2++;
    }

    /* find common pin of both resistors */
    if ((R1->A == R2->A) || (R1->A == R2->B)) Pin = R1->A;
    else Pin = R1->B;
  }


  /*
   *  display the pins
   */

  /* first resistor */
  if (R1->A != Pin) LCD_ProbeNumber(R1->A);
  else LCD_ProbeNumber(R1->B);
  LCD_EEString(Resistor_str);
  LCD_ProbeNumber(Pin);

  if (R2)           /* second resistor */
  {
    LCD_EEString(Resistor_str);
    if (R2->A != Pin) LCD_ProbeNumber(R2->A);
    else LCD_ProbeNumber(R2->B);
  }


  /*
   *  display the values
   */

  /* first resistor */
  LCD_NextLine();
  DisplayValue(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

  if (R2)                /* second resistor */
  {
    LCD_Space();
    DisplayValue(R2->Value, R2->Scale, LCD_CHAR_OMEGA);
  }
  #ifdef SW_INDUCTOR
  else                   /* single resistor */
  {
    /* get inductance and display if relevant */
    if (MeasureInductor(R1) == 1)
    {
      LCD_Space();
      DisplayValue(Inductor.Value, Inductor.Scale, 'H');
    }
  }
  #endif
}


/*
 *  show capacitor
 */

void Show_Capacitor(void)
{
  Capacitor_Type    *MaxCap;       /* pointer to largest cap */
  Capacitor_Type    *Cap;          /* pointer to cap */
  #ifdef SW_ESR
  uint16_t          ESR;           /* ESR (in 0.01 Ohms) */
  #endif
  uint8_t           Counter;       /* loop counter */

  /* find largest cap */
  MaxCap = &Caps[0];               /* pointer to first cap */
  Cap = MaxCap;

  for (Counter = 1; Counter <= 2; Counter++) 
  {
    Cap++;                         /* next cap */

    if (CmpValue(Cap->Value, Cap->Scale, MaxCap->Value, MaxCap->Scale) == 1)
    {
      MaxCap = Cap;
    }
  }

  /* display pinout */
  LCD_ProbeNumber(MaxCap->A);      /* display pin #1 */
  LCD_EEString(Cap_str);           /* display capacitor symbol */
  LCD_ProbeNumber(MaxCap->B);      /* display pin #2 */

  /* show capacitance */
  LCD_NextLine();                  /* move to next line */
  DisplayValue(MaxCap->Value, MaxCap->Scale, 'F');

  #ifdef SW_ESR
  /* show ESR */
  ESR = MeasureESR(MaxCap);        /* measure ESR */
  if (ESR > 0)                     /* if successfull */
  {
//    LCD_NextLine_EEString_Space(ESR_str);    /* display: ESR */
    LCD_Space();
    DisplayValue(ESR, -2, LCD_CHAR_OMEGA);   /* display ESR */
  }
  #endif
}



/*
 *  display capacitance of a diode
 *
 *  requires:
 *  - pointer to diode structure
 */

void Show_Diode_Cap(Diode_Type *Diode)
{
  /* sanity check */
  if (Diode == NULL) return;

  /* get capacitance (opposite of flow direction) */
  MeasureCap(Diode->C, Diode->A, 0);

  /* and show capacitance */
  DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');
}



/*
 *  show diode
 */

void Show_Diode(void)
{
  Diode_Type        *D1;           /* pointer to diode #1 */
  Diode_Type        *D2 = NULL;    /* pointer to diode #2 */
  uint8_t           CapFlag = 1;   /* flag for capacitance output */
  uint8_t           A = 5;         /* ID of common anode */
  uint8_t           C = 5;         /* ID of common cothode */
  uint8_t           R_Pin1 = 5;    /* B_E resistor's pin #1 */
  uint8_t           R_Pin2 = 5;    /* B_E resistor's pin #2 */
  uint16_t          I_leak;        /* leakage current */

  D1 = &Diodes[0];                 /* pointer to first diode */


  /*
   *  figure out which diodes to display
   */

  if (Check.Diodes == 1)           /* single diode */
  {
    C = D1->C;                     /* make anode first pin */
  }
  else if (Check.Diodes == 2)      /* two diodes */
  {
    D2 = D1;
    D2++;                          /* pointer to second diode */

    if (D1->A == D2->A)            /* common anode */
    {
      A = D1->A;                   /* save common anode */

      /* possible PNP BJT with low value B-E resistor and flyback diode */
      R_Pin1 = D1->C;
      R_Pin2 = D2->C;
    }
    else if (D1->C == D2->C)       /* common cathode */
    {
      C = D1->C;                   /* save common cathode */

      /* possible NPN BJT with low value B-E resistor and flyback diode */
      R_Pin1 = D1->A;
      R_Pin2 = D2->A;
    }
    else if ((D1->A == D2->C) && (D1->C == D2->A))   /* anti-parallel */
    {
      A = D1->A;                   /* anode and cathode */
      C = A;                       /* are the same */
      CapFlag = 0;                 /* skip capacitance */
    }
  }
  else if (Check.Diodes == 3)      /* three diodes */
  {
    uint8_t         n;
    uint8_t         m;

    /*
     *  Two diodes in series are additionally detected as third big diode:
     *  - Check for any possible way of 2 diodes be connected in series.
     *  - Only once the cathode of diode #1 matches the anode of diode #2.
     */

    for (n = 0; n <= 2; n++)       /* loop for first diode */
    {
      D1 = &Diodes[n];             /* get pointer of first diode */

      for (m = 0; m <= 2; m++)     /* loop for second diode */
      {
        D2 = &Diodes[m];           /* get pointer of second diode */

        if (n != m)                /* don't check same diode :-) */
        {
          if (D1->C == D2->A)      /* got match */
          {
            n = 5;                 /* end loops */
            m = 5;
          }
        }
      }
    }

    if (n < 5) D2 = NULL;          /* no match found */
    C = D1->C;                     /* cathode of first diode */
    A = 3;                         /* in series mode */
  }
  else                             /* to much diodes */
  {
    D1 = NULL;                     /* don't display any diode */
    Show_Fail();                   /* and tell user */
    return;
  }


  /*
   *  display pins 
   */

  /* first Diode */
  if (A < 3) LCD_ProbeNumber(D1->C);       /* common anode */
  else LCD_ProbeNumber(D1->A);             /* common cathode */

  if (A < 3) LCD_EEString(Diode_CA_str);   /* common anode */
  else LCD_EEString(Diode_AC_str);         /* common cathode */

  if (A < 3) LCD_ProbeNumber(A);           /* common anode */
  else LCD_ProbeNumber(C);                 /* common cathode */

  if (D2)           /* second diode */
  {
    if (A <= 3) LCD_EEString(Diode_AC_str);  /* common anode or in series */
    else LCD_EEString(Diode_CA_str);         /* common cathode */

    if (A == C) LCD_ProbeNumber(D2->A);           /* anti parallel */
    else if (A <= 3) LCD_ProbeNumber(D2->C);      /* common anode or in series */
    else LCD_ProbeNumber(D2->A);                  /* common cathode */
  }

  /* check for B-E resistor for possible BJT */
  if (R_Pin1 < 5)                  /* possible BJT */
  {
    if (CheckSingleResistor(R_Pin1, R_Pin2) == 1) /* found B-E resistor */
    {
      /* show: PNP/NPN? */
      LCD_Space();
      if (A < 3) LCD_EEString(PNP_str);
      else LCD_EEString(NPN_str);
      LCD_Char('?');

      LCD_NextLine();                     /* go to line #2 */
      R_Pin1 += '1';                      /* convert to character */
      R_Pin2 += '1';
      Show_SingleResistor(R_Pin1, R_Pin2);   /* show resistor */
      CapFlag = 0;                        /* skip capacitance */
    }
  }


  /*
   *  display:
   *  - Uf (forward voltage)
   *  - reverse leakage current (for single diodes)
   *  - capacitance (not for anti-parallel diodes)
   */

  /* display Uf */
  LCD_NextLine_EEString_Space(Vf_str);  /* display: Vf */

  /* first diode */
  DisplayValue(D1->V_f, -3, 'V');

  LCD_Space();

  /* display low current Uf and reverse leakage current for a single diode */
  if (D2 == NULL)                       /* single diode */
  {
    /* display low current Uf if it's quite low (Ge/Schottky diode) */
    if (D1->V_f2 < 250)
    {
      LCD_Char('(');
      DisplayValue(D1->V_f2, 0, 0);
      LCD_Char(')');
    }

    /* reverse leakage current */
    UpdateProbes(D1->C, D1->A, 0);      /* reverse diode */
    I_leak = GetLeakageCurrent();       /* get current (in �A) */
    if (I_leak > 0)                     /* show if not zero */
    {
      LCD_NextLine_EEString_Space(I_R_str);  /* display: I_R */
      DisplayValue(I_leak, -6, 'A');    /* display current */
    }
  }
  else                                  /* two diodes */
  {
    /* show Uf of second diode */
    DisplayValue(D2->V_f, -3, 'V');
  }

  /* display capacitance */
  if (CapFlag == 1)                     /* if requested */ 
  {
    LCD_NextLine_EEString_Space(DiodeCap_str);  /* display: C */
    Show_Diode_Cap(D1);                 /* first diode */
    LCD_Space();
    Show_Diode_Cap(D2);                 /* second diode (optional) */
  }
}



/*
 *  show intrinsic/freewheeling diode of transistor
 */

void Show_FlybackDiode(void)
{
  LCD_Space();                /* display space */

  /* first pin */
  if (Check.Found == COMP_FET)     /* FET */
  {
    LCD_Char('D');                 /* drain */
  }
  else                             /* BJT/IGBT */
  {
    LCD_Char('C');                 /* collector */
  }

  /* diode */
  if (Check.Type & TYPE_N_CHANNEL)      /* n-channel/NPN */
  {
    /*
     *  anode pointing to source/emitter
     *  cathode pointing to drain/collector
     */

    LCD_Char(LCD_CHAR_DIODE_CA);        /* |<| */
  }
  else                                  /* p-channel/PNP */
  {
    /*
     *  anode pointing to drain/collector
     *  cathode pointing to source/emitter
     */

    LCD_Char(LCD_CHAR_DIODE_AC);        /* |>| */
  }

  /* second pin */
  if (Check.Found == COMP_FET)     /* FET */
  {
    LCD_Char('S');                 /* source */
  }
  else                             /* BJT/IGBT */
  {
    LCD_Char('E');                 /* emitter */
  }
}



/*
 *  show BJT
 */

void Show_BJT(void)
{
  Diode_Type        *Diode;        /* pointer to diode */
  unsigned char     *String;       /* display string pointer */
  uint8_t           Counter;       /* counter */
  uint8_t           A_Pin;         /* pin acting as anode */
  uint8_t           C_Pin;         /* pin acting as cathode */
  uint16_t          V_BE;          /* V_BE */
  int16_t           Slope;         /* slope of forward voltage */

  /*
   *  Mapping for Semi structure:
   *  A   - Base pin
   *  B   - Collector pin
   *  C   - Emitter pin
   *  U_1 - U_BE (mV)
   *  I_1 - I_CE0 (�A)
   *  F_1 - hFE
   */

  /* preset stuff based on BJT type */
  if (Check.Type & TYPE_NPN)       /* NPN */
  {
    String = (unsigned char *)NPN_str;       /* "NPN" */

    /* direction of B-E diode: B -> E */
    A_Pin = Semi.A;      /* anode at base */
    C_Pin = Semi.C;      /* cathode at emitter */
  }
  else                             /* PNP */
  {
    String = (unsigned char *)PNP_str;       /* "PNP" */

    /* direction of B-E diode: E -> B */
    A_Pin = Semi.C;      /* anode at emitter */
    C_Pin = Semi.A;      /* cathode at base */
  }

  /* display type */
  LCD_EEString_Space(BJT_str);     /* display: BJT */
  LCD_EEString(String);            /* display: NPN / PNP */

  /* parasitic BJT (freewheeling diode on same substrate) */
  if (Check.Type & TYPE_PARASITIC)
  {
    LCD_Char('+');
  }

  LCD_NextLine();                  /* move to line #2 */

  /* display pinout */
  Show_SemiPinout('B', 'C', 'E');

  /* optional freewheeling diode */
  if (Check.Diodes > 2)       /* transistor is a set of two diodes :-) */
  {
    Show_FlybackDiode();           /* show diode */
  }

  LCD_NextLine();                  /* next line */

  /* display either optional B-E resistor or hFE & V_BE */
  if (CheckSingleResistor(C_Pin, A_Pin) == 1)     /* found B-E resistor */
  {
    Show_SingleResistor('B', 'E');
  }
  else                                            /* no B-E resistor found */
  {
    /* hFE and V_BE */

    /* display hFE */
    LCD_EEString_Space(hFE_str);        /* display: h_FE */
    DisplayValue(Semi.F_1, 0, 0);

    /* display V_BE (taken from diode forward voltage) */
    Diode = &Diodes[0];                 /* get pointer of first diode */  
    Counter = 0;

    while (Counter < Check.Diodes)      /* check all diodes */
    {
      /* if the diode matches the transistor's B-E diode */
      if ((Diode->A == A_Pin) && (Diode->C == C_Pin))
      {
        LCD_NextLine_EEString_Space(V_BE_str);   /* display: V_BE */

        /*
         *  Vf is quite linear for a logarithmicly scaled I_b.
         *  So we may interpolate the Vf values of low and high test current
         *  measurements for a virtual test current. Low test current is 10�A
         *  and high test current is 7mA. That's a logarithmic scale of
         *  3 decades.
         */

        /* calculate slope for one decade */
        Slope = Diode->V_f - Diode->V_f2;
        Slope /= 3;

        /* select V_BE based on hFE */
        if (Semi.F_1 < 100)             /* low hFE */
        {
          /*
           *  BJTs with low hFE are power transistors and need a large I_b
           *  to drive the load. So we simply take Vf of the high test current
           *  measurement (7mA). 
           */

          V_BE = Diode->V_f;
        }
        else if (Semi.F_1 < 250)        /* mid-range hFE */
        {
          /*
           *  BJTs with a mid-range hFE are signal transistors and need
           *  a small I_b to drive the load. So we interpolate Vf for
           *  a virtual test current of about 1mA.
           */

          V_BE = Diode->V_f - Slope;
        }
        else                            /* high hFE */
        {
          /*
           *  BJTs with a high hFE are small signal transistors and need
           *  only a very small I_b to drive the load. So we interpolate Vf
           *  for a virtual test current of about 0.1mA.
           */

          V_BE = Diode->V_f2 + Slope;
        }

        DisplayValue(V_BE, -3, 'V');

        Counter = 10;              /* end loop */
      }
      else                         /* diode doesn't match */
      {
        Counter++;                      /* increase counter */
        Diode++;                        /* next one */
      }
    }
  }

  /* I_CEO: collector emitter cutoff current (leakage) */
  if (Semi.I_1 > 0)                     /* show if not zero */
  {
    LCD_NextLine_EEString_Space(I_CEO_str);  /* display: I_CE0 */
    DisplayValue(Semi.I_1, -6, 'A');         /* display current */
  }
}



/*
 *  show MOSFET/IGBT extras
 *  - diode
 *  - V_th
 *  - Cgs
 */

void Show_FET_Extras(void)
{
  /* show instrinsic/freewheeling diode */
  if (Check.Diodes > 0)            /* diode found */
  {
    Show_FlybackDiode();           /* show diode */
  }

  /* skip remaining stuff for depletion-mode FETs/IGBTs */
  if (Check.Type & TYPE_DEPLETION) return;

  /* gate threshold voltage */
  if (Semi.U_2 != 0)
  {
    LCD_NextLine_EEString_Space(Vth_str);    /* display: Vth */
    DisplaySignedValue(Semi.U_2, -3, 'V');   /* display V_th in mV */
  }

  /* display gate-source capacitance */
  LCD_NextLine_EEString_Space(GateCap_str);  /* display: Cgs */
  MeasureCap(Semi.A, Semi.C, 0);             /* measure capacitance */
  /* display value and unit */
  DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');
}



/*
 *  show FET/IGBT channel type
 */

void Show_FET_Channel(void)
{
  LCD_Space();                     /* display space */

  /* channel type */
  if (Check.Type & TYPE_N_CHANNEL)   /* n-channel */
  {
    LCD_Char('N');
  }
  else                               /* p-channel */
  {
    LCD_Char('P');
  }

  LCD_EEString(Channel_str);         /* display: -ch */
}



/*
 *  show FET/IGBT mode
 */

void Show_FET_Mode(void)
{
  LCD_Space();

  if (Check.Type & TYPE_ENHANCEMENT)    /* enhancement mode */
  {
    LCD_EEString(Enhancement_str);
  }
  else                                  /* depletion mode */
  {
    LCD_EEString(Depletion_str);
  }
}



/*
 *  show FET
 */

void Show_FET(void)
{
  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Drain pin
   *  C   - Source pin
   *  U_2 - V_th (mV)
   */

  /* display type */
  if (Check.Type & TYPE_MOSFET)    /* MOSFET */
  {
    LCD_EEString(MOS_str);           /* display: MOS */
  }
  else                             /* JFET */
  {
    LCD_Char('J');                   /* display: J */
  }
  LCD_EEString(FET_str);           /* display: FET */

  /* display channel type */
  Show_FET_Channel();
      
  /* display mode for MOSFETs*/
  if (Check.Type & TYPE_MOSFET) Show_FET_Mode();

  /* pinout */
  LCD_NextLine();                       /* move to line #2 */

  if (Check.Type & TYPE_SYMMETRICAL)    /* symmetrical Drain and Source */
  {
    /* we can't distinguish D and S */
    Show_SemiPinout('G', 'x', 'x');     /* show pinout */
  }
  else                                  /* unsymmetrical Drain and Source */
  {
    Show_SemiPinout('G', 'D', 'S');     /* show pinout */
  }

  /* show diode, V_th and Cgs for MOSFETs */
  if (Check.Type & TYPE_MOSFET) Show_FET_Extras();
}



/*
 *  show IGBT  
 */

void Show_IGBT(void)
{
  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Collector pin
   *  C   - Emitter pin
   *  U_2 - V_th (mV)
   */

  LCD_EEString(IGBT_str);          /* display: IGBT */
  Show_FET_Channel();              /* display channel type */
  Show_FET_Mode();                 /* display mode */

  LCD_NextLine();                  /* move to line #2 */
  Show_SemiPinout('G', 'C', 'E');  /* show pinout */
  Show_FET_Extras();               /* show diode, V_th and C_GE */
}



/*
 *  show special components like Thyristor and Triac
 */

void Show_Special(void)
{
  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Anode/MT2 pin
   *  C   - Cathode/MT1 pin
   *  U_1 - V_GT (mV)
   */

  /* display component type any pinout */
  if (Check.Found == COMP_THYRISTOR)    /* SCR */
  {
    LCD_EEString(Thyristor_str);        /* display: thyristor */
    LCD_NextLine();                     /* move to line #2 */
    Show_SemiPinout('G', 'A', 'C');     /* display pinout */
  }
  else                                  /* Triac */
  {
    LCD_EEString(Triac_str);            /* display: triac */
    LCD_NextLine();                     /* move to line #2 */
    Show_SemiPinout('G', '2', '1');     /* display pinout */
  }

  /* show V_GT (gate trigger voltage) */
  if (Semi.U_1 > 0)                /* show if not zero */
  {
    LCD_NextLine_EEString_Space(V_GT_str);   /* display: V_GT */
    DisplayValue(Semi.U_1, -3, 'V');         /* display V_GT in mV */
  }
}



/* ************************************************************************
 *   the one and only main()
 * ************************************************************************ */


/*
 *  main function
 */

int main(void)
{
  uint16_t          U_Bat;         /* voltage of power supply */
  uint8_t           Test;          /* test value */

  /*
   *  init
   */

  /* switch on power to keep me alive */
  CONTROL_DDR = (1 << POWER_CTRL);      /* set pin as output */
  CONTROL_PORT = (1 << POWER_CTRL);     /* set pin to drive power management transistor */

  /* setup MCU */
  MCUCR = (1 << PUD);                        /* disable pull-up resistors globally */
  ADCSRA = (1 << ADEN) | ADC_CLOCK_DIV;      /* enable ADC and set clock divider */

  #ifdef HW_RELAY
  /* init relay (safe mode) */
                                      /* ADC_PORT should be 0 */
  ADC_DDR = (1 << TP_REF);            /* short circuit probes */
  #endif

  /* catch watchdog */  
  Test = (MCUSR & (1 << WDRF));         /* save watchdog flag */
  MCUSR &= ~(1 << WDRF);                /* reset watchdog flag */
  wdt_disable();                        /* disable watchdog */

  /* init LCD module */
  LCD_BusSetup();                       /* setup bus */
  LCD_Init();                           /* initialize LCD */
  LCD_NextLine_Mode(MODE_NONE);         /* reset line mode */


  /*
   *  watchdog was triggered (timeout 2s)
   *  - This is after the MCU done a reset driven by the watchdog.
   *  - Does only work if the capacitor at the base of the power management
   *    transistor is large enough to survive a MCU reset. Otherwise the
   *    tester simply loses power.
   */

  if (Test)
  {
    LCD_Clear();                        /* display was initialized before */
    LCD_EEString(Timeout_str);          /* display: timeout */
    LCD_NextLine_EEString(Error_str);   /* display: error */
    MilliSleep(2000);                   /* give user some time to read */
    CONTROL_PORT = 0;                   /* power off myself */
    return 0;                           /* exit program */
  }


  /*
   *  operation mode selection
   */

  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* default: power save */
  UI.TesterMode = MODE_CONTINOUS;            /* set default mode: continous */
  Test = 0;                                  /* key press */

  /* catch long key press */
  if (!(CONTROL_PIN & (1 << TEST_BUTTON)))   /* if test button is pressed */
  {
    RunsMissed = 0;

    while (Test == 0)
    {
      MilliSleep(20);
      if (!(CONTROL_PIN & (1 << TEST_BUTTON)))    /* if button is still pressed */
      {
        RunsMissed++;
        if (RunsMissed > 100) Test = 3;      /* >2000ms */
      }
      else                                        /* button released */
      {
        Test = 1;                            /* <300ms */
        if (RunsMissed > 15) Test = 2;       /* >300ms */
      }
    }
  }

  /* key press >300ms sets autohold mode */
  if (Test > 1) UI.TesterMode = MODE_AUTOHOLD;


  /*
   *  load saved offsets and values
   */

  if (Test == 3)              /* key press >2s resets to defaults */
  {
    SetAdjustDefaults();           /* set default values */
  }
  else                        /* normal mode */
  {
    LoadAdjust();                  /* load adjustment values */
  }

  /* set extra stuff */
  #ifdef SW_CONTRAST
    LCD_Contrast(NV.Contrast);          /* set LCD contrast */
  #endif


  /*
   *  welcome user
   */

  LCD_EEString(Tester_str);             /* display: Component Tester */
  LCD_NextLine_EEString_Space(Edition_str);   /* display firmware edition */
  LCD_EEString(Version_str);            /* display firmware version */
  MilliSleep(1500);                     /* let the user read the display */


  /*
   *  init variables
   */

  /* cycling */
  RunsMissed = 0;
  RunsPassed = 0;

  /* default offsets and values */
  Config.Samples = ADC_SAMPLES;         /* number of ADC samples */
  Config.AutoScale = 1;                 /* enable ADC auto scaling */
  Config.RefFlag = 1;                   /* no ADC reference set yet */
  Config.Vcc = UREF_VCC;                /* voltage of Vcc */
  wdt_enable(WDTO_2S);		        /* enable watchdog (timeout 2s) */


  /*
   *  main processing cycle
   */

start:

  /* reset variabels */
  Check.Found = COMP_NONE;
  Check.Type = 0;
  Check.Done = 0;
  Check.Diodes = 0;
  Check.Resistors = 0;
  Semi.U_1 = 0;
  Semi.I_1 = 0;
  Semi.F_1 = 0;

  /* reset hardware */
  ADC_DDR = 0;                     /* set all pins of ADC port as input */
                                   /* also remove short circuit by relay */
  LCD_NextLine_Mode(MODE_KEEP);    /* line mode: keep first line */
  LCD_Clear();                     /* clear LCD */


  /*
   *  voltage reference
   */

  #ifdef HW_REF25
  /* external 2.5V reference */
  Config.Samples = 200;            /* do a lot of samples for high accuracy */
  U_Bat = ReadU(TP_REF);           /* read voltage of reference (mV) */
  Config.Samples = ADC_SAMPLES;    /* set samples back to default */

  if ((U_Bat > 2250) && (U_Bat < 2750))   /* check for valid reference */
  {
    uint32_t        Temp;

    /* adjust Vcc (assuming 2.495V typically) */
    Temp = ((uint32_t)Config.Vcc * UREF_25) / U_Bat;
    Config.Vcc = (uint16_t)Temp;
  }
  #endif

  /* internal bandgap reference */
  Config.Bandgap = ReadU(0x0e);         /* dummy read for bandgap stabilization */
  Config.Samples = 200;                 /* do a lot of samples for high accuracy */
  Config.Bandgap = ReadU(0x0e);         /* get voltage of bandgap reference (mV) */
  Config.Samples = ADC_SAMPLES;         /* set samples back to default */
  Config.Bandgap += NV.RefOffset;       /* add voltage offset */ 


  /*
   *  battery check
   */

  /*
   *  ADC pin is connected to a voltage divider Rh = 10k and Rl = 3k3.
   *  Ul = (Uin / (Rh + Rl)) * Rl  ->  Uin = (Ul * (Rh + Rl)) / Rl
   *  Uin = (Ul * (10k + 3k3)) / 3k3 = 4 * Ul  
   */

  /* get current voltage */
  U_Bat = ReadU(TP_BAT);                /* read voltage (mV) */
  U_Bat *= 4;                           /* calculate U_bat (mV) */
  U_Bat += BAT_OFFSET;                  /* add offset for voltage drop */

  /* display battery voltage */
  LCD_EEString_Space(Battery_str);      /* display: Bat. */
  DisplayValue(U_Bat / 10, -2, 'V');    /* display battery voltage */
  LCD_Space();

  /* check limits */
  if (U_Bat < BAT_POOR)                 /* low level reached */
  {
    LCD_EEString(Low_str);              /* display: low */
    MilliSleep(2000);                   /* let user read info */
    goto power_off;                     /* power off */
  }
  else if (U_Bat < BAT_POOR + 1000)     /* warning level reached */
  {
    LCD_EEString(Weak_str);             /* display: weak */
  }
  else                                  /* ok */
  {
    LCD_EEString(OK_str);               /* display: ok */
  }


  /*
   *  probing
   */

  /* display start of probing */
  LCD_NextLine_EEString(Running_str);   /* display: probing... */

  /* try to discharge any connected component */
  DischargeProbes();
  if (Check.Found == COMP_ERROR)   /* discharge failed */
  {
    goto result;                   /* skip all other checks */
  }

  /* enter main menu if requested by short-circuiting all probes */
  if (AllProbesShorted() == 3)
  {
    MainMenu();                    /* enter mainmenu */;
    goto end;                      /* new cycle after job is is done */
  }

  /* check all 6 combinations of the 3 probes */ 
  CheckProbes(TP1, TP2, TP3);
  CheckProbes(TP2, TP1, TP3);
  CheckProbes(TP1, TP3, TP2);
  CheckProbes(TP3, TP1, TP2);
  CheckProbes(TP2, TP3, TP1);
  CheckProbes(TP3, TP2, TP1);

  /* if component might be a capacitor */
  if ((Check.Found == COMP_NONE) ||
      (Check.Found == COMP_RESISTOR))
  {
    /* tell user to be patient with large caps :-) */
    LCD_Space();
    LCD_Char('C');    

    /* check all possible combinations */
    MeasureCap(TP3, TP1, 0);
    MeasureCap(TP3, TP2, 1);
    MeasureCap(TP2, TP1, 2);
  }


  /*
   *  output test results
   */

result:

  LCD_Clear();                     /* clear LCD */
  LCD_NextLine_Mode(MODE_KEEP | MODE_KEY);

  /* call output function based on component type */
  switch (Check.Found)
  {
    case COMP_ERROR:
      Show_Error();
      goto end;
      break;

    case COMP_DIODE:
      Show_Diode();
      break;

    case COMP_BJT:
      Show_BJT();
      break;

    case COMP_FET:
      Show_FET();
      break;

    case COMP_IGBT:
      Show_IGBT();
      break;

    case COMP_THYRISTOR:
      Show_Special();
      break;

    case COMP_TRIAC:
      Show_Special();
      break;

    case COMP_RESISTOR:
      Show_Resistor();
      break;

    case COMP_CAPACITOR:
      Show_Capacitor();
      break;

    default:                  /* no component found */
      Show_Fail();
      goto end;
  }

  /* component was found */
  RunsMissed = 0;             /* reset counter */
  RunsPassed++;               /* increase counter */


  /*
   *  take care about cycling and power-off
   */

end:

  #ifdef HW_RELAY
  ADC_DDR = (1<<TP_REF);              /* short circuit probes */
  #endif

  LCD_NextLine_Mode(MODE_NONE);       /* reset next line mode */

  /* get key press or timeout */
  Test = TestKey((uint16_t)CYCLE_DELAY, 12);

  if (Test == 0)              /* timeout (no key press) */
  {
    /* check if we reached the maximum number of rounds (continious mode only) */
    if ((RunsMissed >= CYCLE_MAX) || (RunsPassed >= CYCLE_MAX * 2))
    {
      goto power_off;              /* -> power off */
    }
  }
  else if (Test == 1)         /* short key press */
  {
    /* a second key press triggers extra functions */
    MilliSleep(50);
    Test = TestKey(300, 0);

    if (Test > 0)           /* short or long key press */
    {
      #ifdef HW_RELAY
      ADC_DDR = 0;               /* remove short circuit */
      #endif

      MainMenu();                /* enter main menu */
      goto end;                  /* re-run cycle control */
    }
  }
  else if (Test == 2)         /* long key press */
  {
    goto power_off;              /* -> power off */
  }
  #ifdef HW_ENCODER
  else if (Test == 4)         /* rotary encoder: left turn */
  {
    MainMenu();                  /* enter main menu */
    goto end;                    /* re-run cycle control */
  }
  #endif

  /* default action (also for rotary encoder) */
  goto start;                 /* -> next round */


power_off:

  /* display feedback (otherwise the user will wait :-) */
  LCD_Clear();
  LCD_EEString(Bye_str);

  wdt_disable();                        /* disable watchdog */
  CONTROL_PORT &= ~(1 << POWER_CTRL);   /* power off myself */

  return 0;
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef MAIN_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
