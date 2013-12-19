// =============================================================================
/*!
 * \file       src/protocol.cpp
 *
 * This file contains the implementation of the classes for the protocol layer
 * in the HAN-FUN specification.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    x.x.x
 *
 * \copyright  Copyright &copy; &nbsp; 2013 Bithium S.A.
 */
// =============================================================================

#include <iostream>

#include "hanfun/protocol.h"
#include "hanfun/interface.h"

using namespace HF;
using namespace HF::Protocol;

// =============================================================================
// Message / Address
// =============================================================================

// =============================================================================
// Message::Address::size
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t Message::Address::size () const
{
   return sizeof(uint16_t) + // Device Address + Flag.
          sizeof(uint8_t);   // Unit Address.
}

// =============================================================================
// Message::Address::pack
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t Message::Address::pack (ByteArray &array, size_t offset) const
{
   size_t start = offset;

   uint16_t dev = ((this->mod & 0x01) << 15) | (this->device & BROADCAST_ADDR);

   offset += array.write (offset, dev);
   offset += array.write (offset, unit);

   return offset - start;
}

// =============================================================================
// Message::Address::unpack
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t Message::Address::unpack (const ByteArray &array, size_t offset)
{
   uint16_t dev;
   size_t   start = offset;

   offset      += array.read (offset, dev);

   this->mod    = (dev & ~BROADCAST_ADDR) >> 15;
   this->device = dev & BROADCAST_ADDR;

   offset      += array.read (offset, this->unit);

   return offset - start;
}

// =============================================================================
// Message / Interface
// =============================================================================

// =============================================================================
// Message::Interface::::size
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t Message::Interface::size () const
{
   return sizeof(uint16_t) +  // Interface UID.
          sizeof(uint8_t);    // Interface Member.
}

// =============================================================================
// Message::Interface::pack
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t Message::Interface::pack (ByteArray &array, size_t offset) const
{
   size_t start = offset;

   uint16_t uid = ((this->role & 0x01) << 15) | (this->uid & HF::Interface::MAX_UID);

   offset += array.write (offset, uid);
   offset += array.write (offset, this->member);

   return offset - start;
}

// =============================================================================
// Message::Interface::unpack
// =============================================================================
/*!
 *
 */
// =============================================================================
size_t Message::Interface::unpack (const ByteArray &array, size_t offset)
{
   size_t start = offset;

   uint16_t uid = 0;
   offset    += array.read (offset, uid);

   this->role = (uid & ~HF::Interface::MAX_UID) >> 15;
   this->uid  = uid & HF::Interface::MAX_UID;

   offset    += array.read (offset, this->member);

   return offset - start;
}

// =============================================================================
// Message
// =============================================================================

size_t Message::size () const
{
   return sizeof(uint8_t) +   // Application Reference.
          sizeof(uint8_t) +   // Message Type.
          itf.size () +       // Interface Addr.
          sizeof(uint16_t) +  // Payload Length Value.
          payload_length ();  // Payload Length.
}

size_t Message::pack (ByteArray &array, size_t offset) const
{
   size_t start = offset;

   // Application Reference.
   offset += array.write (offset, this->reference);

   // Message Type.
   offset += array.write (offset, static_cast <uint8_t>(this->type));

   // Interface Address
   offset += itf.pack (array, offset);

   // Payload Length.
   uint16_t length = payload_length ();

   offset += array.write (offset, length);

   if (payload != nullptr)
   {
      offset += payload->pack (array, offset);
   }

   return offset - start;
}

size_t Message::unpack (const ByteArray &array, size_t offset)
{
   size_t start = offset;

   // Application Reference.
   offset += array.read (offset, this->reference);

   // Message Type.
   uint8_t type = 0;
   offset    += array.read (offset, type);
   this->type = static_cast <Type>(type);

   // Interface Address
   offset += itf.unpack (array, offset);

   // Payload Length.
   offset       += array.read (offset, this->length);
   this->length &= MAX_PAYLOAD;

   return offset - start;
}

// =============================================================================
// Response
// =============================================================================

size_t Response::size () const
{
   return sizeof(uint8_t);
}

size_t Response::pack (ByteArray &array, size_t offset) const
{
   size_t start = offset;

   offset += array.write (offset, static_cast <uint8_t>(this->code));

   return offset - start;
}

size_t Response::unpack (const ByteArray &array, size_t offset)
{
   size_t  start = offset;

   uint8_t code  = 0;
   offset    += array.read (offset, code);

   this->code = static_cast <Code>(code);

   return offset - start;
}

// =============================================================================
// Packet
// =============================================================================

size_t Packet::size () const
{
   return source.size () +       // Source Address.
          destination.size () +  // Destination Address.
          sizeof(uint16_t) +     // Transport header layer header.
          message.size ();       // Message payload size.
}

size_t Packet::pack (ByteArray &array, size_t offset) const
{
   uint16_t transport = 0;
   size_t   start     = offset;

   offset += source.pack (array, offset);
   offset += destination.pack (array, offset);

   offset += array.write (offset, transport);

   offset += message.pack (array, offset);

   return offset - start;
}

size_t Packet::unpack (const ByteArray &array, size_t offset)
{
   uint16_t transport = 0;
   size_t   start     = offset;

   offset += source.unpack (array, offset);
   offset += destination.unpack (array, offset);

   offset += array.read (offset, transport);

   offset += message.unpack (array, offset);

   return offset - start;
}