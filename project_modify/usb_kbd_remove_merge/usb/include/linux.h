/*
 * linux.h
 *
 * created: 2022/3/12
 *  author: 
 */

#ifndef _LINUX_H
#define _LINUX_H

//************************************************************************************************** linux_c9.h
/*
 * This file holds USB constants and structures that are needed for
 * USB device APIs.  These are used by the USB device model, which is
 * defined in chapter 9 of the USB 2.0 specification and in the
 * Wireless USB 1.0 (spread around).  Linux has several APIs in C that
 * need these:
 *
 * - the master/host side Linux-USB kernel driver API;
 * - the "usbfs" user space API; and
 * - the Linux "gadget" slave/device/peripheral side driver API.
 *
 * USB 2.0 adds an additional "On The Go" (OTG) mode, which lets systems
 * act either as a USB master/host or as a USB slave/device.  That means
 * the master and slave side APIs benefit from working well together.
 *
 * There's also "Wireless USB", using low power short range radios for
 * peripheral interconnection but otherwise building on the USB framework.
 *
 * Note all descriptors are declared '__attribute__((packed))' so that:
 *
 * [a] they never get padded, either internally (USB spec writers
 *     probably handled that) or externally;
 *
 * [b] so that accessing bigger-than-a-bytes fields will never
 *     generate bus errors on any platform, even when the location of
 *     its descriptor inside a bundle isn't "naturally aligned", and
 *
 * [c] for consistency, removing all doubt even when it appears to
 *     someone that the two other points are non-issues for that
 *     particular descriptor type.
 */
 //-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  Within the kernel and when exposed
 * through the Linux-USB APIs, they are not converted to cpu byte
 * order; it is the responsibility of the client code to do this.
 * The single exception is when device and configuration descriptors (but
 * not other descriptors) are read from usbfs (i.e. /proc/bus/usb/BBB/DDD);
 * in this case the fields are converted to host endianness by the kernel.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER         0x06
