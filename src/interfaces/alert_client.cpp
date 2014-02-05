// =============================================================================
/*!
 * \file       src/interfaces/alert_server.cpp
 *
 * This file contains the implementation of the Alert interface : Client role.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    x.x.x
 *
 * \copyright  Copyright &copy; &nbsp; 2013 Bithium S.A.
 */
// =============================================================================

#include "hanfun/interfaces/alert.h"

using namespace HF;
using namespace HF::Interfaces;

// =============================================================================
// Alert Interface : Client Role
// =============================================================================

// =============================================================================
// AlertClient::handle_commands
// =============================================================================
/*!
 *
 */
// =============================================================================
Result AlertClient::handle_command (Protocol::Packet &packet, ByteArray &payload, size_t offset)
{
   if (packet.message.itf.member != Alert::STATUS_CMD)
   {
      return Result::FAIL_SUPPORT;
   }

   Message alert_msg;

   alert_msg.unpack (payload, offset);

   status (alert_msg);

   return Result::OK;
}
