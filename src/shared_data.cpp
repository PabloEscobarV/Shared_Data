/* see header file */
SharedData::SharedData(Can_app_ic_notify_new_data_ifc * ptr_can_app_ic, Controller_addresses::Values addr_name)
  : Can_data_handler_ifc(ptr_can_app_ic, addr_name),
  idx_ssv(0),
  address_name(addr_name),
  tick(0)
{

}

/* see header file */
bool SharedData::set_data_can_thread(uint8_t data_idx,
                                             uint8_t *ptr_src_data,
                                             uint8_t src_data_len,
                                             uint8_t can_addr)
{
  bool result = false;
  can_data_t can_data;

  if (src_data_len <= 8)
  {
    can_data.message_type = data_idx;
    can_data.data_len = src_data_len;
    can_data.idx = Addr_mngr::get_instance().get_addr(static_cast<Controller_addresses::Values>(address_name));
    can_data.idx_can = can_addr;
    memcpy(can_data.data, ptr_src_data, src_data_len);
    result = handle_messages(can_data);
  }
  return result;
}

/* see header file */
int8_t SharedData::get_data_app_thread(uint8_t data_idx,
                                               uint8_t *ptr_dst_data,
                                               size_t dst_data_len,
                                               uint8_t can_addr)
{
  (void)data_idx;       // Suppress unused parameter warning
  (void)dst_data_len;   // Suppress unused parameter warning
  (void)can_addr;       // Suppress unused parameter warning
  (void)ptr_dst_data;   // Suppress unused parameter warning

  return Can_data_handler_ifc::NO_DATA; // No data for APP thread in this implementation
}

/* see header file */
int8_t SharedData::get_data_can_thread(uint8_t data_idx,
                                               uint8_t *ptr_dst_data,
                                               size_t dst_data_len,
                                               Controller_addresses::Values addr_name)
{
  can_data_t can_data;

  (void)data_idx;      // Suppress unused parameter warning
  (void)addr_name;      // Suppress unused parameter warning

  can_data.data_len = csl_cmp_int<uint8_t>::NOT_VALID;
  if (get_messages(can_data))
  {
    if (dst_data_len >= can_data.data_len)
    {
      memcpy(ptr_dst_data, can_data.data, can_data.data_len);
    }
  }
  return static_cast<int8_t>(can_data.data_len);
}

/* see header file */
int8_t SharedData::get_data_len(uint8_t data_idx) const
{
  int8_t result = NO_DATA;

  switch (data_idx)
  {
    case 0: // temporery idx for SSV message
      result = static_cast<int8_t>(sizeof(ssv_message_t));
      break;
    case 1: // temporery idx for SSRV message
      result = static_cast<int8_t>(sizeof(ssrv_message_t));
      break;
    case 2: // temporery idx for SSE message
      result = static_cast<int8_t>(sizeof(sse_message_t));
      break;
    default:
      break;
  }
  return result;
}

/* see header file */
void SharedData::initialize()
{
  for (int i = 0; i < NUM_SYNC_PARAM; i++)
  {
    shared_params[i].init(sync_param_list[i]);
  }
}

/* see header file */
void  SharedData::init(const uint16_t p_num, const uint16_t idx)
{
  if ((p_num > 0) && (idx < COUNT))
  {
    shared_params[idx].init(p_num);
  }
}

/* see header file */
uint16_t SharedData::get_param_num(const uint16_t idx) const
{
  uint16_t p_num = 0;

  if (idx < COUNT)
  {
    p_num = shared_params[idx].get_param_num();
  }
  return p_num;
}

/* see header file */
void  SharedData::period_counter()
{
  tick++;
}

/* see header file */
bool  SharedData::add_ssrv_message(const uint16_t param_num, const uint8_t *new_param_val, uint16_t size)
{
  ssrv_service_t new_message;
  bool  result = false;

  new_message.idx = get_idx(param_num);
  if (new_message.idx < COUNT)
  {
    shared_params[new_message.idx].add_new_param_value(SSRV_ATTEMPTS, new_param_val, size);
    result = ssrv_queue.push(new_message);
  }
  return result;
}

