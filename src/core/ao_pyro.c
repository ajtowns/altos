/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao.h>
#include <ao_pyro.h>
#include <ao_sample.h>
#include <ao_flight.h>

#if IS_COMPANION
#include <ao_companion.h>
#define ao_accel ao_companion_command.accel
#define ao_speed ao_companion_command.speed
#define ao_height ao_companion_command.height
#define ao_flight_state ao_companion_command.flight_state
#define ao_motor_number ao_companion_command.motor_number
#endif

#define ao_lowbit(x)	((x) & (-x))

/*
 * Given a pyro structure, figure out
 * if the current flight state satisfies all
 * of the requirements
 */
static uint8_t
ao_pyro_ready(struct ao_pyro *pyro)
{
	enum ao_pyro_flag flag, flags;

	flags = pyro->flags;
	while (flags != ao_pyro_none) {
		flag = ao_lowbit(flags);
		flags &= ~flag;
		switch (flag) {

		case ao_pyro_accel_less:
			if (ao_accel <= pyro->accel_less)
				continue;
			break;
		case ao_pyro_accel_greater:
			if (ao_accel >= pyro->accel_greater)
				continue;
			break;


		case ao_pyro_speed_less:
			if (ao_speed <= pyro->speed_less)
				continue;
			break;
		case ao_pyro_speed_greater:
			if (ao_speed >= pyro->speed_greater)
				continue;
			break;

		case ao_pyro_height_less:
			if (ao_height <= pyro->height_less)
				continue;
			break;
		case ao_pyro_height_greater:
			if (ao_height >= pyro->height_greater)
				continue;
			break;

#if HAS_ORIENT
		case ao_pyro_orient_less:
			if (ao_orient <= pyro->orient_less)
				continue;
			break;
		case ao_pyro_orient_greater:
			if (ao_orient >= pyro->orient_greater)
				continue;
			break;
#endif

		case ao_pyro_time_less:
			if ((int16_t) (ao_time() - ao_boost_tick) <= pyro->time_less)
				continue;
			break;
		case ao_pyro_time_greater:
			if ((int16_t) (ao_time() - ao_boost_tick) >= pyro->time_greater)
				continue;
			break;

		case ao_pyro_ascending:
			if (ao_speed > 0)
				continue;
			break;
		case ao_pyro_descending:
			if (ao_speed < 0)
				continue;
			break;

		case ao_pyro_after_motor:
			if (ao_motor_number == pyro->motor)
				continue;
			break;

		case ao_pyro_delay:
			/* handled separately */
			continue;

		case ao_pyro_state_less:
			if (ao_flight_state < pyro->state_less)
				continue;
			break;
		case ao_pyro_state_greater_or_equal:
			if (ao_flight_state >= pyro->state_greater_or_equal)
				continue;
			break;

		default:
			continue;
		}
		return FALSE;
	}
	return TRUE;
}

#define ao_pyro_fire_port(port, bit, pin) do {	\
		ao_gpio_set(port, bit, pin, 1);	\
		ao_delay(AO_MS_TO_TICKS(50));	\
		ao_gpio_set(port, bit, pin, 0);	\
	} while (0)


static void
ao_pyro_fire(uint8_t p)
{
	switch (p) {
#if AO_PYRO_NUM > 0
	case 0: ao_pyro_fire_port(AO_PYRO_PORT_0, AO_PYRO_PIN_0, AO_PYRO_0); break;
#endif
#if AO_PYRO_NUM > 1
	case 1: ao_pyro_fire_port(AO_PYRO_PORT_1, AO_PYRO_PIN_1, AO_PYRO_1); break;
#endif
#if AO_PYRO_NUM > 2
	case 2: ao_pyro_fire_port(AO_PYRO_PORT_2, AO_PYRO_PIN_2, AO_PYRO_2); break;
#endif
#if AO_PYRO_NUM > 3
	case 3: ao_pyro_fire_port(AO_PYRO_PORT_3, AO_PYRO_PIN_3, AO_PYRO_3); break;
#endif
#if AO_PYRO_NUM > 4
	case 4: ao_pyro_fire_port(AO_PYRO_PORT_4, AO_PYRO_PIN_4, AO_PYRO_4); break;
#endif
#if AO_PYRO_NUM > 5
	case 5: ao_pyro_fire_port(AO_PYRO_PORT_5, AO_PYRO_PIN_5, AO_PYRO_5); break;
#endif
#if AO_PYRO_NUM > 6
	case 6: ao_pyro_fire_port(AO_PYRO_PORT_6, AO_PYRO_PIN_6, AO_PYRO_6); break;
#endif
#if AO_PYRO_NUM > 7
	case 7: ao_pyro_fire_port(AO_PYRO_PORT_7, AO_PYRO_PIN_7, AO_PYRO_7); break;
#endif
	default: break;
	}
	ao_delay(AO_MS_TO_TICKS(50));
}

