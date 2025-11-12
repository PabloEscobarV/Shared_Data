
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   info_all_cu.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/07 09:42:41 by blackrider        #+#    #+#             */
/*   Updated: 2025/11/07 17:04:23 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "info_all_cu.hpp"

#include "../can_shared_data/can_shared_data.hpp"
#include "../can_shared_data/can_app_message.hpp"
#include "../hdrs/socket.hpp"
#include "../hdrs/test.hpp"

#include <iostream>
#include <unordered_map>
#include <unistd.h>

using namespace std;

using ssv_message_t = Can_shared_data::ssv_message_t;
using ssrv_message_t = Can_shared_data::ssrv_message_t;
using sse_message_t = Can_shared_data::sse_message_t;

struct most_popular_t
{
	int value;
	int count;
};

enum
{
	NO_DATA = 0,
	SSV_DATA,
	SSRV_DATA,
	SSE_DATA
};

bool check_ssv_info(uint16_t pid, ssv_message_t& ssv_msg, unordered_map<uint16_t, Cu_data>& site)
{
	uint32_t p_val = 0;
	Cu_data::param_info_t param_info;
	
	memcpy(&p_val, ssv_msg.param_val, sizeof(uint32_t));
	
	// Check if parameter exists
	if (site[pid].get_param_info(ssv_msg.param_num, param_info))
	{	
		site[pid].set_iterator(ssv_msg.param_num, ssv_msg.iterator);
		site[pid].increment_ssv_count(ssv_msg.param_num);
		if (p_val != param_info.param_value)
		{
			site[pid].set_param_value(ssv_msg.param_num, p_val);
			return true;
		}
		return false;
	}
	else
	{
		// Parameter doesn't exist - create it
		cout << "PID: " << pid << " Param Num: " << ssv_msg.param_num
				 << " New parameter created with Value: " << p_val << endl;
		site[pid].set_param_info(ssv_msg.param_num, p_val, ssv_msg.iterator, 1, 0, 0);
		return true;
	}
}

bool get_ssv_info(Can_app_message& can_app_message, unordered_map<uint16_t, Cu_data>& site)
{
	bool result = false;
	ssv_message_t ssv_msg;
	uint16_t pid = can_app_message.get_source_controller_id();
	
	can_app_message.reset_offset();  // Reset offset before reading SSV messages
	
	for (uint16_t i = Can_shared_data::SSV_MESSAGE_0; i < Can_shared_data::SSV_MESSAGE_MAX_COUNT; ++i)
	{
		if (can_app_message.get_data<ssv_message_t>(i, ssv_msg))
		{
			result |= check_ssv_info(pid, ssv_msg, site);
		}
	}
	return result;
}

bool get_ssrv_info(Can_app_message& can_app_message, unordered_map<uint16_t, Cu_data>& site)
{
	ssrv_message_t ssrv_msg;
	uint16_t pid = can_app_message.get_source_controller_id();
	Cu_data::param_info_t param_info;
	bool result = false;
	
	can_app_message.reset_offset();  // Reset offset before reading SSRV messages

	for (uint16_t i = Can_shared_data::SSRV_MESSAGE_0; i < Can_shared_data::SSRV_MESSAGE_MAX_COUNT; ++i)
	{
		if (can_app_message.get_data<ssrv_message_t>(i, ssrv_msg))
		{
			if (site[pid].get_param_info(ssrv_msg.param_num, param_info))
			{
				site[pid].increment_ssrv_count(ssrv_msg.param_num);
			}
			result = true;
		}
	}
	return result;
}

bool get_sse_info(Can_app_message& can_app_message, unordered_map<uint16_t, Cu_data>& site)
{
	sse_message_t sse_msg;
	uint16_t pid = can_app_message.get_source_controller_id();
	Cu_data::param_info_t param_info;
	bool result = false;
	
	can_app_message.reset_offset();  // Reset offset before reading SSE messages

	for (uint16_t i = Can_shared_data::SSE_MESSAGE_0; i < Can_shared_data::SSE_MESSAGE_MAX_COUNT; ++i)
	{
		if (can_app_message.get_data<sse_message_t>(i, sse_msg))
		{
			// Only increment if the parameter already exists (was created by SSV)
			if (site[pid].get_param_info(sse_msg.param_num, param_info))
			{
				site[pid].increment_sse_count(sse_msg.param_num);
				result = true;
			}
		}
	}
	return result;
}

template <typename data_t>
void set_bit(data_t& data, int bit)
{
	data |= 1 << bit;
}

template<typename data_t>
bool test_bit(const data_t& data, int bit)
{
	return (data & (1 << bit)) != 0;
}

int get_data_statistik(Can_app_message& can_app_message, unordered_map<uint16_t, Cu_data>& site)
{
	int result = 0;

	if (get_ssv_info(can_app_message, site))
		set_bit(result, SSV_DATA);
	if (get_ssrv_info(can_app_message, site))
		set_bit(result, SSRV_DATA);
	if (get_sse_info(can_app_message, site))
		set_bit(result, SSE_DATA);
	return result;
}

unordered_map<int, int> get_same_val_count(uint16_t p_idx, unordered_map<uint16_t, Cu_data>& site)
{
	unordered_map<int, int> count_map;
	Cu_data::param_info_t param_info;

	for (auto& pair : site)
	{
		if (pair.second.get_param_info(p_idx, param_info))
		{
			int val = param_info.param_value;
			count_map[val]++;
		}
	}
	return count_map;
}

most_popular_t find_most_popular_simple(uint16_t p_idx, unordered_map<uint16_t, Cu_data>& site)
{
	most_popular_t result = {0, 0};
	unordered_map<int, int> count_map = get_same_val_count(p_idx, site);

	for (auto& pair : count_map)
	{
		if (pair.second > result.count)
		{
			result.value = pair.first;
			result.count = pair.second;
		}
	}
	return result;
}

float get_percent(int data, int total)
{
	return static_cast<float>(data) / total * 100.0f;
}

void print_complete_statistik(unordered_map<uint16_t, Cu_data>& site)
{
	Cu_data::param_info_t param_info;

	for (auto& pair : site)
	{
		cout << "CU ID: " << pair.first << endl;
		for (uint16_t i = 0; i < NUM_SYNC_PARAM; ++i)
		{
			if (pair.second.get_param_info(i, param_info))
			{
				cout << " Param Num: " << i
						 << ", Value: " << param_info.param_value
						 << ", Iterator: " << param_info.iterator
						 << ", SSV Count: " << param_info.ssv_count
						 << ", SSRV Count: " << param_info.ssrv_count
						 << ", SSE Count: " << param_info.sse_count
						 << endl;
			}
		}
	}
}

void print_statistik(unordered_map<uint16_t, Cu_data>& site)
{
	Cu_data::param_info_t param_info;

	for (int i = 0; i < NUM_SYNC_PARAM; ++i)
	{
		most_popular_t popular = find_most_popular_simple(i, site);
		cout << "Param Num: " << i
				 << ", Most Popular Value: " << popular.value
				 << " [" << get_percent(popular.count, site.size()) << "%]" << endl;
	}
}

void receive_all_cu_data()
{
	udp_data_t udp_data_receiver = create_receive_socket();
	Can_app_message can_app_message;
	unordered_map<uint16_t, Cu_data> site;
	int result = 0;

	while (true)
	{
		receive_udp(udp_data_receiver, can_app_message);
		result = get_data_statistik(can_app_message, site);
		if (test_bit(result, SSV_DATA))
		{
			cout << "\n========== STATISTICS ==========\n";
			print_statistik(site);
			cout << "=========================================\n\n";
		}
		if (test_bit(result, SSRV_DATA) || test_bit(result, SSE_DATA))
		{
			cout << "\n===== COMPLETE STATISTICS =====\n";
			print_complete_statistik(site);
			cout << "================================\n\n";
		}
		result = 0;
		can_app_message = Can_app_message();
	}
	close(udp_data_receiver.sock_fd);
}