/* see header file */
bool  SharedData::get_messages(can_data_t &can_data)
{
  bool  result = false;

  result = set_sse_message(can_data);
  if (!result)
  {
    result = set_ssrv_message(can_data);
  }
  if (!result)
  {
    result = set_ssv_message(can_data);
  }
  return result;
}

/* see header file */
bool  SharedData::handle_messages(can_data_t &can_data)
{
  ssv_message_t    ssv_message;
  ssrv_message_t  ssrv_message;
  sse_message_t    sse_message;
  bool  result = false;

  switch (can_data.message_type)
  {
    case SSV_MESSAGE:
      memcpy(&ssv_message, can_data.data, can_data.data_len);
      result = handle_ssv_message(ssv_message, can_data.idx, can_data.idx_can);
      break;
    case SSRV_MESSAGE:
      memcpy(&ssrv_message, can_data.data, can_data.data_len);
      result = handle_ssrv_message(ssrv_message);
      break;
    case SSE_MESSAGE:
      memcpy(&sse_message, can_data.data, can_data.data_len);
      result = handle_sse_message(sse_message);
      break;
    default:
      break;
  }
  return result;
}

/* see header file */
bool  SharedData::get_ssv_message(ssv_message_t &message)
{
  uint16_t  idx = check_ssrv_end_counters();

  if (idx < COUNT)
  {
    shared_params[idx].accept_new_value();
  }
  else
  {
    idx = idx_ssv;
    idx_ssv = static_cast<uint16_t>((idx_ssv + 1) % COUNT);
  }
  message.iterator = shared_params[idx].get_iterator();
  message.param_num = shared_params[idx].get_param_num();
  return shared_params[idx].get_param_value(message.param_val);
}

/* see header file */
bool  SharedData::get_ssrv_message(ssrv_message_t &message)
{
  uint8_t  queue_counter = 0;
  ssrv_service_t  ssrv_service;
  const bool  result = ssrv_queue.pop(ssrv_service);

  if (result)
  {
    shared_params[ssrv_service.idx].get_ssrv_end_counter(queue_counter);
    message.param_num = shared_params[ssrv_service.idx].get_param_num();
    shared_params[ssrv_service.idx].get_param_value(message.param_val);
    if ((--queue_counter) > 0)
    {
      ssrv_queue.push(ssrv_service);
      ssrv_queue.swap(SSRV_PERIOD);
      shared_params[ssrv_service.idx].set_ssrv_end_counter(queue_counter);
    }
    else
    {
      shared_params[ssrv_service.idx].set_ssrv_end_counter(SSRV_WAIT_TICKS);
    }
  }
  return result;
}

/* see header file */
bool  SharedData::get_sse_message(sse_message_t &message)
{
  sse_service_t  sse_service;
  const bool  result = sse_queue.pop(sse_service);

  if (result)
  {
    message.param_num = shared_params[sse_service.idx].get_param_num();
    message.error_code = shared_params[sse_service.idx].get_error_code();
    sse_service.counter--;
    if (sse_service.counter > 0)
    {
      sse_queue.push(sse_service);
      sse_queue.swap(SSE_PERIOD);
    }
  }
  return result;
}

/* see header file */
bool  SharedData::set_ssv_message(can_data_t &can_data)
{
  ssv_message_t  message;
  bool  result = false;

  if (sizeof(ssv_message_t) <= can_data.data_len)
  {
    if (check_counter_ssv() && get_ssv_message(message))
    {
      memcpy(&can_data.data, &message, sizeof(ssv_message_t));
      can_data.data_len = sizeof(ssv_message_t);
      can_data.message_type = SSV_MESSAGE;
      result = true;
    }
  }
  return result;
}

