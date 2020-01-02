#pragma once
#pragma once

#include <string>
#include <vector>

#define SYNC_MSG_MASTER_HEARTBEAT 1

#define SYNC_MSG_AUX_HEARTBEAT 10000

struct SyncMsg
{
  std::string group_name, sender_hostname;
  int sender_mode, msg_type;
  std::vector<std::string> params;

  SyncMsg()
  {
    sender_mode = -1;
    msg_type = -1;
  }

  SyncMsg(const char* _group_name, int _sender_mode, int _msg_type)
  {
    group_name = _group_name;
    sender_mode = _sender_mode;
    msg_type = _msg_type;

    params.clear();
  }
};