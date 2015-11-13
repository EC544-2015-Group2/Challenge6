# Challenge6
Create a swarm with the Arduinos

Setup: configure each of your Arduino/XBees with 1 input switch and 3 output LEDs (Red, Green, Blue) as per the schematic below. The LEDs should be currentlimited with 220 ohm resistors. A logical 1 on the DIO will energize the LED. The switch will be normally high (pulled up by 10K ohm resistors) and produce a digital zero when pressed. Be sure to debounce the switch in software. 


Programming: Create a single program that operates on each Arduino that realizes a larger system behavior. The details are below. 

1. Basic elements – any device 
1.1 There are three LEDs. The Blue indicates a leader; the Green indicates not infected; and the Red indicates infected
1.2 Pressing the button with either start an infection or clear an infection depending on being a leader or a non-leader 
1.3 One leader is elected on startup per connected network 
1.4 A new leader is elected if an existing leader disappears or is turned off 
1.5 While infected, each device will send out infection messages to other connected devices with a fixed period of 2 seconds 
1.6 A clear infection message has priority over infection message 

2.Leader-specific 
2.1 The leader is immune to infections and cannot infect itself
2.2 Pressing the button will cause a leader to send a “clear message” to other devices. This should happen only once per button press (not continuous). 
2.3 It is possible to have reinfections if not all devices are immediately reachable 

3.Non-leader-specific 
3.1 Pressing the button causes a non-leader to infect itself • Upon receiving a clear infection message, a non-leader will return to the not infected state and propagate the clear message to its neighbors 
3.2 Infections are persistent; repeat infections can occur if any of the non-leaders in the network are not cleared 
3.3 A non-leader is immune to infection for 3 seconds following a clear message to prevent immediate re-infection 

Deliverables: Presentation summarizing design decisions, architecture and data flow of solution, demonstration of solution.
 
Learning objectives: Swarming, multihop, distributed algorithms, leader election. 