/* see header file */
bool  SharedData::set_ssrv_message(can_data_t &can_data)
{
  ssrv_message_t  message;
  bool  result = false;

  if (sizeof(ssrv_message_t) <= can_data.data_len)
  {
    if ((tick % SSRV_PERIOD == 0) && get_ssrv_message(message))
    {
      memcpy(&can_data.data, &message, sizeof(ssrv_message_t));
      can_data.data_len = sizeof(ssrv_message_t);
      can_data.message_type = SSRV_MESSAGE;
      result = true;
    }
  }
  return result;
}

/* see header file */
bool  SharedData::set_sse_message(can_data_t &can_data)
{
  sse_message_t  message;
  bool  result = false;

  if ((sizeof(sse_message_t) <= can_data.data_len))
  {
    if (get_sse_message(message))
    {
      memcpy(&can_data.data, &message, sizeof(sse_message_t));
      can_data.data_len = sizeof(sse_message_t);
      can_data.message_type = SSE_MESSAGE;
      result = true;
    }
  }
  return result;
}

/* see header file */
bool  SharedData::handle_ssv_message(const ssv_message_t &message,
                                            const uint16_t id,
                                            const uint16_t id_can)
{
  sse_service_t  sse_service;
  bool  result = false;

  sse_service.counter = SSRV_ATTEMPTS;
  sse_service.idx = get_idx(message.param_num);
  if (sse_service.idx < COUNT)
  {
    result = shared_params[sse_service.idx].handle_p_synchro(message.param_val, message.iterator, id, id_can);
    if (!result)
    {
      sse_queue.push(sse_service);
    }
  }
  return result;
}

/* see header file */
bool  SharedData::handle_ssrv_message(const ssrv_message_t &message)
{
  sse_service_t  sse_service;
  bool  result = false;

  sse_service.counter = 1;
  sse_service.idx = get_idx(message.param_num);
  if (sse_service.idx < COUNT)
  {
    result = shared_params[sse_service.idx].handle_p_val_change_req(message.param_val);
    if (!result)
    {
      sse_queue.push(sse_service);
    }
  }
  return result;
}

/* see header file */
bool  SharedData::handle_sse_message(const sse_message_t &message)
{
  bool  result = false;
  const uint16_t idx = get_idx(message.param_num);

  if (idx < COUNT)
  {
    result = shared_params[idx].handle_error_code(message.error_code);
  }
  return result;
}

/* see header file */
uint8_t  SharedData::get_idx(const uint16_t p_num) const
{
  uint8_t  left = 0;
  uint8_t  right = COUNT;
  uint8_t   mid = 0;

  while (left < right)
  {
    mid = left + static_cast<uint8_t>((right - left) / 2);

    if (shared_params[mid].get_param_num() == p_num)
    {
      break ;
    }
    if (shared_params[mid].get_param_num() < p_num)
    {
      left = mid + 1;
    }
    else
    {
      right = mid;
    }
  }
  if (shared_params[mid].get_param_num() != p_num)
  {
    mid = COUNT;
  }
  return mid;
}

/* see header file */
uint16_t  SharedData::check_ssrv_end_counters()
{
  uint8_t    ssrv_counter = 0;
  uint16_t  idx = 0;

  while (idx < COUNT)
  {
    if (shared_params[idx].get_ssrv_end_counter(ssrv_counter))
    {
      if (get_ticks_diff(ssrv_counter) >= SSRV_WAIT_TICKS)
      {
        break ;
      }
    }
    ++idx;
  }
  return idx;
}

/* see header file */
bool  SharedData::check_counter_ssv() const
{
  bool  result = false;

  if (tick % SharedData::SSV_PERIOD == 0)
  {
    result = true;
    if (idx_ssv == 0 && (tick % SSV_ALL_PERIOD != 0))
    {
      result = false;
    }
  }
  return result;
}
