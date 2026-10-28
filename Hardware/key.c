#include "key.h"

uint8_t keyValue(void)
{
    return ( DL_GPIO_readPins(KEY_PORT,KEY_key_PIN) & KEY_key_PIN ) > 0 ? 0:1;
}

UserKeyState_t key_scan(uint16_t freq)
{
	static uint8_t last_state = 1;
	static uint8_t click_cnt = 0;
	static uint16_t gap_cnt = 0;
	static uint16_t press_cnt = 0;

	float Count_time = (((float)(1.0f/(float)freq))*1000.0f);
	uint8_t cur = keyValue();

	/* pressed */
	if (cur == 0) {
		press_cnt++;
		gap_cnt = 0;
	}

	/* long press (about 300ms) */
	if (press_cnt > (uint16_t)(300.0f / Count_time)) {
		press_cnt = 0;
		click_cnt = 0;
		gap_cnt = 0;
		last_state = cur;
		return USEKEY_long_click;
	}

	/* rising edge: press -> release */
	if (last_state == 0 && cur == 1) {
		click_cnt++;
		press_cnt = 0;
		gap_cnt = 0;
	}

	/* wait for multi-click window to finish (about 200ms after last release) */
	if (cur == 1 && click_cnt > 0) {
		gap_cnt++;
		if (gap_cnt > (uint16_t)(200.0f / Count_time)) {
			UserKeyState_t ret = USEKEY_stateless;
			if (click_cnt >= 3) {
				ret = USEKEY_triple_click;
			} else if (click_cnt == 2) {
				ret = USEKEY_double_click;
			} else {
				ret = USEKEY_single_click;
			}
			click_cnt = 0;
			gap_cnt = 0;
			last_state = cur;
			return ret;
		}
	}

	last_state = cur;
	return USEKEY_stateless;
}


