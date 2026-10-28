#include "led.h"

static volatile LedMode_t s_ledMode = LED_MODE_NORMAL;
static volatile uint16_t s_pulseTicks = 0;

void LED_SetMode(LedMode_t mode)
{
	s_ledMode = mode;
}

void LED_PulseOn(uint16_t duration_ticks)
{
	if (duration_ticks == 0U) {
		duration_ticks = 1U;
	}
	if (duration_ticks > s_pulseTicks) {
		s_pulseTicks = duration_ticks;
	}
}

void LED_Task(void)
{
	static uint16_t tick = 0;
	uint16_t period = 100U;

	/* Overlay pulse has highest priority */
	if (s_pulseTicks > 0U) {
		s_pulseTicks--;
		LED_ON();
		return;
	}

	if (s_ledMode == LED_MODE_ODOM_OFF) {
		LED_ON();
		return;
	}

	if (s_ledMode == LED_MODE_AVOID) {
		period = 20U; /* 20 * 5ms = 100ms toggle */
	} else {
		period = 100U; /* 100 * 5ms = 500ms toggle */
	}

	if (++tick >= period) {
		tick = 0;
		LED_Toggle();
	}
}

void LED_ON(void)
{
	DL_GPIO_clearPins(LED_PORT,LED_led_PIN);
}

void LED_OFF(void)
{
	DL_GPIO_setPins(LED_PORT,LED_led_PIN);
}

void LED_Toggle(void)
{
	DL_GPIO_togglePins(LED_PORT,LED_led_PIN);
}

void LED_Flash(uint16_t time)
{
	static uint16_t temp;
	if(time==0) LED_ON();
	else if(++temp==time) LED_Toggle(),temp=0;
}



