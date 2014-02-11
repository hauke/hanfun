// =============================================================================
/*!
 * \file       src/interfaces.cpp
 *
 * This file contains the implementation of the interfaces common code.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    x.x.x
 *
 * \copyright  Copyright &copy; &nbsp; 2014 Bithium S.A.
 */
// =============================================================================

#include "hanfun/common.h"
#include "hanfun/interface.h"

using namespace HF;
using namespace HF::Protocol;

// =============================================================================
// Helper Functions
// =============================================================================


// =============================================================================
// update_attribute
// =============================================================================
/*!
 * Update the attribute with the given \c uid at given interface.
 *
 * @param [in] itf      pointer to the interface to update the attribute at.
 * @param [in] uid      the UID of the attribute to update.
 * @param [in] payload  the ByteArray containing the value to update the attribute.
 * @param [in] offset   the offset to read the payload from.
 * @param [in] nop      if \c true, do everything except change the attribute value.
 *
 * @retval Result::OK            if the attribute was updated;
 * @retval Result::FAIL_SUPPORT  if the attribute does not exist;
 * @retval Result::FAIL_RO_ATTR  if the attribute does not exist;
 * @retval Result::FAIL_UNKNOWN  otherwise.
 */
// =============================================================================
static Result update_attribute (Interface *itf, uint8_t uid, ByteArray &payload, size_t &offset,
                                bool nop = false)
{
   Result result    = Result::FAIL_UNKNOWN;

   IAttribute *attr = itf->attribute (uid);

   if (attr == nullptr)
   {
      result = Result::FAIL_SUPPORT;
   }
   else if (attr->isWritable ())
   {
      if (payload.available (offset, attr->size ()))
      {
         if (!nop)
         {
            offset += attr->unpack (payload, offset);
         }

         result = Result::OK;
      }
      else
      {
         result = Result::FAIL_UNKNOWN;
      }
   }
   else
   {
      offset += attr->size ();

      result  = Result::FAIL_RO_ATTR;
   }

   delete attr;

   return result;
}

// =============================================================================
// update_attributes
// =============================================================================
/*!
 * Update the given interface (\c itf) attributes with the values present in the
 * ByteArray \c payload.
 *
 * @param [in]    itf        the interface instance to update the attributes on.
 * @param [in]    payload    the ByteArray containing the new values for the attributes.
 * @param [inout] offset     the offset at the ByteArray to start reading the values from.
 * @param [in]    resp       boolean indicating if a response is necessary.
 * @param [in]    nop        if \c true, do everything except change the attribute value.
 *
 * @return  pointer to a SetAttributePack::Response instance if a response
 *          is necessary, \c nullptr otherwise.
 */
// =============================================================================
static SetAttributePack::Response *update_attributes (Interface *itf, ByteArray &payload,
                                                      size_t &offset, bool resp, bool nop = false)
{
   SetAttributePack::Request request;

   offset += request.unpack (payload, offset);

   SetAttributePack::Response *result = (resp ? new SetAttributePack::Response () : nullptr);

   for (uint8_t i = 0; i < request.count; i++)
   {
      SetAttributePack::Response::Result attr_res;

      offset       += payload.read (offset, attr_res.uid);

      attr_res.code = update_attribute (itf, attr_res.uid, payload, offset, nop);

      if (resp)
      {
         result->results.push_back (attr_res);
      }

      if (attr_res.code == Result::FAIL_SUPPORT || attr_res.code == Result::FAIL_UNKNOWN)
      {
         break;
      }
   }

   return result;
}

// =============================================================================
// update_attributes_atomic
// =============================================================================
/*!
 * Update the given interface (\c itf) attributes with the values present in the
 * ByteArray \c payload, if all values can be set.
 *
 * @param [in]    itf        the interface instance to update the attributes on.
 * @param [in]    payload    the ByteArray containing the new values for the attributes.
 * @param [inout] offset     the offset at the ByteArray to start reading the values from.
 * @param [in]    resp       boolean indicating if a response is necessary.
 *
 * @return  pointer to a HF::Response instance if a response is necessary, \c nullptr otherwise.
 */
