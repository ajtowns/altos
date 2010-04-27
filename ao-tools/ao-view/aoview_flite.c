/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include <stdio.h>
#include <flite/flite.h>
#include "aoview.h"
#include <alsa/asoundlib.h>

cst_voice *register_cmu_us_kal16();
cst_voice *register_cmu_us_kal();

static cst_voice *voice;

static FILE *pipe_write;
static GThread *aoview_flite_thread;

static snd_pcm_t	*alsa_handle;

gpointer
aoview_flite_task(gpointer data)
{
	FILE		*input = data;
	char		line[1024];
	cst_wave	*wave;
	int		rate;
	int		channels;
	int		err;
	char		*samples;
	int		num_samples;

	err = snd_pcm_open(&alsa_handle, "default",
			   SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr, "alsa open failed %s\n",
			strerror(-err));
		alsa_handle = NULL;
	}
	rate = 0;
	channels = 0;
	while (fgets(line, sizeof (line) - 1, input) != NULL) {
		if (!alsa_handle)
			continue;
		wave = flite_text_to_wave(line, voice);
		if (wave->sample_rate != rate ||
		    wave->num_channels != channels)
		{
			rate = wave->sample_rate;
			channels = wave->num_channels;
			err = snd_pcm_set_params(alsa_handle,
						 SND_PCM_FORMAT_S16,
						 SND_PCM_ACCESS_RW_INTERLEAVED,
						 channels,
						 rate,
						 1,
						 100000);
			if (err < 0)
				fprintf(stderr, "alsa set_params error %s\n",
					strerror(-err));
		}
		err = snd_pcm_prepare(alsa_handle);
		if (err < 0)
			fprintf(stderr, "alsa pcm_prepare error %s\n",
				strerror(-err));
		samples = (char *) wave->samples;
		num_samples = wave->num_samples;
		while (num_samples > 0) {
			err = snd_pcm_writei(alsa_handle,
					     samples, num_samples);
			if (err <= 0) {
				fprintf(stderr, "alsa write error %s\n",
					strerror(-err));
				break;
			}
			num_samples -= err;
			samples += err * 2 * channels;
		}
		snd_pcm_drain(alsa_handle);
		delete_wave(wave);
	}
	snd_pcm_close(alsa_handle);
	alsa_handle = 0;
	return NULL;
}

void
aoview_flite_stop(void)
{
	int status;
	if (pipe_write) {
		fclose(pipe_write);
		pipe_write = NULL;
	}
	if (aoview_flite_thread) {
		g_thread_join(aoview_flite_thread);
		aoview_flite_thread = NULL;
	}
}

FILE *
aoview_flite_start(void)
{
	static once;
	int	p[2];
	GError	*error;
	FILE	*pipe_read;

	if (!once) {
		flite_init();
#if HAVE_REGISTER_CMU_US_KAL16
		voice = register_cmu_us_kal16();
#else
#if HAVE_REGISTER_CMU_US_KAL
		voice = register_cmu_us_kal();
#endif
#endif
		if (!voice) {
			perror("register voice");
			exit(1);
		}
	}
	aoview_flite_stop();
	pipe(p);
	pipe_read = fdopen(p[0], "r");
	pipe_write = fdopen(p[1], "w");
	g_thread_create(aoview_flite_task, pipe_read, TRUE, &error);
	return pipe_write;
}
