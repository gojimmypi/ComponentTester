/* ************************************************************************
 *
 *   global functions
 *
 *   (c) 2012-2015 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz K�bbeler
 *
 * ************************************************************************ */


/*
 *  In each source file we create a local ID definition. If the ID definition
 *  is unset we may import functions of that specific source file.
 */

/* ************************************************************************
 *   functions from main.c
 * ************************************************************************ */

#ifndef MAIN_C

  extern void Show_SemiPinout(uint8_t A, uint8_t B, uint8_t C);

#endif


/* ************************************************************************
 *   functions from LCD module specific driver
 * ************************************************************************ */

#ifndef LCD_DRIVER_C

  extern void LCD_BusSetup(void);
  extern void LCD_Init(void);
  extern void LCD_ClearLine(uint8_t Line);
  extern void LCD_Clear(void);
  extern void LCD_Pos(uint8_t x, uint8_t y);
  extern void LCD_Cursor(uint8_t Mode);
  extern void LCD_Char(unsigned char Char);

  #ifdef SW_CONTRAST
    extern void LCD_Contrast(uint8_t Contrast);
  #endif

#endif


/* ************************************************************************
 *   functions from display.c
 * ************************************************************************ */

#ifndef DISPLAY_C

  extern void LCD_EEString(const unsigned char *String);
  extern void LCD_EEString_Space(const unsigned char *String);
  extern void LCD_ProbeNumber(unsigned char Probe);

  extern void LCD_ClearLine2(void);
  extern void LCD_Space(void);

  extern void LCD_NextLine(void);
  extern void LCD_NextLine_Mode(uint8_t Mode);
  extern void LCD_NextLine_EEString(const unsigned char *String);
  extern void LCD_NextLine_EEString_Space(const unsigned char *String);

#endif


/* ************************************************************************
 *   functions from pause.c
 * ************************************************************************ */

#ifndef PAUSE_C

  extern void MilliSleep(uint16_t Time);

#endif


/* ************************************************************************
 *   functions from adjust.c
 * ************************************************************************ */

#ifndef ADJUST_C

  extern void SetAdjustDefaults(void);
  extern uint8_t CheckSum(void);
  extern void SafeAdjust(void);
  extern void LoadAdjust(void);

  extern void ShowAdjust(void);
  extern uint8_t SelfAdjust(void);

  extern uint8_t SelfTest(void);

#endif


/* ************************************************************************
 *   functions from user.c
 * ************************************************************************ */

#ifndef USER_C

  extern int8_t CmpValue(uint32_t Value1, int8_t Scale1,
    uint32_t Value2, int8_t Scale2);

  #ifdef SW_INDUCTOR
    extern uint32_t RescaleValue(uint32_t Value, int8_t Scale, int8_t NewScale);
  #endif

  #ifdef SW_SQUAREWAVE
    extern void DisplayFullValue(uint32_t Value, uint8_t DecPlaces, unsigned char Unit);
  #endif

  extern void DisplayValue(uint32_t Value, int8_t Exponent, unsigned char Unit);
  extern void DisplaySignedValue(int32_t Value, int8_t Exponent, unsigned char Unit);

  extern uint8_t TestKey(uint16_t Timeout, uint8_t Mode);
  extern void WaitKey(void);
  extern int8_t ShortCircuit(uint8_t Mode);
  extern void MainMenu(void);

#endif


/* ************************************************************************
 *   functions from extras.c
 * ************************************************************************ */

#ifndef EXTRAS_C

  #ifdef SW_PWM
    extern void PWM_Tool(uint16_t Frequency);
  #endif
  #ifdef SW_SQUAREWAVE
    extern void SquareWave_SignalGenerator(void);
  #endif
  #ifdef SW_ESR
    extern void ESR_Tool(void);
  #endif
  #ifdef HW_ZENER
    extern void Zener_Tool(void);
  #endif
  #ifdef HW_FREQ_COUNTER
    extern void FrequencyCounter(void);
  #endif
  #ifdef SW_ENCODER
    extern void Encoder_Tool(void);
  #endif

