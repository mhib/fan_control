fan_control: fan_control.c
	gcc -O2 fan_control.c -o fan_control
clean:
	rm fan_control

all: fan_control