// =============================================================================
static Response *update_attributes_atomic (Interface *itf, ByteArray &payload, size_t &offset,
                                           bool resp)
{
   size_t start                         = offset;

   SetAttributePack::Response *attr_res = update_attributes (itf, payload, offset, true, true);

   Result result                        = Result::OK;

   for (SetAttributePack::Response::results_t::iterator it = attr_res->results.begin ();
        it != attr_res->results.end (); ++it)
   {
      result = it->code;

      if (result != Result::OK)
      {
         break;
      }
   }

   delete attr_res;

   if (result == Result::OK)
   {
      offset = start;
      update_attributes (itf, payload, offset, false);
   }

   if (resp)
   {
      return new Response (result);
   }
   else
   {
      return nullptr;
   }
}

// =============================================================================
// AbstractInterface API.
// =============================================================================

// =============================================================================
// AbstractInterface::handle
// =============================================================================
/*!
 *
 */
// =============================================================================
Result AbstractInterface::handle (Packet &packet, ByteArray &payload, size_t offset)
{
   Result result = check_message (packet.message, payload, offset);

   if (result != Result::OK)
   {
      return result;
   }

   result = check_payload_size (packet.message, payload, offset);

   if (result != Result::OK)
   {
      return result;
   }

   if (packet.message.type >= Message::COMMAND_REQ && packet.message.type <= Message::COMMAND_RES)
   {
      return handle_command (packet, payload, offset);
   }
   else if (packet.message.type >= Message::GET_ATTR_REQ && packet.message.type <= Message::ATOMIC_SET_ATTR_PACK_RES)
   {
      return handle_attribute (packet, payload, offset);
   }

   return Result::FAIL_UNKNOWN;
}


// =============================================================================
// AbstractInterface::check_message
// =============================================================================
/*!
 *
 */
// =============================================================================
Result AbstractInterface::check_message (Message &message, ByteArray &payload, size_t offset)
{
   UNUSED (payload);
   UNUSED (offset);

   if (uid () != message.itf.uid)
   {
      return Result::FAIL_ID;
   }

   switch (message.type)
   {
      case Message::COMMAND_REQ:
      case Message::COMMAND_RESP_REQ:
      case Message::GET_ATTR_REQ:
      case Message::SET_ATTR_REQ:
      case Message::SET_ATTR_RESP_REQ:
      case Message::SET_ATTR_PACK_REQ:
      case Message::SET_ATTR_PACK_RESP_REQ:
      {
         if (role () != message.itf.role)
         {
            return Result::FAIL_SUPPORT;
         }

         break;
      }
      case Message::COMMAND_RES:
      {
         if (role () == message.itf.role)
         {
            return Result::FAIL_SUPPORT;
         }

         break;
      }
      default:
         break;
   }

   return Result::OK;
}

// =============================================================================
// AbstractInterface::check_payload_size
// =============================================================================
/*!
 *
 */
// =============================================================================
Result AbstractInterface::check_payload_size (Message &message, ByteArray &payload, size_t offset)
{
   size_t _payload_size = payload_size (message.itf);

   if (_payload_size != 0)
   {
      if (message.length < _payload_size || payload.size () < offset ||
          (payload.size () - offset) < _payload_size)
      {
         return Result::FAIL_ARG;
      }
   }

   return Result::OK;
}

// =============================================================================
// AbstractInterface::payload_size
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t AbstractInterface::payload_size (Message &message) const
{
   if (message.type == Message::GET_ATTR_PACK_REQ)
   {
      GetAttributePack::Request req;
      return req.size ();
   }
   else
   {
      return payload_size (message.itf);
   }
}

// =============================================================================
// AbstractInterface::handle_command
// =============================================================================
/*!
 *
 */
