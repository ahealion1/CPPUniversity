/*======================================*/
/* DIT Erasmus 2018 - MCU Renesas	*/
/* Written by: 	Alex Healion		*/
/* 		Lloyd Mundo		*/
/* 		Conor Davenport		*/
/* Last Updated: 19/06/2018             */
/*======================================*/

/*======================================*/
/* Include                              */
/*======================================*/
#include "iodefine.h"
#include "math.h"


/*======================================*/
/* Symbol definitions                   */
/*======================================*/

/* Constant settings */
#define PWM_CYCLE       24575           /* Motor PWM period (16ms)     			*/
#define SERVO_CENTER    2380            /* Servo center value          			*/
#define HANDLE_STEP     13              /* 1 degree value              			*/
#define motorMin		1534			/* 1ms	ESC.h => 1000        			*/
#define motorMax		3069			/* 2ms	ESC.h => 2000		  			*/
#define armVal 			2300			/* 1.5ms ESC.h => 1500		   			*/
#define calDelay		8000			/* 8s calibration delay for motorInit	*/
#define stopPulse		2300			/* 1.5ms ESC.h => 1500		   			*/
#define motorSlow		2434			/* Slowest speed of the motor  			*/

/* Sensor bar masks */
#define MASK4_0         0xf0            /* O O O O  X X X X            			*/
#define MASK0_4         0x0f            /* X X X X  O O O O            			*/
#define MASK4_4         0xff            /* O O O O  O O O O		       			*/
#define MASKE_6			0xe6

/*======================================*/
/* Prototype declarations               */
/*======================================*/
void init(void);
unsigned char sensor_inp( unsigned char mask );
unsigned char dipsw_get( void );
void led_out_m( unsigned char led );
void timer( unsigned long timer_set );
void motor( float speed );
void motorInit( void );
void handle( float angle );
int reverse( int );
float pControl( float );
float iControl( float );
float dControl( float );
unsigned char startbar_get( void );
int check_crossline( void );
int check_rightline( void );
int check_leftline( void );
void setSpeedMode( void );
float getSensorBar( void );
void rightAngleTurn( void );

/*======================================*/
/* Global variable declarations         */
/*======================================*/
unsigned long   cnt0;
unsigned long   cnt1;
int             pattern;
float 			spd1, spd2, spd3, spd4;
int rAngle;

/* PID control variables */
float 	setPoint = 0;
float 	error;
float 	Kp = 0.16;
float 	Ti = 0.4;
float 	dT = 0.016;
float 	Td = 0.001;
float	prevE = 0;
float 	prevIn = 0;
float 	position = 0;


/***********************************************************************/
/* Main program                                                        */
/***********************************************************************/
void main(void)
{
	spd1 = spd2 = spd3 = spd4 = 0;

    /* Initialize MCU functions */
    init();
    motorInit();
    setSpeedMode();
    handle( 0 );
    motor( 5 );
    timer( 300 );

    while(1)
    {
    	rAngle = check_crossline();
    	if( rAngle == 1 )
    	{
    		rightAngleTurn();
    	}
    	else
    	{
        	if ( position<=5 && position>=-5 )
        	{
        		motor( spd1 );
        	}
        	else if( position<=8 && position>=-8 )
        	{
        		motor( spd2 );
        	}
        	else if( position<=10 && position>=-10 )
        	{
        	    motor( spd3 );
        	}
        	else
        	{
        		motor( spd4 );
        	}
    	}
    }
}