#define USB_DT_OTHER_SPEED_CONFIG       0x07
#define USB_DT_INTERFACE_POWER          0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG                      0x09
#define USB_DT_DEBUG                    0x0a
#define USB_DT_INTERFACE_ASSOCIATION    0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY                 0x0c
#define USB_DT_KEY                      0x0d
#define USB_DT_ENCRYPTION_TYPE          0x0e
#define USB_DT_BOS                      0x0f
#define USB_DT_DEVICE_CAPABILITY        0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP   0x11
#define USB_DT_WIRE_ADAPTER             0x21
#define USB_DT_RPIPE                    0x22
#define USB_DT_CS_RADIO_CONTROL         0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE               0x24
/* From the USB 3.0 spec */
#define USB_DT_SS_ENDPOINT_COMP         0x30
/* From HID 1.11 spec */
#define USB_DT_HID_REPORT               0x22

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE        (USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG        (USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING        (USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE     (USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT      (USB_TYPE_CLASS | USB_DT_ENDPOINT)

/*
 * All standard descriptors have these 2 fields at the beginning
 */
struct usb_descriptor_header
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;
} __attribute__ ((packed));

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/* USB_DT_CONFIG: Configuration descriptor information.
 *
 * USB_DT_OTHER_SPEED_CONFIG is the same descriptor, except that the
 * descriptor type is different.  Highspeed-capable devices can look
 * different depending on what speed they're currently running.  Only
 * devices with a USB_DT_DEVICE_QUALIFIER have any OTHER_SPEED_CONFIG
 * descriptors.
 */
struct usb_config_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  bMaxPower;
} __attribute__ ((packed));

#define USB_DT_CONFIG_SIZE          9

/*
 * from config descriptor bmAttributes
 */
#define USB_CONFIG_ATT_ONE          (1 << 7)    /* must be set */
#define USB_CONFIG_ATT_SELFPOWER    (1 << 6)    /* self powered */
#define USB_CONFIG_ATT_WAKEUP       (1 << 5)    /* can wakeup */
#define USB_CONFIG_ATT_BATTERY      (1 << 4)    /* battery powered */

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * USB_DT_SS_ENDPOINT_COMP: SuperSpeed Endpoint Companion descriptor
 */
struct usb_ss_ep_comp_descriptor
{
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned char  bMaxBurst;
    unsigned char  bmAttributes;
    unsigned short wBytesPerInterval;
} __attribute__ ((packed));

#define USB_DT_SS_EP_COMP_SIZE      6

/*
 * Bits 4:0 of bmAttributes if this is a bulk endpoint
 */
static inline int
usb_ss_max_streams(const struct usb_ss_ep_comp_descriptor *comp)
{
    int max_streams;

    if (!comp)
        return 0;

    max_streams = comp->bmAttributes & 0x1f;

    if (!max_streams)
        return 0;

    max_streams = 1 << max_streams;

    return max_streams;
}

/*
 * Bits 1:0 of bmAttributes if this is an isoc endpoint
 */
#define USB_SS_MULT(p)          (1 + ((p) & 0x3))

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * USB 2.0 defines three speeds, here's how Linux identifies them
 */

enum usb_device_speed
{
    USB_SPEED_UNKNOWN = 0,              /* enumerating */
    USB_SPEED_LOW, USB_SPEED_FULL,      /* usb 1.1 */
    USB_SPEED_HIGH,                     /* usb 2.0 */
    USB_SPEED_WIRELESS,                 /* wireless (usb 2.5) */
    USB_SPEED_SUPER,                    /* usb 3.0 */
};

/**
 * struct usb_string - wraps a C string and its USB id
 * @id:the (nonzero) ID for this string
 * @s:the string, in UTF-8 encoding
 *
 * If you're using usb_gadget_get_string(), use this to wrap a string
 * together with its ID.
 */
struct usb_string
{
    unsigned char id;
    const char *s;
};

//************************************************************************************************** linux_kernel.h
#define REPEAT_BYTE(x)          ((~0ul / 0xff) * (x))

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#define ALIGN(x,a)              __ALIGN_MASK((x),(typeof(x))(a)-1)
#define ALIGN_DOWN(x, a)        ALIGN((x) - ((a) - 1), (a))

#define PTR_ALIGN(p, a)         ((typeof(p))ALIGN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)        (((x) & ((typeof(x))(a) - 1)) == 0)

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y)      ((__typeof__(x))((y)-1))
#define round_up(x, y)          ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y)        ((x) & ~__round_mask(x, y))

#define FIELD_SIZEOF(t, f)      (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))

#define DIV_ROUND_DOWN_ULL(ll, d) \
    ({ unsigned long long _tmp = (ll); do_div(_tmp, d); _tmp; })

#define DIV_ROUND_UP_ULL(ll, d) DIV_ROUND_DOWN_ULL((ll) + (d) - 1, (d))

#define ROUND(a, b)             (((a) + (b) - 1) & ~((b) - 1))

#if BITS_PER_LONG == 32
#define DIV_ROUND_UP_SECTOR_T(ll,d) DIV_ROUND_UP_ULL(ll, d)
#else
#define DIV_ROUND_UP_SECTOR_T(ll,d) DIV_ROUND_UP(ll,d)
#endif

/* The `const' in roundup() prevents gcc-3.3 from calling __divdi3 */
#define roundup(x, y) ({     \
    const typeof(y) __y = y; \
    (((x) + (__y - 1)) / __y) * __y; \
})

#define rounddown(x, y) ({ \
    typeof(x) __x = (x);   \
    __x - (__x % (y));     \
})

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)({ \
    typeof(x) __x = x;                  \
    typeof(divisor) __d = divisor;      \
    (((typeof(x))-1) > 0 ||             \
     ((typeof(divisor))-1) > 0 || (__x) > 0) ? \
        (((__x) + ((__d) / 2)) / (__d)) : \
        (((__x) - ((__d) / 2)) / (__d));  \
})

/*
 * Same as above but for u64 dividends. divisor must be a 32-bit
 * number.
 */
#define DIV_ROUND_CLOSEST_ULL(x, divisor)({ \
    typeof(divisor) __d = divisor;          \
    unsigned long long _tmp = (x) + (__d) / 2; \
    do_div(_tmp, __d); \
    _tmp;              \
})

