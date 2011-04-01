vpath % ..
vpath % ../kalman

PROGS=ao_flight_test ao_flight_test_baro ao_flight_test_accel ao_gps_test ao_gps_test_skytraq ao_convert_test

CFLAGS=-I.. -I.

all: $(PROGS)

clean:
	rm -f $(PROGS)

install:

ao_flight_test: ao_flight_test.c ao_host.h ao_flight.c ao_sample.c ao_kalman.c altitude.h ao_kalman.h
	cc -g -o $@ $<

ao_flight_test_baro: ao_flight_test.c ao_host.h ao_flight.c  ao_sample.c ao_kalman.c altitude.h ao_kalman.h
	cc -g -o $@ -DHAS_ACCEL=0 ../ao_flight_test.c

ao_flight_test_accel: ao_flight_test.c ao_host.h ao_flight.c  ao_sample.c ao_kalman.c altitude.h ao_kalman.h
	cc -g -o $@ -DFORCE_ACCEL=1 ../ao_flight_test.c

ao_gps_test: ao_gps_test.c ao_gps_sirf.c ao_gps_print.c ao_host.h
	cc -g -o $@ $<

ao_gps_test_skytraq: ao_gps_test_skytraq.c ao_gps_skytraq.c ao_gps_print.c ao_host.h
	cc -g -o $@ $<

ao_convert_test: ao_convert_test.c ao_convert.c altitude.h
	cc -g -o $@ $<

../ao_kalman.h: make-kalman kalman.5c kalman_filter.5c load_csv.5c matrix.5c
	sh $< > $@
