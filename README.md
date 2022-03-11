# Root_Commander
Root Commander is an Arduino MIDI controller that sends single notes. The scale or 'mode', key, and octave are selected with 3 potentiometers. Then the note or 'step' in the scale is sent out via MIDI when you press any of 7 normally open, momentary buttons. An OLED display is used to show the scale, key, octave, and step (shown in Roman Numerals). This code is set up for an Arduino Pro Micro. The SDA and SCL pins used for I2C OLED are different on different Arduino boards, so you may need to adjust your pins in the code.

MIDI.h and ss_oled libraries are required. You will need to download them if you don't already have them. Be sure they're stored in your Arduino Libraries folder.
