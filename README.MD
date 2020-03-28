# Fan control
A simple daemon for Raspberry PI, that toggles fan depending on current temperature.

## Usage
Just run 

```fan_control```

## Pins
Transistor's gate should be connected to Raspberry's PIN 18(GPIO 24).

Raspberry's 5/3.3V power PINs can be used as a power source.

## Acknowledgements
GPIO-related code copied from https://github.com/tranter/blogs/blob/master/gpio/part5/demo1.c