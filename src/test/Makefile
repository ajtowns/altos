vpath % ..:../core:../drivers:../util

PROGS=ao_flight_test ao_flight_test_baro ao_flight_test_accel ao_flight_test_noisy_accel ao_gps_test ao_gps_test_skytraq ao_convert_test ao_convert_pa_test

KALMAN=make-kalman 

CFLAGS=-I.. -I. -I../core -I../drivers -O0 -g

all: $(PROGS)

clean:
	rm -f $(PROGS)

install:

ao_flight_test: ao_flight_test.c ao_host.h ao_flight_mm.c ao_sample_mm.c ao_kalman.c altitude.h ao_kalman.h
	cc $(CFLAGS) -o $@ $<

ao_flight_test_noisy_accel: ao_flight_test.c ao_host.h ao_flight_mm.c ao_sample_mm.c ao_kalman.c altitude.h ao_kalman.h
	cc -DNOISY_ACCEL=1 $(CFLAGS) -o $@ $<

ao_flight_test_baro: ao_flight_test.c ao_host.h ao_flight.c  ao_sample.c ao_kalman.c altitude.h ao_kalman.h
	cc $(CFLAGS) -o $@ -DHAS_ACCEL=0 ao_flight_test.c

ao_flight_test_accel: ao_flight_test.c ao_host.h ao_flight.c  ao_sample.c ao_kalman.c altitude.h ao_kalman.h
	cc $(CFLAGS) -o $@ -DFORCE_ACCEL=1 ao_flight_test.c

ao_gps_test: ao_gps_test.c ao_gps_sirf.c ao_gps_print.c ao_host.h
	cc $(CFLAGS) -o $@ $<

ao_gps_test_skytraq: ao_gps_test_skytraq.c ao_gps_skytraq.c ao_gps_print.c ao_host.h
	cc $(CFLAGS) -o $@ $<

ao_convert_test: ao_convert_test.c ao_convert.c altitude.h
	cc $(CFLAGS) -o $@ $<

ao_convert_pa_test: ao_convert_pa_test.c ao_convert_pa.c altitude-pa.h
	cc $(CFLAGS) -o $@ $<

ao_kalman.h: $(KALMAN)
	(cd .. && make ao_kalman.h)