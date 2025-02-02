/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  GMime
 *  Copyright (C) 2000-2022 Jeffrey Stedfast and Michael Zucchi
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "gmime-table-private.h"
#include "gmime-encodings.h"


#ifdef ENABLE_WARNINGS
#define w(x) x
#else
#define w(x)
#endif /* ENABLE_WARNINGS */

#define d(x)


/**
 * SECTION: gmime-encodings
 * @title: gmime-encodings
 * @short_description: MIME encoding functions
 * @see_also:
 *
 * Utility functions to encode or decode MIME
 * Content-Transfer-Encodings.
 **/


#define GMIME_UUENCODE_CHAR(c) ((c) ? (c) + ' ' : '`')
#define	GMIME_UUDECODE_CHAR(c) (((c) - ' ') & 077)

static char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char gmime_base64_rank[256] = {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
	 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255,
	255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

static unsigned char gmime_uu_rank[256] = {
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
};

static unsigned char tohex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


static gboolean
is (const char *str, const char *value, size_t n)
{
	return g_ascii_strncasecmp (str, value, n) == 0 &&
		(str[n] == '\0' || is_lwsp (str[n]));
}


/**
 * g_mime_content_encoding_from_string:
 * @str: a string representing a Content-Transfer-Encoding value
 *
 * Gets the appropriate #GMimeContentEncoding enumeration value based
 * on the input string.
 *
 * Returns: the #GMimeContentEncoding specified by @str or
 * #GMIME_CONTENT_ENCODING_DEFAULT on error.
 **/
GMimeContentEncoding
g_mime_content_encoding_from_string (const char *str)
{
	while (is_lwsp (*str))
		str++;
	
	if (is (str, "7bit", 4))
		return GMIME_CONTENT_ENCODING_7BIT;
	else if (is (str, "8bit", 4))
		return GMIME_CONTENT_ENCODING_8BIT;
	else if (is (str, "7-bit", 5))
		return GMIME_CONTENT_ENCODING_7BIT;
	else if (is (str, "8-bit", 5))
		return GMIME_CONTENT_ENCODING_8BIT;
	else if (is (str, "binary", 6))
		return GMIME_CONTENT_ENCODING_BINARY;
	else if (is (str, "base64", 6))
		return GMIME_CONTENT_ENCODING_BASE64;
	else if (is (str, "quoted-printable", 16))
		return GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE;
	else if (is (str, "uuencode", 8))
		return GMIME_CONTENT_ENCODING_UUENCODE;
	else if (is (str, "x-uuencode", 10))
		return GMIME_CONTENT_ENCODING_UUENCODE;
	else if (is (str, "x-uue", 5))
		return GMIME_CONTENT_ENCODING_UUENCODE;
	else
		return GMIME_CONTENT_ENCODING_DEFAULT;
}


/**
 * g_mime_content_encoding_to_string:
 * @encoding: a #GMimeContentEncoding
 *
 * Gets the string value of the content encoding.
 *
 * Returns: the encoding type as a string or %NULL on error. Available
 * values for the encoding are: #GMIME_CONTENT_ENCODING_DEFAULT,
 * #GMIME_CONTENT_ENCODING_7BIT, #GMIME_CONTENT_ENCODING_8BIT,
 * #GMIME_CONTENT_ENCODING_BINARY, #GMIME_CONTENT_ENCODING_BASE64,
 * #GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE and
 * #GMIME_CONTENT_ENCODING_UUENCODE.
 **/
const char *
g_mime_content_encoding_to_string (GMimeContentEncoding encoding)
{
	switch (encoding) {
	case GMIME_CONTENT_ENCODING_7BIT:
		return "7bit";
	case GMIME_CONTENT_ENCODING_8BIT:
		return "8bit";
	case GMIME_CONTENT_ENCODING_BINARY:
		return "binary";
	case GMIME_CONTENT_ENCODING_BASE64:
		return "base64";
	case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
		return "quoted-printable";
	case GMIME_CONTENT_ENCODING_UUENCODE:
		return "x-uuencode";
	default:
		/* I guess this is a good default... */
		return NULL;
	}
}


/**
 * g_mime_encoding_init_encode:
 * @state: a #GMimeEncoding to initialize
 * @encoding: a #GMimeContentEncoding to use
 *
 * Initializes a #GMimeEncoding state machine for encoding to
 * @encoding.
 **/
void
g_mime_encoding_init_encode (GMimeEncoding *state, GMimeContentEncoding encoding)
{
	state->encoding = encoding;
	state->encode = TRUE;
	
	g_mime_encoding_reset (state);
}


/**
 * g_mime_encoding_init_decode:
 * @state: a #GMimeEncoding to initialize
 * @encoding: a #GMimeContentEncoding to use
 *
 * Initializes a #GMimeEncoding state machine for decoding from
 * @encoding.
 **/
void
g_mime_encoding_init_decode (GMimeEncoding *state, GMimeContentEncoding encoding)
{
	state->encoding = encoding;
	state->encode = FALSE;
	
	g_mime_encoding_reset (state);
}


/**
 * g_mime_encoding_reset:
 * @state: a #GMimeEncoding to reset
 *
 * Resets the state of the #GMimeEncoding.
 **/
void
g_mime_encoding_reset (GMimeEncoding *state)
{
	if (state->encode) {
		if (state->encoding == GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE)
			state->state = -1;
		else
			state->state = 0;
	} else {
		state->state = 0;
	}
	
	state->save = 0;
}


/**
 * g_mime_encoding_outlen:
 * @state: a #GMimeEncoding
 * @inlen: an input length
 *
 * Given the input length, @inlen, calculate the needed output length
 * to perform an encoding or decoding step.
 *
 * Returns: the maximum number of bytes needed to encode or decode a
 * buffer of @inlen bytes.
 **/
size_t
g_mime_encoding_outlen (GMimeEncoding *state, size_t inlen)
{
	switch (state->encoding) {
	case GMIME_CONTENT_ENCODING_BASE64:
		if (state->encode)
			return GMIME_BASE64_ENCODE_LEN (inlen);
		else
			return inlen + 3;
	case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
		if (state->encode)
			return GMIME_QP_ENCODE_LEN (inlen);
		else
			return inlen + 2;
	case GMIME_CONTENT_ENCODING_UUENCODE:
		if (state->encode)
			return GMIME_UUENCODE_LEN (inlen);
		else
			return inlen + 3;
	default:
		return inlen;
	}
}


/**
 * g_mime_encoding_step:
 * @state: a #GMimeEncoding
 * @inbuf: an input buffer to encode or decode
 * @inlen: input buffer length
 * @outbuf: an output buffer
 *
 * Incrementally encodes or decodes (depending on @state) an input
 * stream by 'stepping' through a block of input at a time.
 *
 * You should make sure @outbuf is large enough by calling
 * g_mime_encoding_outlen() to find out how large @outbuf might need
 * to be.
 *
 * Returns: the number of bytes written to @outbuf.
 **/
size_t
g_mime_encoding_step (GMimeEncoding *state, const char *inbuf, size_t inlen, char *outbuf)
{
	const unsigned char *inptr = (const unsigned char *) inbuf;
	unsigned char *outptr = (unsigned char *) outbuf;
	
	switch (state->encoding) {
	case GMIME_CONTENT_ENCODING_BASE64:
		if (state->encode)
			return g_mime_encoding_base64_encode_step (inptr, inlen, outptr, &state->state, &state->save);
		else
			return g_mime_encoding_base64_decode_step (inptr, inlen, outptr, &state->state, &state->save);
	case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
		if (state->encode)
			return g_mime_encoding_quoted_encode_step (inptr, inlen, outptr, &state->state, &state->save);
		else
			return g_mime_encoding_quoted_decode_step (inptr, inlen, outptr, &state->state, &state->save);
	case GMIME_CONTENT_ENCODING_UUENCODE:
		if (state->encode)
			return g_mime_encoding_uuencode_step (inptr, inlen, outptr, state->uubuf, &state->state, &state->save);
		else
			return g_mime_encoding_uudecode_step (inptr, inlen, outptr, &state->state, &state->save);
	default:
		memcpy (outbuf, inbuf, inlen);
		return inlen;
	}
}


/**
 * g_mime_encoding_flush:
 * @state: a #GMimeEncoding
 * @inbuf: an input buffer to encode or decode
 * @inlen: input buffer length
 * @outbuf: an output buffer
 *
 * Completes the incremental encode or decode of the input stream (see
 * g_mime_encoding_step() for details).
 *
 * Returns: the number of bytes written to @outbuf.
 **/
size_t
g_mime_encoding_flush (GMimeEncoding *state, const char *inbuf, size_t inlen, char *outbuf)
{
	const unsigned char *inptr = (const unsigned char *) inbuf;
	unsigned char *outptr = (unsigned char *) outbuf;
	
	switch (state->encoding) {
	case GMIME_CONTENT_ENCODING_BASE64:
		if (state->encode)
			return g_mime_encoding_base64_encode_close (inptr, inlen, outptr, &state->state, &state->save);
		else
			return g_mime_encoding_base64_decode_step (inptr, inlen, outptr, &state->state, &state->save);
	case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
		if (state->encode)
			return g_mime_encoding_quoted_encode_close (inptr, inlen, outptr, &state->state, &state->save);
		else
			return g_mime_encoding_quoted_decode_step (inptr, inlen, outptr, &state->state, &state->save);
	case GMIME_CONTENT_ENCODING_UUENCODE:
		if (state->encode)
			return g_mime_encoding_uuencode_close (inptr, inlen, outptr, state->uubuf, &state->state, &state->save);
		else
			return g_mime_encoding_uudecode_step (inptr, inlen, outptr, &state->state, &state->save);
	default:
		memcpy (outbuf, inbuf, inlen);
		return inlen;
	}
}


/**
 * g_mime_encoding_base64_encode_close:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been encoded
 *
 * Base64 encodes the input stream to the output stream. Call this
 * when finished encoding data with g_mime_encoding_base64_encode_step()
 * to flush off the last little bit.
 *
 * Returns: the number of bytes encoded.
 **/
size_t
g_mime_encoding_base64_encode_close (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	register unsigned char *outptr = outbuf;
	register int quartets;
	unsigned char *saved;

	if (inlen > 0)
		outptr += g_mime_encoding_base64_encode_step (inbuf, inlen, outbuf, state, save);

	saved = (unsigned char *) save;
	quartets = *state;

	if (*saved > 0) {
		int c1 = saved[1];
		int c2 = saved[2];

		*outptr++ = base64_alphabet[c1 >> 2];
		*outptr++ = base64_alphabet[c2 >> 4 | ((c1 & 0x3) << 4)];
		if (*saved == 2)
			*outptr++ = base64_alphabet[(c2 & 0x0f) << 2];
		else
			*outptr++ = '=';
		*outptr++ = '=';
		quartets++;
	}

	if (quartets > 0)
		*outptr++ = '\n';

	*state = 0;
	*save = 0;

	return (size_t) (outptr - outbuf);
}


/**
 * g_mime_encoding_base64_encode_step:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been encoded
 *
 * Base64 encodes a chunk of data. Performs an 'encode step', only
 * encodes blocks of 3 characters to the output at a time, saves
 * left-over state in state and save (initialise to 0 on first
 * invocation).
 *
 * Returns: the number of bytes encoded.
 **/
size_t
g_mime_encoding_base64_encode_step (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	register const unsigned char *inptr;
	register unsigned char *outptr;
	register int quartets;
	unsigned char *saved;
	size_t remaining;

	if (inlen == 0)
		return 0;

	saved = (unsigned char *) save;
	quartets = *state;
	outptr = outbuf;
	inptr = inbuf;

	if (inlen + *saved > 2) {
		const unsigned char *inend = inbuf + inlen - 2;
		register int c1, c2, c3;

		c1 = *saved < 1 ? *inptr++ : saved[1];
		c2 = *saved < 2 ? *inptr++ : saved[2];
		c3 = *inptr++;

	  loop:
		/* encode our triplet into a quartet */
		*outptr++ = base64_alphabet[c1 >> 2];
		*outptr++ = base64_alphabet[(c2 >> 4) | ((c1 & 0x3) << 4)];
		*outptr++ = base64_alphabet[((c2 & 0x0f) << 2) | (c3 >> 6)];
		*outptr++ = base64_alphabet[c3 & 0x3f];

		/* encode 19 quartets per line */
		if ((++quartets) >= 19) {
			*outptr++ = '\n';
			quartets = 0;
		}

		if (inptr >= inend)
			goto loop_exit;

		c1 = *inptr++;
		c2 = *inptr++;
		c3 = *inptr++;
		goto loop;

	  loop_exit:
		remaining = 2 - (size_t) (inptr - inend);
		*save = 0;
	} else {
		remaining = inlen;
	}

	if (remaining > 0) {
		/* At this point, saved can only be 0 or 1. */
		if (*saved == 0) {
			/* We can have up to 2 remaining input bytes. */
			saved[0] = (unsigned char) remaining;
			saved[1] = *inptr++;
			if (remaining == 2)
				saved[2] = *inptr;
			else
				saved[2] = 0;
		} else {
			/* We have 1 remaining input byte. */
			saved[2] = *inptr;
			saved[0] = 2;
		}
	}

	*state = quartets;

	return (size_t) (outptr - outbuf);
}


/**
 * g_mime_encoding_base64_decode_step:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been decoded
 *
 * Decodes a chunk of base64 encoded data.
 *
 * Returns: the number of bytes decoded (which have been dumped in
 * @outbuf).
 **/
size_t
g_mime_encoding_base64_decode_step (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	register const unsigned char *inptr = inbuf;
	const unsigned char *inend = inptr + inlen;
	unsigned char *outptr = outbuf;
	unsigned int saved = *save;
	unsigned char c, rank;
	int n, eq, eof = 0;

	n = *state;

	/* if n == -1, then it means we've encountered the end of the base64 stream */
	if (n == -1)
		return 0;

	/* decode every quartet into a triplet */
	while (inptr < inend) {
		rank = gmime_base64_rank[(c = *inptr++)];

		if (rank != 0xFF) {
			saved = (saved << 6) | rank;
			n++;

			if (n == 4) {
				/* flush our decoded quartet */
				*outptr++ = saved >> 16;
				*outptr++ = saved >> 8;
				*outptr++ = saved;
				saved = 0;
				n = 0;
			}
		} else if (c == '=') {
			/* Note: this marks the end of the base64 stream. The only octet that can
			 * appear after this is another '=' (and possibly mailing-list junk). */
			eof = 1;
			break;
		}
	}

	if (eof) {
		/* Note: there shouldn't be more than 2 '=' octets at the end of a quartet,
		 * so if n < 2, then it means the encoder is broken. */
		if (n > 1) {
			/* at this point, n should be either 3 or 4 */
			eq = 4 - n;
			saved <<= (6 * eq);

			*outptr++ = saved >> 16;
			if (n > 2)
				*outptr++ = saved >> 8;
		}

		n = -1;
	}

	/* save state */
	*save = saved;
	*state = n;

	return (size_t) (outptr - outbuf);
}


/**
 * g_mime_encoding_uuencode_close:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @uubuf: temporary buffer of 60 bytes
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been encoded
 *
 * Uuencodes a chunk of data. Call this when finished encoding data
 * with g_mime_encoding_uuencode_step() to flush off the last little bit.
 *
 * Returns: the number of bytes encoded.
 **/
size_t
g_mime_encoding_uuencode_close (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, unsigned char *uubuf, int *state, guint32 *save)
{
	register unsigned char *outptr, *bufptr;
	register guint32 saved;
	int uulen, uufill, i;
	
	outptr = outbuf;
	
	if (inlen > 0)
		outptr += g_mime_encoding_uuencode_step (inbuf, inlen, outbuf, uubuf, state, save);
	
	uufill = 0;
	
	saved = *save;
	i = *state & 0xff;
	uulen = (*state >> 8) & 0xff;
	
	bufptr = uubuf + ((uulen / 3) * 4);
	
	if (i > 0) {
		while (i < 3) {
			saved <<= 8;
			uufill++;
			i++;
		}
		
		if (i == 3) {
			/* convert 3 normal bytes into 4 uuencoded bytes */
			unsigned char b0, b1, b2;
			
			b0 = (saved >> 16) & 0xff;
			b1 = (saved >> 8) & 0xff;
			b2 = saved & 0xff;
			
			*bufptr++ = GMIME_UUENCODE_CHAR ((b0 >> 2) & 0x3f);
			*bufptr++ = GMIME_UUENCODE_CHAR (((b0 << 4) | ((b1 >> 4) & 0xf)) & 0x3f);
			*bufptr++ = GMIME_UUENCODE_CHAR (((b1 << 2) | ((b2 >> 6) & 0x3)) & 0x3f);
			*bufptr++ = GMIME_UUENCODE_CHAR (b2 & 0x3f);
			
			uulen += 3;
		}
	}
	
	if (uulen > 0) {
		int cplen = ((uulen / 3) * 4);
		
		*outptr++ = GMIME_UUENCODE_CHAR ((uulen - uufill) & 0xff);
		memcpy (outptr, uubuf, cplen);
		outptr += cplen;
		*outptr++ = '\n';
		uulen = 0;
	}
	
	*outptr++ = GMIME_UUENCODE_CHAR (uulen & 0xff);
	*outptr++ = '\n';
	
	*save = 0;
	*state = 0;
	
	return (outptr - outbuf);
}


/**
 * g_mime_encoding_uuencode_step:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output stream
 * @uubuf: temporary buffer of 60 bytes
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been encoded
 *
 * Uuencodes a chunk of data. Performs an 'encode step', only encodes
 * blocks of 45 characters to the output at a time, saves left-over
 * state in @uubuf, @state and @save (initialize to 0 on first
 * invocation).
 *
 * Returns: the number of bytes encoded.
 **/
size_t
g_mime_encoding_uuencode_step (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, unsigned char *uubuf, int *state, guint32 *save)
{
	register unsigned char *outptr, *bufptr;
	register const unsigned char *inptr;
	const unsigned char *inend;
	unsigned char b0, b1, b2;
	guint32 saved;
	int uulen, i;
	
	if (inlen == 0)
		return 0;
	
	inend = inbuf + inlen;
	outptr = outbuf;
	inptr = inbuf;
	
	saved = *save;
	i = *state & 0xff;
	uulen = (*state >> 8) & 0xff;
	
	if ((inlen + i + uulen) < 45) {
		/* not enough input to write a full uuencoded line */
		bufptr = uubuf + ((uulen / 3) * 4);
	} else {
		bufptr = outptr + 1;
		
		if (uulen > 0) {
			/* copy the previous call's tmpbuf to outbuf */
			memcpy (bufptr, uubuf, ((uulen / 3) * 4));
			bufptr += ((uulen / 3) * 4);
		}
	}
	
	if (i == 2) {
		b0 = (saved >> 8) & 0xff;
		b1 = saved & 0xff;
		saved = 0;
		i = 0;
		
		goto skip2;
	} else if (i == 1) {
		if ((inptr + 2) < inend) {
			b0 = saved & 0xff;
			saved = 0;
			i = 0;
			
			goto skip1;
		}
		
		while (inptr < inend) {
			saved = (saved << 8) | *inptr++;
			i++;
		}
	}
	
	while (inptr < inend) {
		while (uulen < 45 && (inptr + 3) <= inend) {
			b0 = *inptr++;
		skip1:
			b1 = *inptr++;
		skip2:
			b2 = *inptr++;
			
			/* convert 3 normal bytes into 4 uuencoded bytes */
			*bufptr++ = GMIME_UUENCODE_CHAR ((b0 >> 2) & 0x3f);
			*bufptr++ = GMIME_UUENCODE_CHAR (((b0 << 4) | ((b1 >> 4) & 0xf)) & 0x3f);
			*bufptr++ = GMIME_UUENCODE_CHAR (((b1 << 2) | ((b2 >> 6) & 0x3)) & 0x3f);
			*bufptr++ = GMIME_UUENCODE_CHAR (b2 & 0x3f);
			
			uulen += 3;
		}
		
		if (uulen >= 45) {
			/* output the uu line length */
			*outptr = GMIME_UUENCODE_CHAR (uulen & 0xff);
			outptr += ((45 / 3) * 4) + 1;
			
			*outptr++ = '\n';
			uulen = 0;
			
			if ((inptr + 45) <= inend) {
				/* we have enough input to output another full line */
				bufptr = outptr + 1;
			} else {
				bufptr = uubuf;
			}
		} else {
			/* not enough input to continue... */
			for (i = 0, saved = 0; inptr < inend; i++)
				saved = (saved << 8) | *inptr++;
		}
	}
	
	*save = saved;
	*state = ((uulen & 0xff) << 8) | (i & 0xff);
	
	return (outptr - outbuf);
}


/**
 * g_mime_encoding_uudecode_step:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been decoded
 *
 * Uudecodes a chunk of data. Performs a 'decode step' on a chunk of
 * uuencoded data. Assumes the "begin mode filename" line has
 * been stripped off.
 *
 * Returns: the number of bytes decoded.
 **/
size_t
g_mime_encoding_uudecode_step (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	register const unsigned char *inptr;
	register unsigned char *outptr;
	const unsigned char *inend;
	unsigned char ch;
	register guint32 saved;
	gboolean last_was_eoln;
	int uulen, i;
	
	if (*state & GMIME_UUDECODE_STATE_END)
		return 0;
	
	saved = *save;
	i = *state & 0xff;
	uulen = (*state >> 8) & 0xff;
	if (uulen == 0)
		last_was_eoln = TRUE;
	else
		last_was_eoln = FALSE;
	
	inend = inbuf + inlen;
	outptr = outbuf;
	inptr = inbuf;
	
	while (inptr < inend) {
		if (*inptr == '\n') {
			last_was_eoln = TRUE;
			
			inptr++;
			continue;
		} else if (!uulen || last_was_eoln) {
			/* first octet on a line is the uulen octet */
			uulen = gmime_uu_rank[*inptr];
			last_was_eoln = FALSE;
			if (uulen == 0) {
				*state |= GMIME_UUDECODE_STATE_END;
				break;
			}
			
			inptr++;
			continue;
		}
		
		ch = *inptr++;
		
		if (uulen > 0) {
			/* save the byte */
			saved = (saved << 8) | ch;
			i++;
			if (i == 4) {
				/* convert 4 uuencoded bytes to 3 normal bytes */
				unsigned char b0, b1, b2, b3;
				
				b0 = saved >> 24;
				b1 = saved >> 16 & 0xff;
				b2 = saved >> 8 & 0xff;
				b3 = saved & 0xff;
				
				if (uulen >= 3) {
					*outptr++ = gmime_uu_rank[b0] << 2 | gmime_uu_rank[b1] >> 4;
					*outptr++ = gmime_uu_rank[b1] << 4 | gmime_uu_rank[b2] >> 2;
				        *outptr++ = gmime_uu_rank[b2] << 6 | gmime_uu_rank[b3];
					uulen -= 3;
				} else {
					*outptr++ = gmime_uu_rank[b0] << 2 | gmime_uu_rank[b1] >> 4;
					uulen--;
					
					if (uulen >= 1) {
						*outptr++ = gmime_uu_rank[b1] << 4 | gmime_uu_rank[b2] >> 2;
						uulen--;
					}
				}
				
				i = 0;
				saved = 0;
			}
		} else {
			break;
		}
	}
	
	*save = saved;
	*state = (*state & GMIME_UUDECODE_STATE_MASK) | ((uulen & 0xff) << 8) | (i & 0xff);
	
	return (outptr - outbuf);
}


/**
 * g_mime_encoding_quoted_encode_close:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been encoded
 *
 * Quoted-printable encodes a block of text. Call this when finished
 * encoding data with g_mime_encoding_quoted_encode_step() to flush off
 * the last little bit.
 *
 * Returns: the number of bytes encoded.
 **/
size_t
g_mime_encoding_quoted_encode_close (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	register unsigned char *outptr = outbuf;
	int last;
	
	if (inlen > 0)
		outptr += g_mime_encoding_quoted_encode_step (inbuf, inlen, outptr, state, save);
	
	last = *state;
	if (last != -1) {
		/* space/tab must be encoded if its the last character on the line */
		if (is_qpsafe (last) && !is_blank (last)) {
			*outptr++ = last;
		} else {
			*outptr++ = '=';
			*outptr++ = tohex[(last >> 4) & 0xf];
			*outptr++ = tohex[last & 0xf];
		}
		
		/* we end with =\n so that the \n isn't interpreted as a real
		   \n when it gets decoded later */
		*outptr++ = '=';
		*outptr++ = '\n';
		*state = -1;
	}
	
	*save = 0;
	
	return (outptr - outbuf);
}


/**
 * g_mime_encoding_quoted_encode_step:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been encoded
 *
 * Quoted-printable encodes a block of text. Performs an 'encode
 * step', saves left-over state in state and save (initialise to -1 on
 * first invocation).
 *
 * Returns: the number of bytes encoded.
 **/
size_t
g_mime_encoding_quoted_encode_step (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	register const unsigned char *inptr = inbuf;
	const unsigned char *inend = inbuf + inlen;
	register unsigned char *outptr = outbuf;
	register guint32 sofar = *save;  /* keeps track of how many chars on a line */
	register int last = *state;  /* keeps track if last char to end was a space cr etc */
	unsigned char c;
	
	while (inptr < inend) {
		c = *inptr++;
		if (c == '\r') {
			if (last != -1) {
				*outptr++ = '=';
				*outptr++ = tohex[(last >> 4) & 0xf];
				*outptr++ = tohex[last & 0xf];
				sofar += 3;
			}
			last = c;
		} else if (c == '\n') {
			if (last != -1 && last != '\r') {
				*outptr++ = '=';
				*outptr++ = tohex[(last >> 4) & 0xf];
				*outptr++ = tohex[last & 0xf];
			}
			*outptr++ = '\n';
			sofar = 0;
			last = -1;
		} else {
			if (last != -1) {
				if (is_qpsafe (last)) {
					*outptr++ = last;
					sofar++;
				} else {
					*outptr++ = '=';
					*outptr++ = tohex[(last >> 4) & 0xf];
					*outptr++ = tohex[last & 0xf];
					sofar += 3;
				}
			}
			
			if (is_qpsafe (c)) {
				if (sofar > 74) {
					*outptr++ = '=';
					*outptr++ = '\n';
					sofar = 0;
				}
				
				/* delay output of space char */
				if (is_blank (c)) {
					last = c;
				} else {
					*outptr++ = c;
					sofar++;
					last = -1;
				}
			} else {
				if (sofar > 72) {
					*outptr++ = '=';
					*outptr++ = '\n';
					sofar = 3;
				} else
					sofar += 3;
				
				*outptr++ = '=';
				*outptr++ = tohex[(c >> 4) & 0xf];
				*outptr++ = tohex[c & 0xf];
				last = -1;
			}
		}
	}
	
	*save = sofar;
	*state = last;
	
	return (outptr - outbuf);
}


/**
 * g_mime_encoding_quoted_decode_step:
 * @inbuf: input buffer
 * @inlen: input buffer length
 * @outbuf: output buffer
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been decoded
 *
 * Decodes a block of quoted-printable encoded data. Performs a
 * 'decode step' on a chunk of QP encoded data.
 *
 * Returns: the number of bytes decoded.
 **/
size_t
g_mime_encoding_quoted_decode_step (const unsigned char *inbuf, size_t inlen, unsigned char *outbuf, int *state, guint32 *save)
{
	/* FIXME: this does not strip trailing spaces from lines (as
	 * it should, rfc 2045, section 6.7) Should it also
	 * canonicalise the end of line to CR LF??
	 *
	 * Note: Trailing rubbish (at the end of input), like = or =x
	 * or =\r will be lost.
	 */
	const register unsigned char *inptr = inbuf;
	const unsigned char *inend = inbuf + inlen;
	register unsigned char *outptr = outbuf;
	guint32 isave = *save;
	int istate = *state;
	unsigned char c;
	
	d(printf ("quoted-printable, decoding text '%.*s'\n", inlen, inbuf));
	
	while (inptr < inend) {
		switch (istate) {
		case 0:
			while (inptr < inend) {
				c = *inptr++;
				/* FIXME: use a specials table to avoid 3 comparisons for the common case */
				if (c == '=') { 
					istate = 1;
					break;
				}
#ifdef CANONICALISE_EOL
				/*else if (c=='\r') {
					state = 3;
				} else if (c=='\n') {
					*outptr++ = '\r';
					*outptr++ = c;
					} */
#endif
				else {
					*outptr++ = c;
				}
			}
			break;
		case 1:
			c = *inptr++;
			if (c == '\n') {
				/* soft break ... unix end of line */
				istate = 0;
			} else {
				isave = c;
				istate = 2;
			}
			break;
		case 2:
			c = *inptr++;
			if (isxdigit (c) && isxdigit (isave)) {
				c = toupper ((int) c);
				isave = toupper ((int) isave);
				*outptr++ = (((isave >= 'A' ? isave - 'A' + 10 : isave - '0') & 0x0f) << 4)
					| ((c >= 'A' ? c - 'A' + 10 : c - '0') & 0x0f);
			} else if (c == '\n' && isave == '\r') {
				/* soft break ... canonical end of line */
			} else {
				/* just output the data */
				*outptr++ = '=';
				*outptr++ = isave;
				*outptr++ = c;
			}
			istate = 0;
			break;
#ifdef CANONICALISE_EOL
		case 3:
			/* convert \n -> to \r\n, leaves \r\n alone */
			c = *inptr++;
			if (c == '\n') {
				*outptr++ = '\r';
				*outptr++ = c;
			} else {
				*outptr++ = '\r';
				*outptr++ = '\n';
				*outptr++ = c;
			}
			istate = 0;
			break;
#endif
		}
	}
	
	*state = istate;
	*save = isave;
	
	return (outptr - outbuf);
}
