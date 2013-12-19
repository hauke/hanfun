// =============================================================================
/*!
 * \file       inc/hanfun/protocol.h
 *
 * This file contains the definitions for the HAN-FUN protocol messages.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    x.x.x
 *
 * \copyright  Copyright &copy; &nbsp; 2013 Bithium S.A.
 */
// =============================================================================
#ifndef HF_PROTOCOL_H
#define HF_PROTOCOL_H

#include <iostream>

#include <vector>

#include "hanfun/common.h"

// =============================================================================
// API
// =============================================================================

namespace HF
{
   /*!
    * HAN-FUN Protocol implementation.
    */
   namespace Protocol
   {
      //! HAN-FUN Broadcast - device address.
      constexpr uint16_t BROADCAST_ADDR = 0x7FFF;

      //! HAN-FUN Broadcast - unit address.
      constexpr uint8_t BROADCAST_UNIT = 0xFF;

      //! HAN-FUN Network - Maximum application payload length.
      constexpr uint16_t MAX_PAYLOAD = 0x01FF;

      // =============================================================================
      // Classes
      // =============================================================================

      /*!
       * Network Message.
       */
      struct Message:public Serializable
      {
         /*!
          * Message types.
          */
         enum Type
         {
            COMMAND_REQ            = 0x01, //!< Command request
            COMMAND_RESP_REQ       = 0x02, //!< Command request with response required.
            COMMAND_RES            = 0x03, //!< Command response.

            GET_ATTR_REQ           = 0x04, //!< Get attributes request.
            GET_ATTR_RES           = 0x05, //!< Get attributes response.

            SET_ATTR_REQ           = 0x06, //!< Set attributes request.
            SET_ATTR_RESP_REQ      = 0x07, //!< Set attributes request with response required.
            SET_ATTR_RES           = 0x08, //!< Set attributes response.

            GET_ATTR_PACK_REQ      = 0x09, //!< Get pack attributes request.
            GET_ATTR_PACK_RES      = 0x0A, //!< Get pack attributes response.

            SET_ATTR_PACK_REQ      = 0x0B, //!< Set pack attributes request.
            SET_ATTR_PACK_RESP_REQ = 0x0C, //!< Set pack attributes request with response required.
            SET_ATTR_PACK_RES      = 0x0D, //!< Set pack attributes response.
         };

         /*!
          * Network Address.
          */
         struct Address:public Serializable
         {
            uint16_t mod    : 1;    //!< Address modifier.
            uint16_t device : 15;   //!< Device Address.

            uint8_t  unit;          //!< Source Unit.

            /*!
             * HAN-FUN Network Destination Address Types.
             */
            enum DestinationType
            {
               DEVICE_ADDR = 0,   //!< Destination address is for single device.
               GROUP_ADDR  = 1,   //!< Destination address is for a group of devices.
            };

            /*!
             * Create a new message address.
             *
             * @param _dev    device address. Default \c HF_BROADCAST_ADDR.
             * @param _unit   unit address. Default \c HF_BROADCAST_UNIT.
             * @param _mod    address modifier. Default \c DEVICE_ADDR.
             */
            Address(uint16_t _dev = BROADCAST_ADDR, uint8_t _unit = BROADCAST_UNIT,
                    DestinationType _mod = DEVICE_ADDR)
               :mod (_mod), device (_dev), unit (_unit)
            {}

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         /*!
          * Interface Address.
          */
         struct Interface:public Serializable
         {
            uint16_t role : 1;         //!< Interface role : Server or Client.
            uint16_t uid  : 15;        //!< Identifier of the interface. \see Interface::UID.

            uint8_t  member;           //!< Interface destination member.

            Interface(uint16_t role = 0, uint16_t uid = 0, uint8_t member = 0):
               role (role), uid (uid), member (member) {}

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         // =============================================================================
         // API
         // =============================================================================

         uint8_t   reference;          //!< Application reference.
         Type      type;               //!< Message type. \see Message::Type

         Interface itf;                //!< Interface Address.

         /*!
          * Message payload.
          *
          * This attribute contains a pointer to the message payload.
          *
          * \warning    No deallocation is performed on this pointer when the
          *             message destructor is called.
          */
         Serializable *payload;

         //! The payload length value read when unpacking the message.
         uint16_t length;

         Message(Type _type, Interface itf, Serializable *payload = nullptr):
            reference (0), type (_type), itf (itf), payload (payload), length (0) {}

         Message(Type _type = COMMAND_REQ):
            reference (0), type (_type), payload (nullptr), length (0) {}

         //! \see HF::Serializable::size.
         size_t size () const;

         //! \see HF::Serializable::pack.
         size_t pack (ByteArray &array, size_t offset = 0) const;

         //! \see HF::Serializable::unpack.
         size_t unpack (const ByteArray &array, size_t offset = 0);

         protected:

         uint16_t payload_length () const
         {
            return ((payload != nullptr) ? payload->size () : 0) % MAX_PAYLOAD;
         }
      };

      /*!
       * HAN-FUN Protocol Packet.
       */
      struct Packet:public Serializable
      {
         Message::Address source;           //!< Source Address.
         Message::Address destination;      //!< Destination Address.

         /*!
          * Packet message payload;
          */
         Message message;

         Packet() {}

         Packet(Message &message):message (message) {}

         Packet(Message::Address &dst_addr, Message &message, uint8_t unit = BROADCAST_UNIT):
            destination (dst_addr), message (message)
         {
            source.mod    = Message::Address::DEVICE_ADDR;
            source.device = BROADCAST_ADDR;
            source.unit   = unit;
         }

         Packet(Message::Address &src_addr, Message::Address &dst_addr, Message &message):
            source (src_addr), destination (dst_addr), message (message) {}

         //! \see HF::Serializable::size.
         size_t size () const;

         //! \see HF::Serializable::pack.
         size_t pack (ByteArray &array, size_t offset = 0) const;

         //! \see HF::Serializable::unpack.
         size_t unpack (const ByteArray &array, size_t offset = 0);
      };

      /*!
       * HAN-FUN Response message.
       */
      struct Response:public Serializable
      {
         /*!
          * Message response codes.
          */
         enum Code
         {
            OK                = 0x00, //!< Request OK
            FAIL_AUTH         = 0x01, //!< Fail - Not Authorized
            FAIL_ARG          = 0x02, //!< Fail - Invalid Argument
            FAIL_SUPPORT      = 0x03, //!< Fail - Not Supported
            FAIL_RO_ATTR      = 0x04, //!< Fail - Read only attribute.
            FAIL_READ_SESSION = 0x20, //!< Fail - Read Session not established
            FAIL_MODIFIED     = 0x21, //!< Fail - Entries table modified
            FAIL_ID           = 0x23, //!< Fail - ID Not Found
            FAIL_RESOURCES    = 0xFE, //!< Fail - Not enough resources
            FAIL_UNKNOWN      = 0xFF, //!< Fail - Unknown reason
         };

         // =============================================================================
         // API
         // =============================================================================

         Code code;

         Response(Code code = OK):code (code) {}

         //! \see HF::Serializable::size.
         size_t size () const;

         //! \see HF::Serializable::pack.
         size_t pack (ByteArray &array, size_t offset = 0) const;

         //! \see HF::Serializable::unpack.
         size_t unpack (const ByteArray &array, size_t offset = 0);
      };

   }  // namespace Protocol

}  // namespace HF

#endif /* HF_PROTOCOL_H */