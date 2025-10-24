/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_bit.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 09:59:51 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/24 20:18:12 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "csl_cmp_int/csl_cmp_int.hpp"

#include <iostream>
#include <cstdint>

using namespace std;

int main()
{
	csl_cmp_int<int> cmp_int;
	cout << "Not valid for int: " << cmp_int.not_valid() << endl;

	csl_cmp_int<uint8_t> cmp_uint8;
	cout << "Not valid for uint8_t: " << static_cast<int>(cmp_uint8.not_valid()) << endl;

	csl_cmp_int<uint64_t> cmp_uint64;
	cout << "Not valid for uint64_t: " << cmp_uint64.not_valid() << endl;

	return 0;
}