/***********************************************************************/
/* RX62T Initialization                                                */
/***********************************************************************/
void init(void)
{
    // System Clock
    SYSTEM.SCKCR.BIT.ICK = 0;               //12.288*8=98.304MHz
    SYSTEM.SCKCR.BIT.PCK = 1;               //12.288*4=49.152MHz

    // Port I/O Settings
    PORT1.DDR.BYTE = 0x03;                  //P10:LED2 in motor drive board

    PORT2.DR.BYTE  = 0x08;
    PORT2.DDR.BYTE = 0x1b;                  //P24:SDCARD_CLK(o)
                                            //P23:SDCARD_DI(o)
                                            //P22:SDCARD_DO(i)
                                            //CN:P21-P20
    PORT3.DR.BYTE  = 0x01;
    PORT3.DDR.BYTE = 0x0f;                  //CN:P33-P31
                                            //P30:SDCARD_CS(o)
    //PORT4:input                           //sensor input
    //PORT5:input
    //PORT6:input

    PORT7.DDR.BYTE = 0x7e;                  //P76:LED3 in motor drive board
                                            //P75:
                                            //P74:
                                            //P73:PWM(motor)
                                            //P72:PWM()
                                            //P71:PWM(servo)
                                            //P70:Push-button in motor drive board
    PORT8.DDR.BYTE = 0x07;                  //CN:P82-P80
    PORT9.DDR.BYTE = 0x7f;                  //CN:P96-P90
    PORTA.DR.BYTE  = 0x0f;                  //CN:PA5-PA4
                                            //PA3:LED3(o)
                                            //PA2:LED2(o)
                                            //PA1:LED1(o)
                                            //PA0:LED0(o)
    PORTA.DDR.BYTE = 0x3f;                  //CN:PA5-PA0
    PORTA.DR.BYTE = 0xFF;
    PORTB.DDR.BYTE = 0xff;                  //CN:PB7-PB0
    PORTD.DDR.BYTE = 0x0f;                  //PD7:TRST#(i)
                                            //PD5:TDI(i)
                                            //PD4:TCK(i)
                                            //PD3:TDO(o)
                                            //CN:PD2-PD0
    PORTE.DDR.BYTE = 0x1b;                  //PE5:SW(i)
                                            //CN:PE4-PE0

    // Compare match timer
    MSTP_CMT0 = 0;                          //CMT Release module stop state
    MSTP_CMT2 = 0;                          //CMT Release module stop state

    ICU.IPR[0x04].BYTE  = 0x0f;             //CMT0_CMI0 Priority of interrupts
    ICU.IPR[0x05].BYTE 	= 0x0f;
    ICU.IER[0x03].BIT.IEN4 = 1;             //CMT0_CMI0 Permission for interrupt
    ICU.IER[0x03].BIT.IEN5 = 1;
    CMT.CMSTR0.WORD     = 0x0000;           //CMT0,CMT1 Stop counting
    CMT0.CMCR.WORD      = 0x00C3;           //PCLK/512
    CMT1.CMCR.WORD		= 0x00C3;
    CMT0.CMCNT          = 0;
    CMT1.CMCNT			= 0;
    CMT0.CMCOR          = 96;               //1ms/(1/(49.152MHz/512))
    CMT1.CMCOR			= 1536;//1920;				//20ms
    CMT.CMSTR0.WORD     = 0x0003;           //CMT0,CMT1 Start counting

    MSTP_MTU = 0;

    MTU.TSTRA.BYTE = 0x00;
    //timer start register
    //b0		CST0 - counter start 0
    //			0 - MTU0.TCNT count operation is stopped
    //			1 - MTU0.TCNT performs count operation
    //b1		CST1
    //b2		CST2
    //b3 - b5	reserved
    //b6		CST3
    //b7		CST4

    MTU3.TCR.BYTE = 0x23;
    //MTU4.TCR.BYTE = 0x00;
    MTU4.TCR.BYTE = 0x33;
    //timer control register
    //b0 - b2	TPSC - time prescaler select
    //b3 - b4	CKEG - clock edge select
    //b5 - b7	CCLR - counter clear source

    MTU3.TCNT =  MTU4.TCNT = 0;
    //timer counter clear

    MTU3.TGRA = MTU3.TGRC = PWM_CYCLE;
    //pwm servo
    //timer general register
    //used as buffer register

    MTU3.TGRB = MTU3.TGRD = SERVO_CENTER;
    //timer general register
    //used as buffer register

    MTU4.TGRA = MTU4.TGRC = PWM_CYCLE;
    MTU4.TGRB = MTU4.TGRD = 2300;
    //timer general register
    //used as buffer register

    MTU.TOCR1A.BYTE = 0x40;
    //timer output control registers
    //b0		OLSP - output level select P
    //b1		OLSN - output level select N
    //both bits above affect pwm phase
    //b2		TOCS - timer output control select
    //b3		TOCL - TOC register write protection
    //b4 - b5	reserved
    //b6		PSYE - pwm synchronous output enable
    //b7		reserved

    MTU3.TMDR1.BYTE = 0x38;
    MTU4.TMDR1.BYTE = 0x30;//00
    //timer mode register
    //b0 - b3	MD - mode select
    //specify the timer operating mode
    //0x8 = reset synchronised pwm mode
    //0x0 = normal operation
    //b4		BFA - buffer operation A
    //			0 = TGRA, TGRC operate normally
    //			1 = TGRA, TGRC used together for buffer operation
    //b5		BFB - buffer operation B
    //			0 = TGRB, TGRD operate normally
    //			1 = TGRB, TGRD used together for buffer operation
    //b6 - b7	reserved

    MTU.TOERA.BYTE  = 0xF7;
    //C7
    //timer output master enable register
    //b0		OE3B - master enable MTIOC3B
    //b1		OE4A - master enable MTIOC4A
    //b2		OE4B - master enable MTIOC4B
    //b3		OE3D - master enable MTIOC3D
    //b4		OE4C - master enable MTIOC4C
    //b5		OE4D - master enable MTIOC4D
    //b6 - b7	reserved
    //0xCF = 1100 1111
    //MTIOC3B, MTIOC4A, MTIOC4B
    //P71	   P72		P73

    MTU.TSTRA.BYTE  = 0x40;
    //timer start register
    //b0		CST0 - counter start 0
    //			0 - MTU0.TCNT count operation is stopped
    //			1 - MTU0.TCNT performs count operation
    //b1		CST1
    //b2		CST2
    //b3 - b5	reserved
    //b6		CST3
    //b7		CST4
    //0xC0 = 1100 0000 = start MTU3, MTU4
}

