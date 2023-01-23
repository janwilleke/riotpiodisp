#include "periph_conf.h"
#include "periph/gpio.h"
#include "pio/pio.h"

#include "disp.pio.h"

#define PIOFREQ 80000000
uint32_t frame[9610]; //to big because bug

int disp_send(void)
{
	pio_sm_transmit_word_block(0, 0, 0xaa55aa55);
	return 0;
}
extern void start_dma(char *data, int size);

int disp_start(void)
{
	int i;
	for (i = 0; i < 9600; i++)
		frame[i] = i + 0xfa000000;
	printf("start DMA\n");

	start_dma((char*)frame, 9600);
	return 0;
}

static void pio_init_pins(pio_t pio, gpio_t pin)
{
	(void)pio;
    const gpio_pad_ctrl_t pad_ctrl = {
        .drive_strength = DRIVE_STRENGTH_12MA,
	.input_enable = 0,

    };
    const gpio_io_ctrl_t io_ctrl = {
        .function_select = FUNCTION_SELECT_PIO0,
    };

    gpio_set_pad_config(pin, pad_ctrl);
    gpio_set_io_config(pin, io_ctrl);
}

static int pio_prog(int pio, int sm, pio_instr_t *instr,
		     pio_program_conf_t *conf,
		     pio_program_t *pro)
{
	int ret;

	if ((ret = pio_alloc_program(pio, pro))){
		printf("alloc failed\n");
		return ret;
	}
	if ((ret = pio_write_program(pio, pro, instr))) {
		printf("write failed\n");
		return ret;
	}
	if ((ret = pio_sm_init_common(pio, sm, pro, conf))) {
		printf("init_common failed\n");
		return ret;
	}
	return 0;
}

static int disp_prog(int pio, int sm)
{
	pio_instr_t instr[] = PIO_DISP_PROGRAM;
	pio_program_conf_t conf = PIO_DISP_PROGRAM_CONF;
	pio_program_t pro = pio_disp_create_program();

	return pio_prog(pio, sm, instr, &conf, &pro);
}

static int disp_prog_frame(int pio, int sm)
{
	pio_instr_t instr[] = PIO_FRAME_PROGRAM;
	pio_program_conf_t conf = PIO_FRAME_PROGRAM_CONF;
	pio_program_t pro = pio_frame_create_program();

	return pio_prog(pio, sm, instr, &conf, &pro);
}

int disp_init(void)
{
    int ret;
    pio_t pio = 0;
    pio_sm_t sm = 0;
    int i;

    ret = disp_prog(0,0);
    printf("init main prog %d\n", ret);
    disp_prog_frame(0,1);
    printf("init frame prog %d\n", ret);

    for (i = 0; i< 8; i++)
	    pio_init_pins(pio, 15 + i);

    pio_init_pins(pio, 27);
    pio_init_pins(pio, 6);
    pio_init_pins(pio, 26); /* frame */

    printf("gpio done\n");


    pio_sm_set_out_shift(pio, sm, true, false, 32);
    //    pio_sm_set_in_shift(pio, sm, false, true, 8);
    //    pio_sm_set_jmp_pin(pio, sm, sda);
    printf("shift done\n");

    printf("exec\n");
    /* this part is bad and hard to read / todo
       to get the gpios used by the pio as outputs,
       they must be initialized like this.
       This should be part of the pio framework driver,
       as every user as todo this.
       original in german:
       der teil ist unentlich daemlisch und umstaendlich...
       um die gpios der pio in out zustand zu gekommen
       werden die set_pins configuret, und dann dise setpins eingestellt
       immer maximal 5 pins auf einmal
       wenn alles aufgesetzt ist werden die eigentlich pins gesetzt.
       maximale scheise das sollte wegabstahiert werden wie in micropython
       das schlechtes ist das pio programm kann das qusi garnicht selbst,
       weil es seine set pins ja nicht ändern kann
       -> der preis um pio zu nutzen ist gross - lohnt sich aber dann auch ;-)
    */
    pio_sm_set_set_pins(pio, sm, 15, 4);
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINDIRS, 0xf));
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINS, 0xa));
    pio_sm_set_set_pins(pio, sm, 19, 4);
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINDIRS, 0xf));
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINS, 0xa));
    pio_sm_set_set_pins(pio, sm, 27, 1);
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINDIRS, 0x1));
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINS, 0x1));
    pio_sm_set_set_pins(pio, sm, 6, 1);
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINDIRS, 0x1));
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINS, 0x1));

    /* set frame pin low */
    pio_sm_set_set_pins(pio, sm, 26, 1);
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINDIRS, 0x1));
    pio_sm_exec_block(pio, sm, pio_inst_set(PIO_INST_SET_DST_PINS, 0x0));


    pio_sm_set_clkdiv(pio, sm, pio_sm_clkdiv(PIOFREQ));

    printf("start\n");
    pio_sm_set_out_pins(pio, sm, 15, 8);
    printf("out done\n");

    pio_sm_set_set_pins(pio, sm, 27, 1);
    printf("set done\n");

    pio_sm_set_sideset_pins(pio, sm, 6);
    printf("side done\n");
    pio_sm_start(0, 0);

    pio_sm_set_set_pins(0, 1, 26, 1);
    pio_sm_set_clkdiv(0, 1, pio_sm_clkdiv(PIOFREQ));

    pio_sm_start(0, 1);

    return 0;
}