uint8_t	ao_pyro_wakeup;

static void
ao_pyro(void)
{
	uint8_t		p, any_waiting;
	struct ao_pyro	*pyro;

	ao_config_get();
	while (ao_flight_state < ao_flight_boost)
		ao_sleep(&ao_flight_state);

	for (;;) {
		ao_alarm(AO_MS_TO_TICKS(100));
		ao_sleep(&ao_pyro_wakeup);
		ao_clear_alarm();
		any_waiting = 0;
		for (p = 0; p < AO_PYRO_NUM; p++) {
			pyro = &ao_config.pyro[p];

			/* Ignore igniters which have already fired
			 */
			if (pyro->fired)
				continue;

			/* Ignore disabled igniters
			 */
			if (!pyro->flags)
				continue;

			any_waiting = 1;
			/* Check pyro state to see if it shoule fire
			 */
			if (!pyro->delay_done) {
				if (!ao_pyro_ready(pyro))
					continue;

				/* If there's a delay set, then remember when
				 * it expires
				 */
				if (pyro->flags & ao_pyro_delay)
					pyro->delay_done = ao_time() + pyro->delay;
			}

			/* Check to see if we're just waiting for
			 * the delay to expire
			 */
			if (pyro->delay_done) {
				if ((int16_t) (ao_time() - pyro->delay_done) < 0)
					continue;
			}

			ao_pyro_fire(p);
		}
		if (!any_waiting)
			break;
	}
	ao_exit();
}

__xdata struct ao_task ao_pyro_task;

#define NO_VALUE	0xff

#define AO_PYRO_NAME_LEN	3

#if !DISABLE_HELP
#define ENABLE_HELP 1
#endif

#if ENABLE_HELP
#define HELP(s)	(s)
#else
#define HELP(s)
#endif

const struct {
	char			name[AO_PYRO_NAME_LEN];
	enum ao_pyro_flag	flag;
	uint8_t			offset;
#if ENABLE_HELP
	char			*help;
#endif
} ao_pyro_values[] = {
	{ "a<",	ao_pyro_accel_less,	offsetof(struct ao_pyro, accel_less), HELP("accel less (m/ss * 16)") },
	{ "a>",	ao_pyro_accel_greater,	offsetof(struct ao_pyro, accel_greater), HELP("accel greater (m/ss * 16)") },

	{ "s<",	ao_pyro_speed_less,	offsetof(struct ao_pyro, speed_less), HELP("speed less (m/s * 16)") },
	{ "s>",	ao_pyro_speed_greater,	offsetof(struct ao_pyro, speed_greater), HELP("speed greater (m/s * 16)") },

	{ "h<",	ao_pyro_height_less,	offsetof(struct ao_pyro, height_less), HELP("height less (m)") },
	{ "h>",	ao_pyro_height_greater,	offsetof(struct ao_pyro, height_greater), HELP("height greater (m)") },

#if HAS_ORIENT
	{ "o<",	ao_pyro_orient_less,	offsetof(struct ao_pyro, orient_less), HELP("orient less (deg)") },
	{ "o>",	ao_pyro_orient_greater,	offsetof(struct ao_pyro, orient_greater), HELP("orient greater (deg)")  },
#endif

	{ "t<",	ao_pyro_time_less,	offsetof(struct ao_pyro, time_less), HELP("time less (s * 100)") },
	{ "t>",	ao_pyro_time_greater,	offsetof(struct ao_pyro, time_greater), HELP("time greater (s * 100)")  },

	{ "f<",	ao_pyro_state_less,	offsetof(struct ao_pyro, state_less), HELP("state less") },
	{ "f>=",ao_pyro_state_greater_or_equal,	offsetof(struct ao_pyro, state_greater_or_equal), HELP("state greater or equal")  },

	{ "A", ao_pyro_ascending,	NO_VALUE, HELP("ascending") },
	{ "D", ao_pyro_descending,	NO_VALUE, HELP("descending") },

	{ "m", ao_pyro_after_motor,	offsetof(struct ao_pyro, motor), HELP("after motor") },

	{ "d", ao_pyro_delay,		offsetof(struct ao_pyro, delay), HELP("delay before firing (s * 100)") },
	{ "", ao_pyro_none,		NO_VALUE, HELP(NULL) },
};

static void
ao_pyro_print_name(uint8_t v)
{
	const char *s = ao_pyro_values[v].name;
	printf ("%s%s", s, "   " + strlen(s));
}

#if ENABLE_HELP
static void
ao_pyro_help(void)
{
	uint8_t v;
	for (v = 0; ao_pyro_values[v].flag != ao_pyro_none; v++) {
		ao_pyro_print_name(v);
		if (ao_pyro_values[v].offset != NO_VALUE)
			printf ("<n> ");
		else
			printf ("    ");
		printf ("%s\n", ao_pyro_values[v].help);
	}
}
#endif

