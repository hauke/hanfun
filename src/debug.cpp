// =============================================================================
/*!
 * @file       src/debug.cpp
 *
 * This file contains the implementation of the debug helper functions.
 *
 * @version    1.2.0
 *
 * @copyright  Copyright &copy; &nbsp; 2014 ULE Alliance
 *
 * For licensing information, please see the file 'LICENSE' in the root folder.
 *
 * Initial development by Bithium S.A. [http://www.bithium.com]
 */
// =============================================================================

#include <iomanip>
#include <algorithm>

#include "hanfun.h"
#include "hanfun/debug.h"

using namespace HF::Debug;


// =============================================================================
// Stream Helpers
// =============================================================================

// =============================================================================
// operator <<
// =============================================================================
/*!
 *
 */
// =============================================================================
std::ostream & operator <<(std::ostream &stream, const HF::Common::ByteArray &array)
{
   if (stream == std::cout || stream == std::cerr)
   {
      std::ios_base::fmtflags ff;
      ff = stream.flags ();
      char f = stream.fill ('0');

      stream << std::noshowbase << std::hex << std::setw (2) << std::setfill ('0');
      stream << "(" << array.size () << ") ";

      /* *INDENT-OFF* */
      std::for_each (array.begin (), array.end (), [&stream](uint8_t byte)
      {
         stream << std::hex << std::setw (2) << std::setfill ('0') << (int)byte << " ";
      });
      /* *INDENT-ON* */

      stream.setf (ff);
      stream.fill (f);
   }
   else
   {
      /* *INDENT-OFF* */
      std::for_each (array.begin (), array.end (), [&stream](uint8_t byte)
      {
         stream << byte;
      });
      /* *INDENT-ON* */
   }

   return stream;
}

// =============================================================================
// operator <<
// =============================================================================
/*!
 *
 */
// =============================================================================
std::ostream &operator <<(std::ostream &stream, const HF::UID::UID &uid)
{
   std::ios_base::fmtflags ff = stream.flags ();
   char f                     = stream.fill ('0');

   switch (uid.raw ()->type ())
   {
      case HF::UID::NONE_UID:
      {
         stream << "uid: NONE";
         break;
      }
      case HF::UID::DECT_UID:
      {
         HF::UID::DECT *dect = (HF::UID::DECT *) uid.raw ();
         stream << "dect: " << std::uppercase << std::setw (2) << std::right << std::hex;

         for (uint8_t i = 0; i < HF::UID::RFPI::length (); i++)
         {
            stream << static_cast <int>((*dect)[i]);
         }

         break;
      }
      case HF::UID::MAC_UID:
      {
         HF::UID::MAC *mac = (HF::UID::MAC *) uid.raw ();
         stream << "mac: " << std::uppercase << std::setw (2) << std::right << std::hex;

         for (uint8_t i = 0; i < HF::UID::MAC::length () - 1; i++)
         {
            stream << static_cast <int>((*mac)[i]) << ":";
         }

         stream << static_cast <int>((*mac)[HF::UID::MAC::length () - 1]);

         break;
      }

      case HF::UID::URI_UID:
      {
         HF::UID::URI *uri = (HF::UID::URI *) uid.raw ();
         stream << "uri: " << uri->str ();
         break;
      }

      default:
         break;
   }

   stream.setf (ff);
   stream.fill (f);
   stream << std::nouppercase << std::left << std::dec;

   return stream;
}

// =============================================================================
// operator <<
// =============================================================================
/*!
 *
 */
// =============================================================================
std::ostream &operator <<(std::ostream &stream, const HF::Protocol::Packet &packet)
{
   std::ios_base::fmtflags ff;
   ff = stream.flags ();
   char f = stream.fill (0);

   stream << std::hex << std::setw (2);

   stream << "Packet : " << NL;

   stream << "  Source  : "
          << (int) packet.source.mod << " "
          << (int) packet.source.device << " "
          << (int) packet.source.unit << NL;

   stream << "  Dest    : "
          << (int) packet.destination.mod << " "
          << (int) packet.destination.device << " "
          << (int) packet.destination.unit << NL;

   stream << "  Message :" << NL;

   stream << "    Reference: "
          << (int) packet.message.reference << NL;

   stream << "    Type     : "
          << (int) packet.message.type << NL;

   stream << "    Interface: "
          << (int) packet.message.itf.role << " "
          << (int) packet.message.itf.id << " "
          << (int) packet.message.itf.member << NL;

   stream << "    Length   : " << std::showbase
          << (int) packet.message.length << NL;

   stream << "    Payload  : " << packet.message.payload << NL;

   stream << std::noshowbase;

   stream.setf (ff);
   stream.fill (f);

   return stream;
}