/*
 * Multiplies an integer by a fraction, while avoiding unnecessary
 * overflow or loss of precision.
 */
#define mult_frac(x, numer, denom)({ \
    typeof(x) quot = (x) / (denom);  \
    typeof(x) rem  = (x) % (denom);  \
    (quot * (numer)) + ((rem * (numer)) / (denom)); \
})

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((u32)(n))

/*
 * abs() handles unsigned and signed longs, ints, shorts and chars.  For all
 * input types abs() returns a signed long.
 * abs() should not be used for 64-bit types (s64, u64, long long) - use abs64()
 * for those.
 */
#define abs(x) ({ \
        long ret; \
        if (sizeof(x) == sizeof(long)) {  \
            long __x = (x);               \
            ret = (__x < 0) ? -__x : __x; \
        } else {                          \
            int __x = (x);                \
            ret = (__x < 0) ? -__x : __x; \
        }    \
        ret; \
    })

#define abs64(x) ({    \
        s64 __x = (x); \
        (__x < 0) ? -__x : __x; \
    })

/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                \
    typeof(x) _max1 = (x);          \
    typeof(y) _max2 = (y);          \
    (void) (&_max1 == &_max2);      \
    _max1 > _max2 ? _max1 : _max2; })

#define min3(x, y, z) min((typeof(x))min(x, y), z)
#define max3(x, y, z) max((typeof(x))max(x, y), z)

/**
 * min_not_zero - return the minimum that is _not_ zero, unless both are zero
 * @x: value1
 * @y: value2
 */
#define min_not_zero(x, y) ({ \
    typeof(x) __x = (x);      \
    typeof(y) __y = (y);      \
    __x == 0 ? __y : ((__y == 0) ? __x : min(__x, __y)); })

/**
 * clamp - return a value clamped to a given range with strict typechecking
 * @val: current value
 * @lo: lowest allowable value
 * @hi: highest allowable value
 *
 * This macro does strict typechecking of lo/hi to make sure they are of the
 * same type as val.  See the unnecessary pointer comparisons.
 */
#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y) ({ \
    type __min1 = (x);       \
    type __min2 = (y);       \
    __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({ \
    type __max1 = (x);       \
    type __max2 = (y);       \
    __max1 > __max2 ? __max1: __max2; })

/**
 * clamp_t - return a value clamped to a given range using a given type
 * @type: the type of variable to use
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of type
 * 'type' to make all the comparisons.
 */
#define clamp_t(type, val, lo, hi) min_t(type, max_t(type, val, lo), hi)

/**
 * clamp_val - return a value clamped to a given range using val's type
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of whatever
 * type the input argument 'val' is.  This is useful when val is an unsigned
 * type and min and max are literals that will otherwise be assigned a signed
 * integer type.
 */
#define clamp_val(val, lo, hi) clamp_t(typeof(val), val, lo, hi)

/*
 * swap - swap value of @a and @b
 */
#define swap(a, b) \
    do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) );})

/*
 * check_member() - Check the offset of a structure member
 *
 * @structure:  Name of structure (e.g. global_data)
 * @member: Name of member (e.g. baudrate)
 * @offset: Expected offset in bytes
 */
#define check_member(structure, member, offset) _Static_assert( \
    offsetof(struct structure, member) == (offset), \
    "`struct " #structure "` offset for `" #member "` is not " #offset)
    
//************************************************************************************************** linux_list.h

#include <stddef.h>

#define LIST_POISON1  ((void *) 0x0)
#define LIST_POISON2  ((void *) 0x0)