/***********************************************************************/
/* Interrupt                                                           */
/***********************************************************************/
#pragma interrupt Excep_CMT0_CMI0(vect=28)
void Excep_CMT0_CMI0(void)
{
    cnt0++;
    cnt1++;
}

/***********************************************************************/
/* PID Steering System Interrupt         	                           */
/***********************************************************************/
#pragma interrupt Excep_CMT1_CMI0(vect=29)
void Excep_CMT1_CMI0(void)
{
	float output;
	float fb;
	float clamp;

	fb = getSensorBar();
	error = setPoint - fb;

	if( rAngle == 1 )
		clamp = 8;
	else
		clamp = 16;

	output = pControl( error ) + iControl( error );// + dControl( error );
	position = position + output;


	if( position > clamp )
	{
		position = clamp;
	}
	if( position < -clamp )
	{
		position = -clamp;
	}
	handle( position );
}

/***********************************************************************/
/* Sensor state detection                                              */
/* Arguments:       masked values                                      */
/* Return values:   sensor value                                       */
/***********************************************************************/
unsigned char sensor_inp( unsigned char mask )
{
    unsigned char sensor;

    sensor  = ~PORT4.PORT.BYTE;

    sensor &= mask;

    return sensor;
}

/***********************************************************************/
/* DIP switch value read                                               */
/* Return values: Switch value, 0 to 15                                */
/***********************************************************************/
unsigned char dipsw_get( void )
{
    unsigned char sw,d0,d1,d2,d3;

    d0 = ( PORT6.PORT.BIT.B3 & 0x01 );  /* P63~P60 read                */
    d1 = ( PORT6.PORT.BIT.B2 & 0x01 ) << 1;
    d2 = ( PORT6.PORT.BIT.B1 & 0x01 ) << 2;
    d3 = ( PORT6.PORT.BIT.B0 & 0x01 ) << 3;
    sw = d0 | d1 | d2 | d3;

    return  sw;
}

/***********************************************************************/
/* LED control in MCU board                                            */
/* Arguments: Switch value, LED0: bit 0, LED1: bit 1. 0: dark, 1: lit  */
/*                                                                     */
/***********************************************************************/
void led_out_m( unsigned char led )
{
    led = ~led;
    PORTA.DR.BYTE = led & 0x0f;
}

/****************************************/
/*	arguments: time value, 1 = 1ms		*/
/*	delay function						*/
/****************************************/
void timer( unsigned long timer_set )
{
	cnt0 = 0;
	while( cnt0 < timer_set ){};
}

