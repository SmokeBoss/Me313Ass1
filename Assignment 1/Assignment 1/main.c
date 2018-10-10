/*
* Assignment 1.c
*
* Created: 26/08/2018 3:25:44 PM
* Author : ssar297 and wliu641
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

// Global variables
volatile uint16_t currentTime = 0; 
volatile uint8_t blinkTimer = 0;
volatile uint16_t lightState = 0;
volatile uint8_t cMode = 0;
volatile uint8_t stateMultiplier = 2;
volatile uint8_t prevS = 1;
volatile uint8_t prevS2 = 1;
volatile uint8_t blinks = 1;
volatile uint16_t carSpeedTime = 0;
volatile uint16_t carSpeed = 0;
volatile uint8_t RLC = 0;
volatile uint8_t redLightBlinks = 0;
volatile uint16_t recordedTime = 0;
volatile uint8_t numberOfSpeedingCars = 0;

/*
CONNECTIONS:

PB1 - PWM for part 3
PB2 - PWM for part 4

Connect these pins using wires:
PC2 (Green) - LED2
PC1 (Yellow) - LED1
PC0 (Red) - LED0

PC3 (Config Blink) - LED4

PC4 (ADC in) - ADC (middle connection for potentiometer)
VTG (ADC power) - ADC Power

PD1 (Config Switch) - SW0

PD3 (Interrupt LB1) - SW5
PD2 (Interrupt LB2) - SW6
PD0 (LB3) - SW7

PC5 (Red Light Camera) - LED3

*/

//controls the periodic switching of the lights based on the period the user has set, which is between 1 - 4 seconds (default 1s)
// its placed up here to avoid warnings
void light_control(){
	// If we've passed the correct time period, and we're not in configuration mode
	// note that lightState denotes red, yellow and green states for our code. If the value is 0 then its red, 1 is yellow, and 2 is green
	if ((currentTime >= (stateMultiplier*250)) && !(cMode && ((lightState) == 0))){ // timing is typically .5 seconds slow due to human reaction time
		// Turn off the current light
		PORTC |= (1<<(lightState));
		// Increase light state
		lightState++;
		volatile uint8_t efficient_lightState = (lightState%3); //We add this to ensure we don't perform the modulus more than we need as it is inefficient can be removed for lightState %= 3 I think.
		lightState = efficient_lightState;
		// Turn on current (new) light.
		PORTC &= ~(1<<(lightState));
		// reset current time so that we count another period
		currentTime = 0;
	}
}

// Timer comparison ISR counts every 2 ms on our 3 variables
ISR(TIMER2_COMP_vect)
{
	// Every 2 ms we increase our 3 timers.
	carSpeedTime++;
	blinkTimer++;
	currentTime++;
	

	// Move these out? Slightly less accurate but we're gaining time for our other functions
	light_control();
}

//timer interrupt for timer 1
ISR(TIMER1_OVF_vect){
}

//determines how the LEDs that represent configuration mode and red light camera blink, based on user inputs
void LEDBlink (void){
	// If our red light camera flag is set, and blink timer is correct then we start our blinks for task 4
	if (RLC && (redLightBlinks <= 3) && (blinkTimer >= 250)){
		// toggle the bit every 500ms and increase blink timer
		PORTB ^= (1<<PB4);
		PORTC ^= (1<<PC5);
		redLightBlinks++;
		blinkTimer = 0;
		} else if (redLightBlinks >= 4) {
		// once the red light blinks has hit 4 indicating 2 blinks then we finish our sequence and reset our bits.
		redLightBlinks = 0;
		RLC = 0;
	}
	
	//starts blinking LED4 once condition for configuration mode is met
	if ((cMode == 1) && (currentTime >= 250) && ((lightState) == 0)){
		//pauses blinking for 3 seconds after a specific number of blinks based on
		//potentiometer that sets traffic light switching period
		if (blinks == stateMultiplier){
			if (currentTime>= (stateMultiplier*250)){ // stateMultiplier is now multiplied by 2 for optimization
				currentTime = 0;
				blinks = 0;
				}else{
				return;
			}
		}
		// Switching LED state every 500ms
		PORTC ^= (1<<PC3);
		blinks++;
		
		// reset timer count after the required number of steps have been completed
		currentTime = 0;
	}
}

/*configuration mode sets the period of switching for the traffic light. System goes into
config mode at next red light and stays at red light until config mode is turned off using SW0 */
void configuration_mode (void){
	// take in button input to find whether or not SW0 is depressed (connected to PD1).
	// if cMode is there, and we compress the switch again, the conditions will be met for us to turn cMode off. The prevS portion is checking for button state change
	// the lightState portion was added such that we can only turn off the configuration mode when we are actually in configuration mode (red light)
	if ((cMode == 1) && (prevS &(1<<PD1)) && ((PIND &(1<<PD1)) == 0) && (lightState == 0)){
		cMode = 0;
		// Reset current time (might not be needed)this will ensure correct time for light flash
		currentTime = 0;
		blinks = 0;
		// ensure that the light is not on when configuration is turned off
		PORTC |= (1<<PC3);
		}else if ((cMode == 0) && (prevS &(1<<PD1)) && ((PIND &(1<<PD1)) == 0)){// If it has been depressed, we toggle the configuration mode bit to show the state.
		cMode = 1;
	}
	
	// store the current state of the button into prevS
	prevS = PIND;
	
	// If we are in fact in red light state, and c mode
	if (cMode && ((lightState) == 0)) {
		// Reset light state to 0
		lightState = 0;
		
		// flag single conversion bit
		ADCSRA |= (1<<ADSC);
		// Wait for the thing to finish converting
		while(ADCSRA & (1<<ADSC)){
		}
		
		// Write ADC to adcValue, ADC should be 10 bits
		volatile uint16_t adcValue = ADC;
		// Perform calculation once here so we dont do it twice later
		volatile uint16_t current_state_multiplier = ((adcValue/256)+1)*2; // Can we use uint8 here? further investigation is required
		
		//checks to see if potentiometer value has been changed
		if ((stateMultiplier) != current_state_multiplier){
			//restarts sequence when potentiometer value changes
			blinks = 0;
			currentTime = 0;
			// set PC3 to low to restart blink sequence (ensure we start on low)
			PORTC |= (1<<PC3);
		}
		// dividing by 4 and adding one will give the correct value, we multiply by 2 here for optimization purposes
		stateMultiplier = current_state_multiplier;

	}
}

