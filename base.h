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

struct LFQryPositionField
{
    char     BrokerID[12];              //经纪公司代码
    char     InvestorID[20];            //投资者代码
    char     InstrumentID[32];          //合约代码
    char     ExchangeID[10];            //交易所代码
};

struct LFRspPositionField
{
    char                    InstrumentID[32];          //合约代码
    int                     YdPosition;            //上日持仓
    int                     Position;              //总持仓
    char                    BrokerID[12];              //经纪公司代码
    char                    InvestorID[20];            //投资者代码
    double                  PositionCost;          //持仓成本
    char                    HedgeFlag;             //投机套保标志
    char                    PosiDirection;         //持仓多空方向
};

struct LFQryAccountField
{
    char        BrokerID[12];              //经纪公司代码
    char        InvestorID[20];            //投资者代码
};


struct LFOrderActionField
{
    char        BrokerID[12];              //经纪公司代码
    char        InvestorID[20];            //投资者代码
    char        InstrumentID[32];          //合约代码
    char        ExchangeID[12];            //交易所代码
    char        UserID[16];                //用户代码
    char        OrderRef[22];              //报单引用
    char        OrderSysID[32];            //报单编号
    int         RequestID;             //请求编号
    char        ActionFlag;            //报单操作标志
    double      LimitPrice;            //价格
    int         VolumeChange;          //数量变化
    int         KfOrderID;             //Kf系统内订单ID
};


typedef double price_t;

#endif
