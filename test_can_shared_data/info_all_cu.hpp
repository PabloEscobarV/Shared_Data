/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   info_all_cu.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/07 12:46:47 by blackrider        #+#    #+#             */
/*   Updated: 2025/11/07 17:23:58 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cstdint"
#include <unordered_map>

using namespace std;

class Cu_data
{
	public:
	
	struct param_info_t
	{
		int32_t		param_value;
		uint16_t	param_num;
		uint16_t	iterator;
		uint16_t	ssv_count;
		uint16_t	ssrv_count;
		uint16_t 	sse_count;

		param_info_t() 
			: param_value(0),
				param_num(0),
				iterator(0),
				ssv_count(0),
				ssrv_count(0),
				sse_count(0)
		{}

		param_info_t(int32_t p_value,
								uint16_t p_num,
								uint16_t iterator,
								uint16_t ssv_count,
								uint16_t ssrv_count,
								uint16_t sse_count)
							: param_value(p_value),
								param_num(p_num),
								iterator(iterator),
								ssv_count(ssv_count),
								ssrv_count(ssrv_count),
								sse_count(sse_count)
		{}
	};

	 void set_param_info(uint16_t p_num,
											int32_t p_value,
											uint16_t iterator,
											uint16_t ssv_count,
											uint16_t ssrv_count,
											uint16_t sse_count)
	 {
		 cu_params[p_num] = param_info_t(p_value, p_num, iterator, ssv_count, ssrv_count, sse_count);
	 }

	 void set_param_value(uint16_t p_num, int32_t p_value)
	 {
		 if (cu_params.find(p_num) != cu_params.end())
		 {
			 cu_params[p_num].param_value = p_value;
		 }
	 }

	 void set_iterator(uint16_t p_num, uint16_t iterator)
	 {
		 if (cu_params.find(p_num) != cu_params.end())
		 {
			 cu_params[p_num].iterator = iterator;
		 }
	 }

	 void increment_ssv_count(uint16_t p_num)
	 {
		 if (cu_params.find(p_num) != cu_params.end())
		 {
			 ++(cu_params[p_num].ssv_count);
		 }
	 }

	 void increment_ssrv_count(uint16_t p_num)
	 {
		 if (cu_params.find(p_num) != cu_params.end())
		 {
			 ++(cu_params[p_num].ssrv_count);
		 }
	 }

	 void increment_sse_count(uint16_t p_num)
	 {
		 if (cu_params.find(p_num) != cu_params.end())
		 {
			 ++(cu_params[p_num].sse_count);
		 }
	 }

	 bool get_param_info(uint16_t p_num, param_info_t& p_info)
	 {
		if (cu_params.find(p_num) != cu_params.end())
		{
			p_info = cu_params[p_num];
			return true;
		}
		return false;
	 }

	 private:
		unordered_map<uint16_t, param_info_t> cu_params;
};

void receive_all_cu_data();