#endif


/* ************************************************************************
 *   functions from semi.c
 * ************************************************************************ */

#ifndef SEMI_C

  extern void GetGateThreshold(uint8_t Type);
  extern uint32_t Get_hfe_c(uint8_t Type);
  extern uint16_t GetLeakageCurrent(void);

  extern void CheckDiode(void);

  extern void VerifyMOSFET(void);
  extern void CheckBJTorEnhModeMOSFET(uint8_t BJT_Type, uint16_t U_Rl);
  extern void CheckDepletionModeFET(void);

  extern uint8_t CheckThyristorTriac(void);

#endif


/* ************************************************************************
 *   functions from resistor.c
 * ************************************************************************ */

#ifndef RESISTOR_C

  extern uint16_t SmallResistor(uint8_t ZeroFlag);
  extern void CheckResistor(void);
  extern uint8_t CheckSingleResistor(uint8_t HighPin, uint8_t LowPin);

#endif


/* ************************************************************************
 *   functions from inductor.c
 * ************************************************************************ */

#ifndef INDUCTOR_C

  #ifdef SW_INDUCTOR
    extern uint8_t MeasureInductor(Resistor_Type *Resistor);
  #endif

#endif


/* ************************************************************************
 *   functions from cap.c
 * ************************************************************************ */

#ifndef CAP_C

  #ifdef SW_ESR
    extern uint16_t MeasureESR(Capacitor_Type *Cap);
  #endif

  extern void MeasureCap(uint8_t Probe1, uint8_t Probe2, uint8_t ID);

#endif


/* ************************************************************************
 *   functions from probes.c
 * ************************************************************************ */

#ifndef PROBES_C

  extern void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);
  extern uint8_t ShortedProbes(uint8_t Probe1, uint8_t Probe2);
  extern uint8_t AllProbesShorted(void);
  extern void DischargeProbes(void);
  extern void PullProbe(uint8_t Probe, uint8_t Mode);
  extern uint16_t GetFactor(uint16_t U_in, uint8_t ID);

  extern void CheckProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);

#endif


/* ************************************************************************
 *   functions from ADC.c
 * ************************************************************************ */

#ifndef ADC_C

  extern uint16_t ReadU(uint8_t Probe);

  extern uint16_t ReadU_5ms(uint8_t Probe);
  extern uint16_t ReadU_20ms(uint8_t Probe);

#endif


/* ************************************************************************
 *   functions from wait.S
 * ************************************************************************ */

#ifndef WAIT_S

  /* clock frequency 1 MHz */
  extern void wait5s(void);
  extern void wait4s(void);
  extern void wait3s(void);
  extern void wait2s(void);
  extern void wait1s(void);
  extern void wait1000ms(void);
  extern void wait500ms(void);
  extern void wait400ms(void);
  extern void wait300ms(void);
  extern void wait200ms(void);
  extern void wait100ms(void);
  extern void wait50ms(void);
  extern void wait40ms(void);
  extern void wait30ms(void);
  extern void wait20ms(void);
  extern void wait10ms(void);
  extern void wait5ms(void);
  extern void wait4ms(void);
  extern void wait3ms(void);
  extern void wait2ms(void);
  extern void wait1ms(void);
  extern void wait500us(void);
  extern void wait400us(void);
  extern void wait300us(void);
  extern void wait200us(void);
  extern void wait100us(void);
  extern void wait50us(void);
  extern void wait40us(void);
  extern void wait30us(void);
  extern void wait20us(void);
  extern void wait10us(void);

  /* clock frequency 2 MHz */
  extern void wait5us(void);

  /* clock frequency 4 MHz */
  extern void wait4us(void);
  extern void wait3us(void);
  extern void wait2us(void);

  /* clock frequency 8 MHz */
  extern void wait1us(void);

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */
