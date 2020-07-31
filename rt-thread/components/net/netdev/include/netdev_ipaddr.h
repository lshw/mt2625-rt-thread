/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-18     ChenYong     First version
 */

#ifndef __NETDEV_IPADDR_H__
#define __NETDEV_IPADDR_H__

#include <stdint.h>
#include <rtconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Only supports the IPV4 protocol */
#ifndef NETDEV_IPV4
#define NETDEV_IPV4           1
#endif

#ifndef NETDEV_IPV6
#define NETDEV_IPV6           0
#endif

/* If your port already typedef's in_addr_t, define IN_ADDR_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(in_addr_t) && !defined(IN_ADDR_T_DEFINED)
typedef uint32_t in_addr_t;
#endif


#ifdef NETDEV_IPV4

struct in_addr
{
    in_addr_t s_addr;
};

typedef struct ip4_addr
{
    uint32_t addr;
} ip4_addr_t;

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

#define htons(x) (uint16_t)PP_HTONS(x)
#define ntohs(x) (uint16_t)PP_NTOHS(x)
#define htonl(x) (uint32_t)PP_HTONL(x)
#define ntohl(x) (uint32_t)PP_NTOHL(x)

/** 255.255.255.255 */
#define IPADDR_NONE         ((uint32_t)0xffffffffUL)
/** 127.0.0.1 */
#define IPADDR_LOOPBACK     ((uint32_t)0x7f000001UL)
/** 0.0.0.0 */
#define IPADDR_ANY          ((uint32_t)0x00000000UL)
/** 255.255.255.255 */
#define IPADDR_BROADCAST    ((uint32_t)0xffffffffUL)

/** 255.255.255.255 */
#define INADDR_NONE         IPADDR_NONE
/** 127.0.0.1 */
#define INADDR_LOOPBACK     IPADDR_LOOPBACK
/** 0.0.0.0 */
#define INADDR_ANY          IPADDR_ANY
/** 255.255.255.255 */
#define INADDR_BROADCAST    IPADDR_BROADCAST

#define IPADDR_BROADCAST_STRING "255.255.255.255"

/** IPv4 only: set the IP address given as an u32_t */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32_t */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

#define IP4ADDR_STRLEN_MAX  16

/** Copy IP address - faster than ip4_addr_set: no NULL check */
#define ip4_addr_copy(dest, src)            ((dest).addr = (src).addr)
#define ip4_addr_cmp(addr1, addr2)          ((addr1)->addr == (addr2)->addr)
/** Set complete address to zero */
#define ip4_addr_set_zero(ipaddr)           ((ipaddr)->addr = 0)
#define ip4_addr_isany_val(ipaddr)          ((ipaddr).addr == IPADDR_ANY)
#define ip4_addr_isany(ipaddr)              ((ipaddr) == NULL || ip4_addr_isany_val(*(ipaddr)))

in_addr_t netdev_ipaddr_addr(const char *cp);
int netdev_ip4addr_aton(const char *cp, ip4_addr_t *addr);
char *netdev_ip4addr_ntoa(const ip4_addr_t *addr);
char *netdev_ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen);

#define inet_addr(cp)                       netdev_ipaddr_addr(cp)
#define inet_aton(cp, addr)                 netdev_ip4addr_aton(cp,(ip4_addr_t*)addr)
#define inet_ntoa(addr)                     netdev_ip4addr_ntoa((const ip4_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen)      netdev_ip4addr_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)

#endif /* NETDEV_IPV4 */

#if NETDEV_IPV6

struct in6_addr
{
    union
    {
        uint32_t u32_addr[4];
        uint8_t u8_addr[16];
    } un;
#define s6_addr  un.u8_addr
};

typedef struct ip6_addr
{
    uint32_t addr[4];
} ip6_addr_t;

/** Copy IPv6 address - faster than ip6_addr_set: no NULL check */
#define ip6_addr_copy(dest, src) do{(dest).addr[0] = (src).addr[0]; \
                                    (dest).addr[1] = (src).addr[1]; \
                                    (dest).addr[2] = (src).addr[2]; \
                                    (dest).addr[3] = (src).addr[3];}while(0)

#define ip6_addr_cmp(addr1, addr2) (((addr1)->addr[0] == (addr2)->addr[0]) && \
                                    ((addr1)->addr[1] == (addr2)->addr[1]) && \
                                    ((addr1)->addr[2] == (addr2)->addr[2]) && \
                                    ((addr1)->addr[3] == (addr2)->addr[3]))

/** Set complete address to zero */
#define ip6_addr_set_zero(ip6addr)    do{(ip6addr)->addr[0] = 0; \
                                         (ip6addr)->addr[1] = 0; \
                                         (ip6addr)->addr[2] = 0; \
                                         (ip6addr)->addr[3] = 0;}while(0)

/** Safely copy one IPv6 address to another (src may be NULL) */
#define ip6_addr_set(dest, src) do{(dest)->addr[0] = (src) == NULL ? 0 : (src)->addr[0]; \
                                   (dest)->addr[1] = (src) == NULL ? 0 : (src)->addr[1]; \
                                   (dest)->addr[2] = (src) == NULL ? 0 : (src)->addr[2]; \
                                   (dest)->addr[3] = (src) == NULL ? 0 : (src)->addr[3];}while(0)

/** Set address to ipv6 'any' (no need for htonl()) */
#define ip6_addr_set_any(ip6addr)       ip6_addr_set_zero(ip6addr)
#define ip6_addr_isany_val(ip6addr) (((ip6addr).addr[0] == 0) && \
                                     ((ip6addr).addr[1] == 0) && \
                                     ((ip6addr).addr[2] == 0) && \
                                     ((ip6addr).addr[3] == 0))
#define ip6_addr_isany(ip6addr) (((ip6addr) == NULL) || ip6_addr_isany_val(*(ip6addr)))

/* directly map this to the lwip internal functions */
#define inet6_aton(cp, addr)            // TODO ip6addr_aton(cp, (ip6_addr_t*)addr)
#define inet6_ntoa(addr)                // TODO ip6addr_ntoa((const ip6_addr_t*)&(addr))
#define inet6_ntoa_r(addr, buf, buflen) // TODO ip6addr_ntoa_r((const ip6_addr_t*)&(addr), buf, buflen)

#endif

#if NETDEV_IPV4 && NETDEV_IPV6

/* IP address types for use in ip_addr_t.type member */
enum netdev_ip_addr_type
{
    /** IPv4 */
    IPADDR_TYPE_V4 = 0U,
    /** IPv6 */
    IPADDR_TYPE_V6 = 6U,
    /** IPv4+IPv6 ("dual-stack") */
    IPADDR_TYPE_ANY = 46U
};

/* A union struct for both IP version's addresses */
typedef struct _ip_addr
{
    union
    {
        ip6_addr_t ip6;
        ip4_addr_t ip4;
    } u_addr;
    /** @ref netdev_ip_addr_type */
    uint8_t type;
} ip_addr_t;

#define IP_SET_TYPE_VAL(ipaddr, iptype) do { (ipaddr).type = (iptype); }while(0)
#define IP_SET_TYPE(ipaddr, iptype)     do { if((ipaddr) != NULL) { IP_SET_TYPE_VAL(*(ipaddr), iptype); }}while(0)
#define IP_GET_TYPE(ipaddr)           ((ipaddr)->type)

/** @ingroup ipaddr */
#define IP_IS_ANY_TYPE_VAL(ipaddr)    (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_ANY)
/** @ingroup ipaddr */
#define IPADDR_ANY_TYPE_INIT          { { { { 0ul, 0ul, 0ul, 0ul } } }, IPADDR_TYPE_ANY }

