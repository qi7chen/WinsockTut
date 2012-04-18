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
#include <stdio.h>
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

const char* inet_ntop4(const unsigned char* src, char* dst, size_t size);
const char* inet_ntop6(const unsigned char* src, char* dst, size_t size);

/* char* inet_ntop(af, src, dst, size)
 *      convert a network format address to presentation format.
 * return:
 *      pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 *      Paul Vixie, 1996.
 */
const char* inet_ntop(int af, const void* src, char* dst, size_t size)
{
    switch (af)
    {
        case AF_INET:
            return inet_ntop4((unsigned char*)src, dst, size);

        case AF_INET6:
            return inet_ntop6((unsigned char*)src, dst, size);

        default:
            /* errno = EAFNOSUPPORT; */
            return (NULL);
    }
    /* NOTREACHED */
}

/* const char* inet_ntop4(src, dst, size)
 *      format an IPv4 address, more or less like inet_ntoa()
 * return:
 *      `dst' (as a const)
 * notes:
 *      (1) uses no statics
 *      (2) takes a u_char* not an in_addr as input
 * author:
 *      Paul Vixie, 1996.
 */
const char* inet_ntop4(const u_char* src, char* dst, size_t size)
{
    static const char fmt[] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];
    int l = sprintf_s(tmp, size, fmt, src[0], src[1], src[2], src[3]);
    if (l <= 0 || (size_t)l >= size)
    {
        return (NULL);
    }
    strcpy_s(dst, size, tmp);
    return (dst);
}

/* const char* inet_ntop6(src, dst, size)
 *      convert IPv6 binary address into presentation (printable) format
 * author:
 *      Paul Vixie, 1996.
 */
const char* inet_ntop6(const u_char* src, char* dst, size_t size)
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
    
    unsigned int words[IN6ADDRSZ / INT16SZ] = {};
    struct
    {
        int base, len;
    } best, cur;
    best.base = -1;
    best.len  = 0;
    cur.base = -1;
    cur.len  = 0;

    /*
     * Preprocess:
     *      Copy the input (bytewise) array into a wordwise array.
     *      Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    for (int i = 0; i < IN6ADDRSZ; i++)
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));

    for (int i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        }
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /*
     * Format the result.
     */
    char* tp = tmp;
    char* ep = tmp + sizeof(tmp);
    for (int i = 0; i < (IN6ADDRSZ / INT16SZ) && tp < ep; i++)
    {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
                i < (best.base + best.len))
        {
            if (i == best.base)
            {
                if (tp + 1 >= ep)
                    return (NULL);
                *tp++ = ':';
            }
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
        {
            if (tp + 1 >= ep)
                return (NULL);
            *tp++ = ':';
        }
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 &&
                (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
        {
            if (!inet_ntop4(src + 12, tp, (size_t)(ep - tp)))
                return (NULL);
            tp += strlen(tp);
            break;
        }
        int advance = sprintf_s(tp, ep - tp, "%x", words[i]);
        if (advance <= 0 || advance >= ep - tp)
            return (NULL);
        tp += advance;
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
    {
        if (tp + 1 >= ep)
            return (NULL);
        *tp++ = ':';
    }
    if (tp + 1 >= ep)
        return (NULL);
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((size_t)(tp - tmp) > size)
    {
        return (NULL);
    }
    strcpy_s(dst, size, tmp);
    return (dst);
}

END_NETWORK_NAMESPACE
