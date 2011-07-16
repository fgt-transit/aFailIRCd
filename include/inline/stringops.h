/*
 *  SporksIRCD: a useless IRC daemon
 *  inline/stringops.h: inlined string operations used in a few places
 *
 *  Copyright (c) 2010 Elizabeth Jennifer Myers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __INLINE_STRINGOPS_H
#define __INLINE_STRINGOPS_H

/* Strip colour codes from a string and put it in result
 *
 * Original may have been by asuffield, according to old comments.
 * Rewritten almost completely. The old code was a disaster and didn't handle
 * a few corner cases well. This code handles them better. --Elizabeth
 */
static inline void
strip_colour(const char *string, char *result, int len)
{
	const char *sptr;
	char *rptr = result;
	int hunt_colour = 0;
	int l = 0;

	/* Why - 1 on len? The null terminator! */
	for(sptr = string; *sptr != '\0' && l < (len - 1); l++, sptr++)
	{
		if(hunt_colour)
		{
			if(isdigit(*sptr) || (*sptr == ','))
				continue;
			else
				hunt_colour = 0;
		}

		switch(*sptr)
		{
		case 3:
			hunt_colour = 1;
			break;
		case 2:
		case 6:
		case 7:
		case 22:
		case 23:
		case 27:
		case 29:
		case 31:
			break;
		default:
			*(rptr++) = *sptr;
			break;
		}
	}

	*(rptr) = '\0';
}

/* This one was a total rewrite. The original came from ShadowIRCD and was
 * substantially derived from the above. Not anymore. It can be a lot simpler
 * since it strips all unprintables. --Elizabeth */
static inline void
strip_unprintable(const char *string, char *result, int len)
{
	const char *sptr;
	char *rptr = result;
	int hunt_colour = 0;
	int l = 0;

	for(sptr = string; *sptr != '\0' && l < (len - 1); sptr++, l++)
	{
		if(hunt_colour)
		{
			if(isdigit(*sptr) || (*sptr == ','))
				continue;
			else
				hunt_colour = 0;
		}

		if(*sptr == '\003')
		{
			hunt_colour = 1;
			continue;
		}
			
		if (*sptr >= 32)
			*(rptr++) = *sptr;
	}

	*(rptr) = '\0';
}

#endif