/** @ingroup ip4addr */
#define IP_IS_V4_VAL(ipaddr)          (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_V4)
/** @ingroup ip6addr */
#define IP_IS_V6_VAL(ipaddr)          (IP_GET_TYPE(&ipaddr) == IPADDR_TYPE_V6)
/** @ingroup ip4addr */
#define IP_IS_V4(ipaddr)              (((ipaddr) == NULL) || IP_IS_V4_VAL(*(ipaddr)))
/** @ingroup ip6addr */
#define IP_IS_V6(ipaddr)              (((ipaddr) != NULL) && IP_IS_V6_VAL(*(ipaddr)))

/** @ingroup ip6addr
 * Convert generic ip address to specific protocol version
 */
#define ip_2_ip6(ipaddr)   (&((ipaddr)->u_addr.ip6))
/** @ingroup ip4addr
 * Convert generic ip address to specific protocol version
 */
#define ip_2_ip4(ipaddr)   (&((ipaddr)->u_addr.ip4))

/** @ingroup ip4addr */
#define IP_ADDR4(ipaddr,a,b,c,d)      do { IP4_ADDR(ip_2_ip4(ipaddr),a,b,c,d); \
                                           IP_SET_TYPE_VAL(*(ipaddr), IPADDR_TYPE_V4); } while(0)
/** @ingroup ip6addr */
#define IP_ADDR6(ipaddr,i0,i1,i2,i3)  do { IP6_ADDR(ip_2_ip6(ipaddr),i0,i1,i2,i3); \
                                           IP_SET_TYPE_VAL(*(ipaddr), IPADDR_TYPE_V6); } while(0)
/** @ingroup ip6addr */
#define IP_ADDR6_HOST(ipaddr,i0,i1,i2,i3)  IP_ADDR6(ipaddr,PP_HTONL(i0),PP_HTONL(i1),PP_HTONL(i2),PP_HTONL(i3))

/** @ingroup ipaddr */
#define ip_addr_copy(dest, src)      do{ IP_SET_TYPE_VAL(dest, IP_GET_TYPE(&src)); if(IP_IS_V6_VAL(src)){ \
  ip6_addr_copy(*ip_2_ip6(&(dest)), *ip_2_ip6(&(src))); }else{ \
  ip4_addr_copy(*ip_2_ip4(&(dest)), *ip_2_ip4(&(src))); }}while(0)
/** @ingroup ip6addr */
#define ip_addr_copy_from_ip6(dest, src)      do{ \
  ip6_addr_copy(*ip_2_ip6(&(dest)), src); IP_SET_TYPE_VAL(dest, IPADDR_TYPE_V6); }while(0)
/** @ingroup ip4addr */
#define ip_addr_copy_from_ip4(dest, src)      do{ \
  ip4_addr_copy(*ip_2_ip4(&(dest)), src); IP_SET_TYPE_VAL(dest, IPADDR_TYPE_V4); }while(0)

/** @ingroup ipaddr */
#define ip_addr_set(dest, src) do{ IP_SET_TYPE(dest, IP_GET_TYPE(src)); if(IP_IS_V6(src)){ \
  ip6_addr_set(ip_2_ip6(dest), ip_2_ip6(src)); }else{ \
  ip4_addr_set(ip_2_ip4(dest), ip_2_ip4(src)); }}while(0)
/** @ingroup ipaddr */
#define ip_addr_set_ipaddr(dest, src) ip_addr_set(dest, src)
/** @ingroup ipaddr */
#define ip_addr_set_zero(ipaddr)     do{ \
  ip6_addr_set_zero(ip_2_ip6(ipaddr)); IP_SET_TYPE(ipaddr, 0); }while(0)
/** @ingroup ip5addr */
#define ip_addr_set_zero_ip4(ipaddr)     do{ \
  ip6_addr_set_zero(ip_2_ip6(ipaddr)); IP_SET_TYPE(ipaddr, IPADDR_TYPE_V4); }while(0)
