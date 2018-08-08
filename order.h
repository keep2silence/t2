#ifndef __ORDER_H__
#define __ORDER_H__

#include <string>
#include "pool.h"
#include "base.h"

class order_rsp_info;
class order_t
{
public:
	typedef enum {
		os_unknown_0 = 0,					/// 一定要从0开始编号，后面数组对此有依赖
		os_order_send_req_1,				/// 发出定单请求
		os_order_rsp_ok_2,					/// 定单请求被定单应答正确
		os_match_partly_3,					/// 定单部分成交
		os_match_all_4,						/// 定单全部成交
		os_match_partly_cancel_send_5, 		/// 部分成交后发出撤单请求
		os_match_partly_cancel_ok_6,		/// 部分成交，剩余撤单成功
		os_match_none_cancel_send_7,		/// 定单无成交，发出撤单请求
		os_match_none_cancel_ok_8,			/// 定单无成交，撤单成功
		os_match_all_cancel_send_9,			/// 撤单发出后，全部成交
		os_match_all_cancel_fail_10,		/// 全部成交，撤单失败
		os_order_game_over_11				
	} order_state_t;

	void handle_order_rsp (order_rsp_info_t* order_rsp_info_ptr);
	void handle_match_rsp (int rsp_match_qty);
	void handle_cancel_req ();
	void handle_cancel_rsp (cancel_rsp_info_t* cancel_rsp_info_ptr);

	/// void handle_quot ();

	/// 当前状态下是否允许撤单
	bool is_cancel_enable ();

	bool is_game_over ()
	{
		return state == os_order_game_over_11;
	}
private:
	void change_order_state (order_state_t new_state);
public:
	int order_id = -1;
	uint32_t contract_no = 0;
	char contract_name[32] = {'\0'};
	direction_t direction = d_unknown;
	offset_flag_t offset = o_unknown;
	order_price_type_t order_price_type;
	int order_qty = 0;
	
	/// 多账户同时运行，需要通过此索引来告诉底层一些基础信息
	int account_index = -1;

	/// double价格强制转为int32_t，double * 100，价格转为整形处理速度快
	int price = 0;		

	/// 已经成交量
	int match_qty = 0;

	/// 定单状态机
	order_state_t state = os_unknown_0;

	const char* order_state_str[12] = {
		"os_unknown_0",				
		"os_order_send_req_1",			
		"os_order_rsp_ok_2",			
		"os_match_partly_3",			
		"os_match_all_4",				
		"os_match_partly_cancel_send_5",
		"os_match_partly_cancel_ok_6",	
		"os_match_none_cancel_send_7",	
		"os_match_none_cancel_ok_8",	
		"os_match_all_cancel_send_9",	
		"os_match_all_cancel_fail_10",
		"os_order_game_over_11"
	};
};

#endif
