/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_stampt.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 00:41:31 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/28 00:50:07 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <chrono>

using namespace std;

void print_time_stamp()
{
    // Get the current time
    auto now = std::chrono::system_clock::now();
    
    // Convert to time_t for seconds
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
    // Get milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // Convert to local time
    std::tm* local_time = std::localtime(&now_c);
    
    // Print the time stamp with milliseconds
    std::cout << "Current Time: " 
              << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") 
              << "." << std::setfill('0') << std::setw(3) << ms.count()
              << std::endl;
}