/** @ingroup ip6addr */
#define ip_addr_set_zero_ip6(ipaddr)     do{ \
  ip6_addr_set_zero(ip_2_ip6(ipaddr)); IP_SET_TYPE(ipaddr, IPADDR_TYPE_V6); }while(0)
/** @ingroup ipaddr */
#define ip_addr_set_any(is_ipv6, ipaddr)      do{if(is_ipv6){ \
  ip6_addr_set_any(ip_2_ip6(ipaddr)); IP_SET_TYPE(ipaddr, IPADDR_TYPE_V6); }else{ \
  ip4_addr_set_any(ip_2_ip4(ipaddr)); IP_SET_TYPE(ipaddr, IPADDR_TYPE_V4); }}while(0)

/** @ingroup ipaddr */
#define ip_addr_cmp(addr1, addr2)    ((IP_GET_TYPE(addr1) != IP_GET_TYPE(addr2)) ? 0 : (IP_IS_V6_VAL(*(addr1)) ? \
  ip6_addr_cmp(ip_2_ip6(addr1), ip_2_ip6(addr2)) : \
  ip4_addr_cmp(ip_2_ip4(addr1), ip_2_ip4(addr2))))
/** @ingroup ipaddr */
#define ip_addr_isany(ipaddr)        ((IP_IS_V6(ipaddr)) ? \
  ip6_addr_isany(ip_2_ip6(ipaddr)) : \
  ip4_addr_isany(ip_2_ip4(ipaddr)))
/** @ingroup ipaddr */
#define ip_addr_isany_val(ipaddr)        ((IP_IS_V6_VAL(ipaddr)) ? \
  ip6_addr_isany_val(*ip_2_ip6(&(ipaddr))) : \
  ip4_addr_isany_val(*ip_2_ip4(&(ipaddr))))

#define IP46_ADDR_ANY(type) (((type) == IPADDR_TYPE_V6)? IP6_ADDR_ANY : IP4_ADDR_ANY)

#else /* LWIP_IPV4 && LWIP_IPV6 */

#if NETDEV_IPV4

typedef ip4_addr_t ip_addr_t;

#define ip_addr_copy(dest, src)             ip4_addr_copy(dest, src)
#define ip_addr_cmp(addr1, addr2)           ip4_addr_cmp(addr1, addr2)
#define ip_addr_set_zero(ipaddr)            ip4_addr_set_zero(ipaddr)
#define ip_addr_isany(ipaddr)               ip4_addr_isany(ipaddr)

#else /* end NETDEV_IPV4 then NETDEV_IPV6 */

typedef ip6_addr ip_addr_t

#define ip_addr_copy(dest, src)             ip6_addr_copy(dest, src)
#define ip_addr_copy_from_ip6(dest, src)    ip6_addr_copy(dest, src)
#define ip_addr_cmp(addr1, addr2)           ip6_addr_cmp(addr1, addr2)
#define ip_addr_set(dest, src)              ip6_addr_set(dest, src)
#define ip_addr_set_ipaddr(dest, src)       ip6_addr_set(dest, src)
#define ip_addr_set_zero(ipaddr)            ip6_addr_set_zero(ipaddr)
#define ip_addr_set_zero_ip6(ipaddr)        ip6_addr_set_zero(ipaddr)
#define ip_addr_set_any(is_ipv6, ipaddr)    ip6_addr_set_any(ipaddr)
#define ip_addr_isany(ipaddr)               ip6_addr_isany(ipaddr)
#define ip_addr_isany_val(ipaddr)           ip6_addr_isany_val(ipaddr)
#endif /* NETDEV_IPV6 */

#endif /* NETIF_IPV4 && NETIF_IPV6 */

#ifdef __cplusplus
}
#endif

#endif /* __NETDEV_IPADDR_H__ */
