/*
 * Copyright (c) 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Kungliga Tekniska
 *      Högskolan and its contributors.
 *
 * 4. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "utility.h"
#include <WinSock2.h>
#include <WS2tcpip.h>



#ifndef INADDRSZ
#   define INADDRSZ     sizeof(in_addr)
#endif 

#ifndef IN6ADDRSZ
#   define IN6ADDRSZ    sizeof(in6_addr)
#endif

#ifndef INT16SZ
#   define INT16SZ      sizeof(short)
#endif



BEGIN_NETWORK_NAMESPACE

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int      inet_pton4(const char* src, unsigned char* dst);
static int      inet_pton6(const char* src, unsigned char* dst);

/* int* inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *      Paul Vixie, 1996.
 */
int inet_aapton(int af, const char* src, void* dst)
{
    switch (af)
    {
        case AF_INET:
            return inet_pton4(src, (unsigned char*)dst);
        case AF_INET6:
            return inet_pton6(src, (unsigned char*)dst);
        default:
            /* errno = EAFNOSUPPORT; */
            return (-1);
    }
    /* NOTREACHED */
}

/* int inet_pton4(src, dst)
 *      like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *      1 if `src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton4(const char* src, unsigned char* dst)
{
    static const char digits[] = "0123456789";
    int saw_digit, octets, ch;
    unsigned char tmp[INADDRSZ], *tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0')
    {
        const char* pch;

        if ((pch = strchr(digits, ch)) != NULL)
        {
            unsigned int new_count = *tp * 10 + (pch - digits);

            if (new_count > 255)
                return (0);
            if (! saw_digit)
            {
                if (++octets > 4)
                    return (0);
                saw_digit = 1;
            }
            *tp = (unsigned char)new_count;
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return (0);
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return (0);
    }
    if (octets < 4)
        return (0);

    memcpy(dst, tmp, INADDRSZ);
    return (1);
}

/* int inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton6(const char* src, unsigned char* dst)
{
    static const char xdigits_l[] = "0123456789abcdef",
                                    xdigits_u[] = "0123456789ABCDEF";
    unsigned char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const char* xdigits, *curtok;
    int ch, saw_xdigit, count_xdigit;
    unsigned int val;

    memset((tp = tmp), '\0', IN6ADDRSZ);
    endp = tp + IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            return (0);
    curtok = src;
    saw_xdigit = count_xdigit = 0;
    val = 0;
    while ((ch = *src++) != '\0')
    {
        const char* pch;

        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = strchr((xdigits = xdigits_u), ch);
        if (pch != NULL)
        {
            if (count_xdigit >= 4)
                return (0);
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return (0);
            saw_xdigit = 1;
            count_xdigit++;
            continue;
        }
        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                    return (0);
                colonp = tp;
                continue;
            }
            else if (*src == '\0')
            {
                return (0);
            }
            if (tp + INT16SZ > endp)
                return (0);
            *tp++ = (unsigned char) (val >> 8) & 0xff;
            *tp++ = (unsigned char) val & 0xff;
            saw_xdigit = 0;
            count_xdigit = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
                inet_pton4(curtok, tp) > 0)
        {
            tp += INADDRSZ;
            saw_xdigit = 0;
            count_xdigit = 0;
            break;  /* '\0' was seen by inet_pton4(). */
        }
        return (0);
    }
    if (saw_xdigit)
    {
        if (tp + INT16SZ > endp)
            return (0);
        *tp++ = (unsigned char) (val >> 8) & 0xff;
        *tp++ = (unsigned char) val & 0xff;
    }
    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        for (i = 1; i <= n; i++)
        {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (0);
    memcpy(dst, tmp, IN6ADDRSZ);
    return (1);
}

END_NETWORK_NAMESPACE
