
# DIY Reflow oven controller

This code controls a 800W oven using a Solid-State-Relay. 
The loop is closed by using a K-type controller which outputs 40uV/C. 
The small signal is amplified by a TL-082 op-amp and then read by the arduino's analog input. Depending on the temperature measured the arduino will try to follow the desired temperature profile.

