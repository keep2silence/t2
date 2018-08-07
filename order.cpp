#include "order.h"
#include "base.h"
#include "adapter.h"

void order_t::change_order_state (order_state_t new_state)
{
    order_state_t old_state = state;
    state = new_state;

    pr_info ("order_id: %d 切换状态 %s ==> %s\n", order_id,
            order_state_str[old_state],
            order_state_str[new_state]);
}

bool order_t::is_cancel_enable ()
{
	switch (state) {
		case os_order_send_req_1:
		case os_order_rsp_ok_2:
		case os_match_partly_3:
			return true;
		default:
			return false;
	}

	return false;
}

void order_t::handle_order_rsp (order_rsp_info_t* order_rsp_info_ptr)
{
	switch (state) {
		case os_order_send_req_1:
			{
				if (order_rsp_info_ptr->success == true) {
					change_order_state (os_order_rsp_ok_2);
				} else {
					change_order_state (os_order_game_over_11);
				}
			}
			break;
		case os_match_none_cancel_send_7:
			{
				/// 正确的定单回报不改变状态
				if (order_rsp_info_ptr->success != true) {
					change_order_state (os_order_game_over_11);
					pr_error ("os_match_none_cancel_send_7状态下收到定单回报失败，严重错误.\n");
				}
			}
			break;
		default:
			pr_error ("state: %s不应该处理定单回报.\n", order_state_str[state]);
	}

	return;
}

void order_t::handle_cancel_req ()
{
	switch (state) {
		case os_order_send_req_1:
		case os_order_rsp_ok_2:
			{
				change_order_state (os_match_none_cancel_send_7);
			}
			break;
		case os_match_partly_3:
			{
				change_order_state (os_match_partly_cancel_send_5);
			}
			break;
		default:
			pr_error ("state: %s不应该处理撤单请求.\n", order_state_str[state]);
	}

	return;
}

void order_t::handle_cancel_rsp (cancel_rsp_info_t* cancel_rsp_info_ptr)
{
	switch (state) {
		case os_match_partly_cancel_send_5:
			{
				if (cancel_rsp_info_ptr->success == true) {
					change_order_state (os_match_partly_cancel_ok_6);
					change_order_state (os_order_game_over_11);
				} else {
					pr_error ("os_match_partly_cancel_send_5状态下收到撤单失败，严重错误.\n");
				}
			}
			break;
		case os_match_none_cancel_send_7:
			{
				if (cancel_rsp_info_ptr->success == true) {
					change_order_state (os_match_none_cancel_ok_8);
				} else {
					pr_error ("os_match_none_cancel_send_7状态下收到撤单失败，严重错误.\n");
				}
			}
			break;
		case os_match_all_cancel_send_9: 
			{
				if (cancel_rsp_info_ptr->success == false) {
					change_order_state (os_match_all_cancel_fail_10);
					change_order_state (os_order_game_over_11);
				} else {
					pr_error ("os_match_all_cancel_send_9状态下收到撤单成功，严重错误.\n");
				}
			}
			break;
		default:
			pr_error ("state: %s不应该处理撤单回报.\n", order_state_str[state]);
	}

	return;
}

void order_t::handle_match_rsp (int rsp_match_qty)
{
	match_qty += rsp_match_qty;
	switch (state) {
		case os_order_rsp_ok_2:
			{
				if (match_qty < order_qty) {
					change_order_state (os_match_partly_3);
				} else if (match_qty == order_qty) {
					change_order_state (os_match_all_4);
					change_order_state (os_order_game_over_11);
				} else {
					pr_error ("match_qty: %d, order_qty: %d\n", match_qty, order_qty);
				}
			}
			break;

		case os_match_partly_3:
			{
				if (match_qty == order_qty) {
					change_order_state (os_match_all_4);
					change_order_state (os_order_game_over_11);
				}
			}
			break;

		case os_match_none_cancel_send_7:
			{
				if (match_qty < order_qty) {
					change_order_state (os_match_partly_cancel_send_5);
				} else if (match_qty == order_qty) {
					change_order_state (os_match_all_cancel_send_9);
				} else {
					pr_error ("match_qty: %d, order_qty: %d\n", match_qty, order_qty);
				}
			}
			break;

		case os_match_partly_cancel_send_5:
			{
				if (match_qty == order_qty) {
					change_order_state (os_match_all_cancel_send_9);
				} else if (match_qty > order_qty) {
					pr_error ("match_qty: %d, order_qty: %d\n", match_qty, order_qty);
				}
			}
			break;
		default:
			pr_error ("state: %s不应该处理成交回报.\n", order_state_str[state]);
	}

	return;
}