#ifndef ARCH_HAS_PREFETCH
#define ARCH_HAS_PREFETCH
static inline void prefetch(const void *x) { ; }
#endif

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head
{
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void list_replace(struct list_head *old, struct list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

static inline void list_replace_init(struct list_head *old, struct list_head *new)
{
    list_replace(old, new);
    INIT_LIST_HEAD(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
    return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int list_empty_careful(const struct list_head *head)
{
    struct list_head *next = head->next;
    return (next == head) && (next == head->prev);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int list_is_singular(const struct list_head *head)
{
    return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_cut_position(struct list_head *list,
                                       struct list_head *head,
                                       struct list_head *entry)
{
    struct list_head *new_first = entry->next;
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *  and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(struct list_head *list,
                                     struct list_head *head,
                                     struct list_head *entry)
{
    if (list_empty(head))
        return;
    if (list_is_singular(head) &&
        (head->next != entry && head != entry))
        return;
    if (entry == head)
        INIT_LIST_HEAD(list);
    else
        __list_cut_position(list, head, entry);
}

static inline void __list_splice(const struct list_head *list,
                                 struct list_head *prev,
                                 struct list_head *next)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(const struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice_tail(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head, head->next);
        INIT_LIST_HEAD(list);
    }
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head->prev, head);
        INIT_LIST_HEAD(list);
    }
}

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member) ({ \
    struct list_head *head__ = (ptr);                  \
    struct list_head *pos__ = READ_ONCE(head__->next); \
    pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

/**
 * list_for_each    -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; prefetch(pos->next), pos != (head); \
        pos = pos->next)

/**
 * __list_for_each  -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev   -   iterate over a list backwards
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
        pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev;   \
         prefetch(pos->prev), pos != (head);  \
         pos = n, n = pos->prev)

/**
 * list_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)                 \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         prefetch(pos->member.next), &pos->member != (head);   \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)         \
    for (pos = list_entry((head)->prev, typeof(*pos), member); \
         prefetch(pos->member.prev), &pos->member != (head);   \
         pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:    the type * to use as a start point
 * @head:   the head of the list
 * @member: the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
    ((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member)            \
    for (pos = list_entry(pos->member.next, typeof(*pos), member); \
         prefetch(pos->member.next), &pos->member != (head);       \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)    \
    for (pos = list_entry(pos->member.prev, typeof(*pos), member); \
         prefetch(pos->member.prev), &pos->member != (head);       \
         pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member)            \
    for (; prefetch(pos->member.next), &pos->member != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = list_entry((head)->next, typeof(*pos), member),  \
        n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member)    \
    for (pos = list_entry(pos->member.next, typeof(*pos), member), \
        n = list_entry(pos->member.next, typeof(*pos), member);    \
         &pos->member != (head);                                   \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member)      \
    for (n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                 \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)  \
    for (pos = list_entry((head)->prev, typeof(*pos), member),  \
        n = list_entry(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.prev, typeof(*n), member))

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct hlist_head
{
    struct hlist_node *first;
};

struct hlist_node
{
    struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
    return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
    return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
    __hlist_del(n);
    n->next = LIST_POISON1;
    n->pprev = LIST_POISON2;
}

static inline void hlist_del_init(struct hlist_node *n)
{
    if (!hlist_unhashed(n))
    {
        __hlist_del(n);
        INIT_HLIST_NODE(n);
    }
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    struct hlist_node *first = h->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{
    n->pprev = next->pprev;
    n->next = next;
    next->pprev = &n->next;
    *(n->pprev) = n;
}

static inline void hlist_add_after(struct hlist_node *n, struct hlist_node *next)
{
    next->next = n->next;
    n->next = next;
    next->pprev = &n->next;

    if(next->next)
        next->next->pprev  = &next->next;
}

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos && ({ prefetch(pos->next); 1; }); \
         pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
         pos = n)

/**
 * hlist_for_each_entry - iterate over list of given type
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(tpos, pos, head, member)    \
    for (pos = (head)->first;                            \
         pos && ({ prefetch(pos->next); 1;}) &&          \
        ({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @member: the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(tpos, pos, member) \
    for (pos = (pos)->next;                              \
         pos && ({ prefetch(pos->next); 1;}) &&          \
        ({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @member: the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(tpos, pos, member) \
    for (; pos && ({ prefetch(pos->next); 1;}) &&    \
        ({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @n:      another &struct hlist_node to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(tpos, pos, n, head, member) \
    for (pos = (head)->first;                                 \
         pos && ({ n = pos->next; 1; }) &&                    \
        ({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = n)

#endif // _LINUX_H

