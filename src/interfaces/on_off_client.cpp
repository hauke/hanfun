// =============================================================================
/*!
 * \file       src/interfaces/on_off_client.cpp
 *
 * This file contains the implementation of the On-Off interface : Client role.
 *
 * \version    0.4.0
 *
 * \copyright  Copyright &copy; &nbsp; 2014 ULE Alliance
 *
 * For licensing information, please see the file 'LICENSE' in the root folder.
 *
 * Initial development by Bithium S.A. [http://www.bithium.com]
 */
// =============================================================================

#include "hanfun/interfaces/on_off.h"

using namespace HF;
using namespace HF::Interfaces;

// =============================================================================
// On-Off Interface : Client Role
// =============================================================================

// =============================================================================
// OnOff::Client::on
// =============================================================================
/*!
 *
 */
// =============================================================================
void OnOff::Client::on (Protocol::Address &addr)
{
   Protocol::Message message;

   message.itf.role   = SERVER_ROLE;
   message.itf.id     = OnOff::Client::uid ();
   message.itf.member = ON_CMD;

   sendMessage (addr, message);
}

// =============================================================================
// OnOff::Client::off
// =============================================================================
/*!
 *
 */
// =============================================================================
void OnOff::Client::off (Protocol::Address &addr)
{
   Protocol::Message message;

   message.itf.role   = SERVER_ROLE;
   message.itf.id     = OnOff::Client::uid ();
   message.itf.member = OFF_CMD;

   sendMessage (addr, message);
}

// =============================================================================
// OnOff::Client::toggle
// =============================================================================
/*!
 *
 */
// =============================================================================
void OnOff::Client::toggle (Protocol::Address &addr)
{
   Protocol::Message message;

   message.itf.role   = SERVER_ROLE;
   message.itf.id     = OnOff::Client::uid ();
   message.itf.member = TOGGLE_CMD;

   sendMessage (addr, message);
}
