#ifndef __QUOT_H__
#define __QUOT_H__

#define LEVEL2 5

#define __USE_LEVEL2__

#include <stdint.h>

/// 部分行情，有的行情信息用不上
typedef struct 
{
	double bid_price;
	double ask_price;
	int bid_qty;
	int ask_qty;
#ifdef __USE_LEVEL2__
	double bid_price_array[LEVEL2 - 1];
	double ask_price_array[LEVEL2 - 1];
	int bid_qty_array[LEVEL2 - 1];
	int ask_qty_array[LEVEL2 - 1];
#endif
	
	double last_price;	/// 最新价
	double tuneover;	/// 成交额
	int volume;	/// 成交量
	double open_interest;		/// 持仓量
	
	/// 不变信息放在一起
	double upper_limit;			/// 涨停板价
	double lower_limit;			/// 跌停板价
	int trade_date;
	const char contract_name[32];
} quot_t;

#endif