// =============================================================================
Result AbstractInterface::handle_command (Packet &packet, ByteArray &payload, size_t offset)
{
   UNUSED (packet);
   UNUSED (payload);
   UNUSED (offset);

   return Result::FAIL_UNKNOWN;
}

// =============================================================================
// AbstractInterface::handle_attributes
// =============================================================================
/*!
 *
 *
 */
// =============================================================================
Result AbstractInterface::handle_attribute (Packet &packet, ByteArray &payload, size_t offset)
{
   switch (packet.message.type)
   {
      case Message::GET_ATTR_REQ:
      {
         AttributeResponse *attr_res = new AttributeResponse (attribute (packet.message.itf.member));
         attr_res->code = (attr_res->attribute != nullptr ? Result::OK : Result::FAIL_SUPPORT);

         Message response (attr_res, packet.message);

         sendMessage (packet.source, response);
         break;
      }
      case Message::GET_ATTR_RES:
      {
         // Do nothing.
         break;
      }
      case Message::SET_ATTR_REQ:
      {
         update_attribute (this, packet.message.itf.member, payload, offset);
         break;
      }
      case Message::SET_ATTR_RESP_REQ:
      {
         Result result  = update_attribute (this, packet.message.itf.member, payload, offset);
         Response *resp = new Response (result);

         Message  response (resp, packet.message);

         sendMessage (packet.source, response);

         break;
      }
      case Message::GET_ATTR_PACK_REQ:
      {
         Result result = Result::OK;
         GetAttributePack::Request request;

         attribute_uids_t attributes;

         switch (packet.message.itf.member)
         {
            case GetAttributePack::Type::MANDATORY:
            {
               attributes = this->attributes ();
               break;
            }
            case GetAttributePack::Type::ALL:
            {
               attributes = this->attributes (true);
               break;
            }
            case GetAttributePack::Type::DYNAMIC:
            {
               offset    += request.unpack (payload, offset);
               attributes = request.attributes;
               break;
            }
            default:
               result = Result::FAIL_ARG;
               break;
         }

         GetAttributePack::Response *attr_response = new GetAttributePack::Response ();

         if (result == Result::OK)
         {
            /* *INDENT-OFF* */
            for_each (attributes.begin (), attributes.end (),
                       [attr_response, &result, this](uint8_t uid)
            {
               IAttribute *attr = attribute (uid);
               if (attr != nullptr)
               {
                  attr_response->attributes.push_back (attr);
               }
               else
               {
                  result = Result::FAIL_SUPPORT;
               }
            });
            /* *INDENT-ON* */
         }

         attr_response->code = result;

         Message response (attr_response, packet.message);

         sendMessage (packet.source, response);

         break;
      }
      case Message::GET_ATTR_PACK_RES:
      {
         // Do nothing.
         break;
      }
      case Message::SET_ATTR_PACK_REQ:
      {
         update_attributes (this, payload, offset, false);
         break;
      }
      case Message::SET_ATTR_PACK_RESP_REQ:
      {
         SetAttributePack::Response *attr_response = update_attributes (this, payload, offset, true);

         Message response (attr_response, packet.message);

         sendMessage (packet.source, response);
         break;
      }
      case Message::SET_ATTR_PACK_RES:
      {
         // Do nothing.
         break;
      }
      case Message::ATOMIC_SET_ATTR_PACK_REQ:
      {
         update_attributes_atomic (this, payload, offset, false);
         break;
      }
      case Message::ATOMIC_SET_ATTR_PACK_RESP_REQ:
      {
         Serializable *attr_response = update_attributes_atomic (this, payload, offset, true);

         Message response (attr_response, packet.message);

         sendMessage (packet.source, response);

         break;
      }
      case Message::ATOMIC_SET_ATTR_PACK_RES:
      {
         // Do nothing.
         break;
      }
      default:
         break;
   }

   return Result::OK;
}