void
ao_pyro_show(void)
{
	uint8_t 	p;
	uint8_t 	v;
	struct ao_pyro	*pyro;

	printf ("Pyro-count: %d\n", AO_PYRO_NUM);
	for (p = 0; p < AO_PYRO_NUM; p++) {
		printf ("Pyro %2d: ", p);
		pyro = &ao_config.pyro[p];
		if (!pyro->flags) {
			printf ("<disabled>\n");
			continue;
		}
		for (v = 0; ao_pyro_values[v].flag != ao_pyro_none; v++) {
			if (!(pyro->flags & ao_pyro_values[v].flag))
				continue;
			ao_pyro_print_name(v);
			if (ao_pyro_values[v].offset != NO_VALUE) {
				int16_t	value;

				value = *((int16_t *) ((char *) pyro + ao_pyro_values[v].offset));
				printf ("%6d ", value);
			} else {
				printf ("       ");
			}
		}
		printf ("\n");
	}
}

void
ao_pyro_set(void)
{
	uint8_t	p;
	struct ao_pyro pyro_tmp;
	char	name[AO_PYRO_NAME_LEN];
	uint8_t	c;
	uint8_t	v;

	ao_cmd_white();

#if ENABLE_HELP
	switch (ao_cmd_lex_c) {
	case '?':
		ao_pyro_help();
		return;
	}
#endif

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	p = ao_cmd_lex_i;
	if (p < 0 || AO_PYRO_NUM <= p) {
		printf ("invalid pyro channel %d\n", p);
		return;
	}
	pyro_tmp.flags = 0;
	for (;;) {
		ao_cmd_white();
		if (ao_cmd_lex_c == '\n')
			break;

		for (c = 0; c < AO_PYRO_NAME_LEN - 1; c++) {
			if (ao_cmd_is_white())
				break;
			name[c] = ao_cmd_lex_c;
			ao_cmd_lex();
		}
		name[c] = '\0';
		for (v = 0; ao_pyro_values[v].flag != ao_pyro_none; v++) {
			if (!strcmp (ao_pyro_values[v].name, name))
				break;
		}
		if (ao_pyro_values[v].flag == ao_pyro_none) {
			printf ("invalid pyro field %s\n", name);
			ao_cmd_status = ao_cmd_syntax_error;
			return;
		}
		pyro_tmp.flags |= ao_pyro_values[v].flag;
		if (ao_pyro_values[v].offset != NO_VALUE) {
			ao_cmd_decimal();
			if (ao_cmd_status != ao_cmd_success)
				return;
			*((int16_t *) ((char *) &pyro_tmp + ao_pyro_values[v].offset)) = ao_cmd_lex_i;
		}
	}
	_ao_config_edit_start();
	ao_config.pyro[p] = pyro_tmp;
	_ao_config_edit_finish();
}

static void
ao_pyro_manual(void)
{
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	ao_cmd_white();
	ao_cmd_decimal();
	if (ao_cmd_lex_i < 0 || AO_PYRO_NUM <= ao_cmd_lex_i)
		return;
	ao_pyro_fire(ao_cmd_lex_i);

}

const struct ao_cmds ao_pyro_cmds[] = {
	{ ao_pyro_manual,	"P DoIt <n>\0Fire igniter" },
	{ 0, NULL }
};

void
ao_pyro_init(void)
{
#if AO_PYRO_NUM > 0
	ao_enable_output(AO_PYRO_PORT_0, AO_PYRO_PIN_0, AO_PYRO_0, 0);
#endif
#if AO_PYRO_NUM > 1
	ao_enable_output(AO_PYRO_PORT_1, AO_PYRO_PIN_1, AO_PYRO_1, 0);
#endif
#if AO_PYRO_NUM > 2
	ao_enable_output(AO_PYRO_PORT_2, AO_PYRO_PIN_2, AO_PYRO_2, 0);
#endif
#if AO_PYRO_NUM > 3
	ao_enable_output(AO_PYRO_PORT_3, AO_PYRO_PIN_3, AO_PYRO_3, 0);
#endif
#if AO_PYRO_NUM > 4
	ao_enable_output(AO_PYRO_PORT_4, AO_PYRO_PIN_4, AO_PYRO_4, 0);
#endif
#if AO_PYRO_NUM > 5
	ao_enable_output(AO_PYRO_PORT_5, AO_PYRO_PIN_5, AO_PYRO_5, 0);
#endif
#if AO_PYRO_NUM > 6
	ao_enable_output(AO_PYRO_PORT_6, AO_PYRO_PIN_6, AO_PYRO_6, 0);
#endif
#if AO_PYRO_NUM > 7
	ao_enable_output(AO_PYRO_PORT_7, AO_PYRO_PIN_7, AO_PYRO_7, 0);
#endif
	ao_cmd_register(&ao_pyro_cmds[0]);
	ao_add_task(&ao_pyro_task, ao_pyro, "pyro");
}