/************************************************************/
/* 	Motor speed control                                     */
/*	Arguments: speed 0 -> 100, stopped to full speed		*/
/************************************************************/
void motor( float speed )
{
	float ret;
	ret = ( 6.7071 * speed ) + 2398.3;
    MTU4.TGRD = ret;
}

/********************************/
/*	Motor ESC initialisation	*/
/********************************/
void motorInit()
{
	MTU4.TGRB = MTU4.TGRD = motorMax;		//Send max value of motor
	timer( calDelay );						//hold for 8s
	MTU4.TGRB = MTU4.TGRD = motorMin;		//Send min value of motor
	timer( calDelay );						//hold for 8s
	MTU4.TGRB = MTU4.TGRD = armVal;			//Sends arm value to ready the motor
	timer( 5000 );							//hold for 5s
}

/***********************************************************************/
/* Servo steering operation                                            */
/* Arguments:   servo operation angle: -40 to 40                       */
/*              -40: 40-degree turn to right, 0: straight,              */
/*               40: 40-degree turn to left                           */
/***********************************************************************/
void handle( float angle )
{
    /* When the servo move from left to right in reverse, replace "-" with "+". */
    MTU3.TGRD = SERVO_CENTER - angle * HANDLE_STEP;
}


/************************************/
/*	binary bit reverse function		*/
/* 	0011 -> 1100					*/
/************************************/
int reverse( int n )
{
	n = ((n & 0x55) << 1) | ((n & 0xAA) >> 1);	//swap even and odd bits
	n = ((n & 0x33) << 2) | ((n & 0xCC) >> 2);	//swap bit0,1 with bit2,3

	return n;
}

/************************************/
/*	P control calculation			*/
/* 	Arguments: error				*/
/* 	Return value: P variable		*/
/************************************/
float pControl ( float e )
{
	float p;

	p = e * Kp;
	return p;
}

/************************************/
/*	I control calculation			*/
/* 	Arguments: error				*/
/* 	Return value: I variable		*/
/************************************/
float iControl( float e )
{
	float in;
	int clamp;
	clamp = 0.5;
	in = ( ( Kp / Ti ) * e * dT ) + prevIn;
	if( in > clamp)
	{
		in = clamp;
	}
	else if( in < -clamp )
	{
		in = -clamp;
	}
	prevIn = in;
	return in;
}

/************************************/
/*	D control calculation			*/
/* 	Arguments: error				*/
/* 	Return value: D variable		*/
/************************************/
float dControl( float e )
{
	float d;
	d = ( Kp * Td * ( e - prevE )) / dT;
	if( e > 0 && d > 0 )
	{
		d = 0;
	}
	if( e < 0 && d < 0 )
	{
		d = 0;
	}
	prevE = e;
	if( (e > 0 && prevE < 0) || (e < 0 && prevE > 0) )
	{
		prevE = 0;
	}
	return d;
}

/***********************************************************************/
/* Read start bar detection sensor                                     */
/* Return values: Sensor value, ON (bar present):1,                    */
/*                              OFF (no bar present):0                 */
/***********************************************************************/
unsigned char startbar_get( void )
{
    unsigned char b;

    b  = ~PORT4.PORT.BIT.B0 & 0x01;     /* Read start bar signal       */

    return  b;
}

/***********************************************************************/
/* Cross line detection processing                                     */
/* Return values: 0: no cross line, 1: cross line                      */
/***********************************************************************/
int check_crossline( void )
{
    unsigned char b;
    int ret;

    ret = 0;
    b = sensor_inp( MASK4_4 );

    if( b==0xff || b==0xfe || b==0x7f )
        ret = 1;

    return ret;
}

/***********************************************************************/
/* Right half line detection processing                                */
/* Return values: 0: not detected, 1: detected                         */
/***********************************************************************/
int check_rightline( void )
{
    unsigned char b;
    int ret;

    ret = 0;
    b = sensor_inp( MASK4_4 );
    if( b == 0x1f ) {
        ret = 1;
    }
    return ret;
}

