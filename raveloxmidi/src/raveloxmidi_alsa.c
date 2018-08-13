/*
   This file is part of raveloxmidi.

   Copyright (C) 2018 Dave Kelly

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA 
*/
#include "config.h"

#ifdef HAVE_ALSA

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <asoundlib.h>

#include "raveloxmidi_alsa.h"
#include "utils.h"
#include "logging.h"

static snd_rawmidi_t *input_handle = NULL;
static snd_rawmidi_t *output_handle = NULL;

void raveloxmidi_alsa_list_rawmidi_devices( void )
{
	char **name_hints = NULL;
	char **n = NULL;
	int err = 0;

	err = snd_device_name_hint(-1, "rawmidi" , (void***)&name_hints);
	if (err != 0) return;

	fprintf(stderr, "Available ALSA devices:\n");
	n = name_hints;
	while (*n != NULL) {
		char *hint_string = NULL;

		hint_string = snd_device_name_get_hint(*n, "NAME");
		if (hint_string != NULL && 0 != strcmp("null", hint_string)) {
			fprintf( stderr, "\t%s\n", hint_string );
			free(hint_string);
		}

		hint_string = snd_device_name_get_hint(*n, "DESC");
		if (hint_string != NULL && 0 != strcmp("null", hint_string)) {
			fprintf( stderr, "\t%s\n", hint_string );
			free(hint_string);
		}

		hint_string = snd_device_name_get_hint(*n, "IOID");
		if (hint_string != NULL && 0 != strcmp("null", hint_string)) {
			fprintf( stderr, "\t%s\n", hint_string );
			free(hint_string);
		}

		n++;
	}//End of while
	fprintf( stderr, "\n");

	snd_device_name_free_hint((void**)name_hints);
}

int raveloxmidi_alsa_init( unsigned char *device_name )
{
	int ret = 0;
	if( ! device_name ) return -1;

	ret = snd_rawmidi_open( &input_handle, &output_handle, device_name, SND_RAWMIDI_APPEND | SND_RAWMIDI_NONBLOCK );
	logging_printf(LOGGING_DEBUG,"raveloxmidi_alsa_init: ret=%d %s\n", ret , snd_strerror( ret ));

	raveloxmidi_alsa_dump_rawmidi( input_handle );
	raveloxmidi_alsa_dump_rawmidi( output_handle );
	
	return ret;
}

void raveloxmidi_alsa_handle_destroy( snd_rawmidi_t *rawmidi )
{
	if( ! rawmidi ) return;

	snd_rawmidi_drain( rawmidi );
	snd_rawmidi_close( rawmidi );
}

void raveloxmidi_alsa_destroy( void )
{
	raveloxmidi_alsa_handle_destroy( input_handle );
	raveloxmidi_alsa_handle_destroy( output_handle );

	input_handle = NULL;
	output_handle = NULL;
}

void raveloxmidi_alsa_dump_rawmidi( snd_rawmidi_t *rawmidi )
{
	snd_rawmidi_info_t *info = NULL;
	int card_number = 0;
	unsigned int device_number = 0, rawmidi_flags = 0;
	const char *hw_id = NULL, *hw_driver_name = NULL, *handle_id = NULL;

	if( ! rawmidi ) return;

	snd_rawmidi_info_malloc( &info );
	snd_rawmidi_info( rawmidi, info );

	card_number = snd_rawmidi_info_get_card( info );
	device_number = snd_rawmidi_info_get_device( info );
	rawmidi_flags = snd_rawmidi_info_get_flags( info );
	hw_id = snd_rawmidi_info_get_id( info );
	hw_driver_name = snd_rawmidi_info_get_name( info );
	handle_id = snd_rawmidi_name( rawmidi );
	
	logging_printf(LOGGING_DEBUG, "rawmidi: handle=\"%s\" hw_id=\"%s\" hw_driver_name=\"%s\" flags=%u card=%d device=%u\n",
		handle_id, hw_id, hw_driver_name, rawmidi_flags, card_number, device_number );

	snd_rawmidi_info_free( info );
}

int raveloxmidi_alsa_output_available( void )
{
	return output_handle != NULL;
}

int raveloxmidi_alsa_output( unsigned char *buffer, size_t buffer_size )
{
	size_t bytes_written = 0;

	if( output_handle ) 
	{
		bytes_written = snd_rawmidi_write( output_handle, buffer, buffer_size );
		logging_printf(LOGGING_DEBUG,"raveloxmidi_alsa_output: bytes_written=%u\n", bytes_written );
	}

	return bytes_written;
}

#endif
