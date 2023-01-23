#include "periph_conf.h"
#include "disp.h"

static void pwm_init_pins(gpio_t pin)
{
	const gpio_pad_ctrl_t pad_ctrl = {
					  .drive_strength = DRIVE_STRENGTH_12MA,
					  .input_enable = 0,

	};
	const gpio_io_ctrl_t io_ctrl = {
					.function_select = FUNCTION_SELECT_PWM,
	};

	gpio_set_pad_config(pin, pad_ctrl);
	gpio_set_io_config(pin, io_ctrl);
}

int start_pwm(void)
{
	// reg values stolen form micropython
	pwm_init_pins(2);
	PWM->CH1_DIV.reg = 16;
	PWM->CH1_TOP.reg = 3124;
	PWM->CH1_CC.reg = 230; //smaller less power bigger more
	PWM->CH1_CSR.reg = 1;
	return 0;
}

int setpwm(int val)
{
	if (val < 400)
		PWM->CH1_CC.reg = val;
	return 0;
}
