/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_buffer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 20:51:24 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/23 21:13:36 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_BUFFER_HPP
#define SHARED_BUFFER_HPP

#include <cstdint>

class Shared_buffer
{
    uint8_t *buffer;
    uint16_t buffer_size;
    uint16_t offset;

    inline bool check_data_bounds(const uint16_t len)
    {
      return ((len > 0) && (get_available_size() >= len));
    }

  public:

    Shared_buffer(uint16_t size = 0) :
      buffer(nullptr),
      buffer_size(size),
      offset(0)
    {
      init(size);
    }

    ~Shared_buffer()
    {
      delete[] buffer;
    }

    inline bool check_access_len(const uint16_t len) const
    {
      return (offset + len <= static_cast<uint32_t>(buffer_size));
    }

    inline void init(const uint16_t size)
    {
      if ((size != 0) && (buffer == nullptr))
      {
        buffer_size = size;
        buffer = new uint8_t[size]();
      }
    }

    inline uint16_t get_available_size() const
    {
      return static_cast<uint16_t>(buffer_size - offset);
    }

    inline bool read_data(uint8_t *ptr_data, const uint16_t len)
    {
      const bool result = (ptr_data != nullptr) && check_data_bounds(len);

      if (result)
      {
        (void)memcpy(ptr_data, buffer + offset, len);
      }
      return result;
    }

    inline uint16_t get_size() const
    {
      return buffer_size;
    }

    inline uint16_t get_offset() const
    {
      return offset;
    }

    inline void reset_offset()
    {
      offset = 0;
    }

    inline bool set_offset(const uint16_t offset_ptr)
    {
      if (offset_ptr < buffer_size)
      {
        offset = offset_ptr;
        return true;
      }
      return false;
    }

    inline bool write_data(const uint8_t *ptr_data, const uint16_t len)
    {
      const bool result = (ptr_data != nullptr) && check_data_bounds(len);

      if (result)
      {
        (void)memcpy(buffer + offset, ptr_data, len);
      }
      return result;
    }

};

#endif // SHARED_BUFFER_HPP
