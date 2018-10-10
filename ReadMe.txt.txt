Traffic light control usage:

Important connections and pins:

PB1 - PWM for part 3
PB2 - PWM for part 4

Connect these pins:
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

To use the program turn it on and wait for the loading screen to stop. Normal functionality will
immediately begin after all lights have turned off. To enter configuration mode, press switch 0
and wait untill the next red light, whereupon the program will enter configuration mode, as indicated
by LED 4 flashing. Once in configuration mode press switch 0 again to turn off configuration mode.
Turning the potentiometer while in configuration mode will adjust the traffic light period, as
indicated by the number of flashes and pause on LED4. Turning off configuration mode after this
will implement the changes (i.e desired period) for the traffic lights.

Other functions such as the red light camera is automatic, similarly car speed capture is automatic.
Functionality can be tested however, by pressing switch 7 to test the red light camera, and
switch 5 and switch 6 for the car speed capture. The distance between the light barriers represented
by switch 5 and switch 6 is 20m. Therefore a delay of 1 second between button presses should generate a 
PWM with duty cycle of approximately 70% on the output pin PB1.
 
The red light camera can be tested by pressing switch 7 which should flash the red light camera 
twice, note that if the camera is already flashing pressing the button will not restart the 
sequence, but the PWM duty cycle which records the number of cars will change. This PWM can be
accessed on pin PB2 and indicates the number of cars that have run a red light (0-100% represents
0-100 cars).

Thank you for choosing our system, we hope that you are able to enjoy the full functionality 
of the traffic light embedded system designed and coded by Simon Sarkar and Kevin Liu. 
(Ssar297 and Wliu641)