// This function is responsible for the functionality of the red light camera for task 4. When a button is pressed we log an increase in the number of speeding cars, increment our PWM, and flash the lights twice. Note that if another car speeds past the gate
// before the lights have finished flashing then we will not restart the light sequence. 
void red_light_camera() {
	//Turn red light camera flag on if LB3 switch changes from high to low and the current light is red
	if (((PIND & (1<<PD0)) == 0) && (prevS2 &(1<<PD0)) && ((lightState) == 0)){
		
		// If the red light camera is going, then we do not flash again
		if(!RLC){
		// Debugging and testing timer response
		//PORTB ^= (1<<PB0);
		// Raise the flag that we have a red light infringement
		RLC = 1;
		// set the time to immediately start our timer.
		blinkTimer = 250; 
		}
		
		//increase the duty cycle by 1% for every button press (representing one car)
		numberOfSpeedingCars++;
		OCR1B = (numberOfSpeedingCars*255)/100; // 1% of 255 rounded up TEST We need to change this.
		
	}
	prevS2 = PIND;
}

//PD3 interrupt
//this interrupt starts/resets our car speed timer when LB1 is passed (switch SW5 is pressed)
ISR(INT1_vect){
	//Start the timer count
	carSpeedTime = 0;
	PORTB ^= (1<<PB0);
	// Debugging and testing timer response
	//PORTB ^= (1<<PB0);
}

// PD2 ISR
/*this interrupt is activated when LB2 is passed (switch SW6 is pressed), and calcuates the duty cycle of our PWM,
by taking the current car speed time during activation to calculate the speed of the car*/
ISR(INT0_vect){
	recordedTime = carSpeedTime;
	// Debugging and testing timer response
	//PORTB ^= (1<<PB0);
	//20m * 3.6 (conversion to Km/h) * 500 (conversion of ms to seconds)
	carSpeed = 36000/recordedTime;

	//if recorded speed is greater than 100, speed is capped at 100 for our duty cycle
	if (carSpeed >= 100){
		carSpeed = 100;
	}
	
	OCR1A = ((carSpeed*255)/100);// Scale OCR1A to be between 0 - 256 instead of 0 - 100
}

int main(void){
	//set outputs
	DDRB |= (1<<DDB0 | 1<<DDB1 | 1<<DDB2 | 1<<DDB3 | 1<<DDB4 | 1<<DDB5);
	PORTB |= (1<<DDB0);
	DDRC |= (1<<DDC0 | 1<<DDC1 | 1<<DDC2 | 1<<DDC3 | 1<<DDC5);
	PORTC |= (1<<PC3 | 1<<PC5);
	// Set PC4 for adc input
	DDRC &= ~(1<<DDC4);
	// Set port D to be inputs
	DDRD &= ~(1<<DDD0 | 1<<DDD1 | 1<<DDD2 | 1<<DDD3 | 1<<DDD4 | 1<<DDD5);

	// Initialize button ISR for PD2 to falling edge
	GICR |= (1<<INT0);
	MCUCR |= (1<<ISC01 | 1<<ISC00);
	
	// Initialize button ISR for PD3 to rising edge
	GICR |= (1<<INT1);
	MCUCR |= (1<<ISC11);
	MCUCR &= ~(1<<ISC10);
	
	// Timer setup for prescaling of 8
	// OCIE2 Timer 2 match enabled set
	// TOEI1 is timer overflow flag
	TIMSK |= (1<<OCIE2 | 1<<TOIE1);
	// CS21 sets prescaler of 8
	// WGM21 sets CTC with a top value of OCR2
	TCCR2 |= (1<<CS21 | 1<<WGM21);
	
	// initial value set
	OCR1A = 0x00;
	OCR1B = 0x00;
	OCR2 = 255;
	
	// Set up bits to fast PWM with a top value of 0x00FF
	// COM1A1 is set so we have OC1A clearing on match.
	TCCR1A |= (1<<WGM10 | 1<<COM1A1);
	TCCR1B |= (1<<WGM12 | 1<<CS12);
	TCCR1A |= (1<<COM1B1);
	TCNT1 = 0xFFF - 3906;
	
	// Initialise ADC for single sample;
	ADMUX = 0;
	// Code to setup ADMUX
	ADMUX &= ~(1<<MUX3 | 1<<MUX1 | 1<<MUX0);
	ADMUX |= (1<<MUX2);
	
	// Code to set ADCSRA
	ADCSRA |= (1<<ADEN);
	ADCSRA |= (1<<ADPS0 | 1<<ADPS1);
	
	//Enable global interrupts
	sei();
	
	while (1)
	{
		_delay_us(1);
		configuration_mode();
		red_light_camera();
		LEDBlink();
	}
}