/***********************************************************************/
/* Left half line detection processing                                 */
/* Return values: 0: not detected, 1: detected                         */
/***********************************************************************/
int check_leftline( void )
{
    unsigned char b;
    int ret;

    ret = 0;
    b = sensor_inp( MASK4_4 );
    if( b == 0xf8 ) {
        ret = 1;
    }
    return ret;
}

/***********************************************************************/
/* Set speed mode according to DIP switch                              */
/* 0x1 -> very slow, 0x02 -> slow, 									   */
/* 0x03 -> moderate speed, 0x04 -> very fast	                       */
/***********************************************************************/
void setSpeedMode( void )
{
	switch( dipsw_get() )
	{
	case 0x01:
		spd1 = 3;
		spd2 = 2;
		spd3 = 1;
		spd4 = 1;
		led_out_m( 0x01 );
		break;

	case 0x02:
		spd1 = 4;
		spd2 = 3;
		spd3 = 2;
		spd4 = 1;
		led_out_m( 0x02 );
		break;

	case 0x03:
		spd1 = 5;
		spd2 = 4;
		spd3 = 3;
		spd4 = 2;
		led_out_m( 0x04 );
		break;

	case 0x04:
		spd1 = 6;
		spd2 = 4;
		spd3 = 4;
		spd4 = 3;
		led_out_m( 0x08 );
		break;

	default:
		spd1 = -5;
		spd2 = -5;
		spd3 = -5;
		spd4 = -5;
		led_out_m( 0x0f );
		break;
	}
}

/***********************************************************************/
/* Sensor bar position detection                                       */
/* Return value: value according to sensor bar position                */
/***********************************************************************/
float getSensorBar( void )
{
	unsigned char s = sensor_inp( MASK4_4 );
	switch( s )
	{
	case 0x80:			/* 1000 0000 */
		return -40;
		break;

	case 0xC0:			/* 1100 0000 */
		return -34.3;
		break;

	case 0xE0:			/* 1110 0000 */
		return -28.6;
		break;

	case 0x60:			/* 0110 0000 */
		return -22.9;
		break;

	case 0x70:			/* 0111 0000 */
		return -17.1;
		break;

	case 0x30:			/* 0011 0000 */
		return -11.4;
		break;

	case 0x38:			/* 0011 1000 */
		return -5.7;
		break;

	case 0x18:			/* 0001 1000 */
		return 0;
		break;

	case 0x1C:			/* 0001 1100 */
		return 5.7;
		break;

	case 0x0C:			/* 0000 1100 */
		return 11.4;
		break;

	case 0x0E:			/* 0000 1110 */
		return 17.1;
		break;

	case 0x06:			/* 0000 0110 */
		return 22.9;
		break;

	case 0x07:			/* 0000 0111 */
		return 28.6;
		break;

	case 0x03:			/* 0000 0011 */
		return 34.3;
		break;

	case 0x01:			/* 0000 0001 */
		return 40;
		break;

	default:
		return 0;
		break;
	}
}

/***********************************************************************/
/* Executes right angle turn                                           */
/***********************************************************************/
void rightAngleTurn( void )
{
	motor( -4.5 );
	timer( 50 );
	motor( -2.5 );
	timer( 100 );
	while( 1 )
	{
		/*right turn*/
		if( sensor_inp( MASK0_4 ) == 0x0f )
		{
			CMT.CMSTR0.WORD = 0x0001;           //stop PI control interrupt
			handle( -30 );
			motor( -1.5 );
			error = 0;
			break;
		}

		/*left turn*/
		else if( sensor_inp( MASK4_0 ) == 0xf0)
		{
			CMT.CMSTR0.WORD = 0x0001;           //stop PI control interrupt
			handle( 30 );
			motor( -1.5 );
			error = 0;
			break;
		}
		else
			motor( -2.5 );
	}

	while( 1 )
	{
		/*wait until 0001 1000 before restarting the interrupt*/
		if( sensor_inp( MASK4_4 ) == 0x18 )
		{
			motor( -1.5 );
			timer( 95 );
			CMT.CMSTR0.WORD = 0x0003;           //restart PI control interrupt
			break;
		}
	}
}

/***********************************************************************/
/* end of file                                                         */
/***********************************************************************/
