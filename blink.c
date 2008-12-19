
sfr at 0x80 P0;
sfr at 0x90 P1;
sfr at 0xA0 P2;

#define PERCFG	SFR(0xF1)
#define ADCCFG	SFR(0xF2)
#define P0SEL	SFR(0xF3)
#define P1SEL	SFR(0xF4)
#define P2SEL	SFR(0xF5)

sfr at 0xFD P0DIR;
sfr at 0xFE P1DIR;
sfr at 0xFF P2DIR;
sfr at 0x8F P0INP;
sfr at 0xF6 P1INP;
sfr at 0xF7 P2INP;

#define P0IFG	SFR(0x89)
#define P1IFG	SFR(0x8A)
#define P2IFG	SFR(0x8B)

#define nop()	_asm \
		nop \
		_endasm;

#if 0
void
delay (int n)
{
	int i, j, k;

	for (k = 0; k < n; k++) {
		for (j = 0; j < 50; j++)
			for (i = 0; i < 1000; i++)
				nop();
	}
}

void
dit() {
	P1 = 0xff;
	delay(1);
	P1 = 0xfd;
	delay(1);
}

void
dah () {
	P1 = 0xff;
	delay(3);
	P1 = 0xfd;
	delay(1);
}

void
charspace () {
	delay(2);
}

void
wordspace () {
	delay(8);
}

#define _ dit();
#define ___ dah();
#define C charspace();
#define W wordspace();

#endif

main ()
{
#if 0
	/* Set p1_1 to output */
	P1DIR = 0x02;
	P1INP = 0x00;
	P2INP = 0x00;
	for (;;) {
		___ _ ___ _ C ___ ___ _ ___ W	/* cq */
		___ _ _ C _ W			/* de */
		___ _ ___ C ___ _ _ C		/* kd */
		___ ___ _ _ _ C	_ _ _ C		/* 7s */
		___ ___ _ ___ C	___ ___ _ W	/* qg */
	}
#else
	P1DIR = 0x02;
	for (;;);
#endif
}
