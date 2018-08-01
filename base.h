#ifndef __BASE_H__
#define __BASE_H__

#include <stdio.h>

#ifdef NDEBUG
#define pr_debug
#else
#define pr_debug(fmt, ...) \
do {\
    printf ("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
}\
while (0)
#endif /// NDEBUG

#define pr_info(fmt, ...) \
do {\
    printf ("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
}\
while (0)

#define pr_error(fmt, ...) \
do {\
    printf ("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
}\
while (0)

#define pr_emerg(fmt, ...) \
do {\
    printf ("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
	exit (EXIT_FAILURE); \
}\
while (0)

typedef enum {
    d_unknown = 0,
    d_buy,
    d_sell
} direction_t;


extern const char* direction_str[];

typedef enum {
	o_unknown = 0,
	o_open,
	o_offset
} offset_flag_t;

extern const char* offset_flag_str[];

class order_rsp_info_t
{
public:
	int order_id = -1;
	bool success = false;
};

class cancel_rsp_info_t
{
public:
	int order_id = 0;
	bool success = false;
	int cancel_qty = 0;
};
class order_match_info_t
{
public:

};
#endif
