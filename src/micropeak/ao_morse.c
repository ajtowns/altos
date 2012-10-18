static void
pause(uint8_t	j)
{
	int64_t	i;

	while (j--) {
		for (i = 0; i < 2000; i++)
			ao_arch_nop();
	}
}

#define BIT(i,x)    	   ((x) ? (1 << (i)) : 0)
#define MORSE1(a)          (1 | BIT(3,a))
#define MORSE2(a,b)        (2 | BIT(3,a) | BIT(4,b))
#define MORSE3(a,b,c)      (3 | BIT(3,a) | BIT(4,b) | BIT(5,c))
#define MORSE4(a,b,c,d)    (4 | BIT(3,a) | BIT(4,b) | BIT(5,c) | BIT(6,d))
#define MORSE5(a,b,c,d,e)  (5 | BIT(3,a) | BIT(4,b) | BIT(5,c) | BIT(6,d) | BIT(7,e))

#define ___	1
#define _	0

static const uint8_t	morse[26] = {
	MORSE2(0,1),		/* A */
	MORSE4(1,0,0,0),	/* B */
	MORSE4(1,0,1,0),	/* C */
	MORSE3(1,0,0),		/* D */
	MORSE1(0),		/* E */
	MORSE4(0,0,1,0),	/* F */
	MORSE3(1,1,0),		/* G */
	MORSE4(0,0,0,0),	/* H */
	MORSE2(0,0),		/* I */
	MORSE4(0,1,1,1),	/* J */
	MORSE3(1,0,1),		/* K */
	MORSE4(0,1,0,0),	/* L */
	MORSE2(1,1),		/* M */
	MORSE2(1,1),		/* N */
	MORSE3(1,1,1),		/* O */
	MORSE4(0,1,1,0),	/* P */
	MORSE4(1,1,0,1),	/* Q */
	MORSE3(0,1,0),		/* R */
	MORSE3(0,0,0),		/* S */
	MORSE1(1),		/* T */
	MORSE3(0,0,1),		/* U */
	MORSE4(0,0,0,1),	/* V */
	MORSE3(0,1,1),		/* W */
	MORSE4(1,0,0,1),	/* X */
	MORSE4(1,0,1,1),	/* Y */
	MORSE4(1,1,0,0),	/* Z */
};

static void
on(void)
{
	PORTB |= (1 << 4);
}

static void
off(void)
{
	PORTB &= ~(1 << 4);
}

static void
morse_char (char c)
{
	uint8_t r = morse[c - 'a'];
	uint8_t l = r & 7;

	if (!r)
		return;
	while (l--) {
		on();
		if (r & 8)
			pause(3);
		else
			pause(1);
		off();
		pause(1);
		r >>= 1;
	}
	pause(2);
}

static void
morse_string(char *s) {
	char	c;

	while ((c = *s++)) {
		if (c == ' ')
			pause(5);
		else
			morse_char(c);
	}
}

