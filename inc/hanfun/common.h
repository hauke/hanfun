// =============================================================================
/*!
 * \file       inc/hanfun/common.h
 *
 * This file contains the common defines for the HAN-FUN library.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    x.x.x
 *
 * \copyright  Copyright &copy; &nbsp; 2013 Bithium S.A.
 */
// =============================================================================

#ifndef HF_COMMON_H
#define HF_COMMON_H

#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstring>

#include <string>
#include <vector>
#include <type_traits>

#include "version.h"
#include "config.h"

#include "hanfun/gcc.h"

using namespace std;

// =============================================================================
// Defines
// =============================================================================

#define UNUSED(x)   (void) x

// =============================================================================
// API
// =============================================================================

/*!
 * Top-level namespace for the HAN-FUN library.
 */
namespace HF
{
   constexpr uint8_t CORE_VERSION       = 0;
   constexpr uint8_t PROFILES_VERSION   = 0;
   constexpr uint8_t INTERFACES_VERSION = 0;

   // =============================================================================
   // Helper Classes.
   // =============================================================================

   /*!
    * These constants represent precisions that a measurement can be in.
    */
   typedef enum
   {
      BASE  = 0x00,
      MILI  = 0x10,
      MICRO = 0x11,
      NANO  = 0x12,
      PICO  = 0x13,
      KILO  = 0x20,
      MEGA  = 0x21,
      GIGA  = 0x22,
      TERA  = 0x23,
   } Precision;

   /*!
    * This represents the type of time that is associated with a
    * time measurement.
    */
   typedef enum
   {
      UPTIME = 0x00, //!< Uptime.
      UTC    = 0x01, //!< UTC time.
   } Time;

   /*!
    * This class represents a byte array.
    *
    * The method in this class are used to serialize the messages to be sent over the
    * network, converting between the host's endianness and the big-endian network format.
    */
   struct ByteArray:private vector <uint8_t>
   {
      /*!
       * Create a byte array with the given initial size.
       *
       * @param size the initial size of the byte array.
       */
      ByteArray(size_t size = 0);

      /*!
       * Create a byte array with the given initial data.
       *
       * @param data    data to initialize the byte array with.
       * @param size    size in bytes of the data.
       */
      ByteArray(const uint8_t data[], const size_t size);

      //! Destructor
      virtual ~ByteArray() {}

      /*!
       * Write a byte into the array at the given \c offset.
       *
       * @param [in] offset  offset to write the byte to.
       * @param [in] data    byte value to write to the array.
       *
       * @return  number of bytes written (1).
       */
      size_t write (size_t offset, uint8_t data);

      /*!
       * Write a word in the big endian format into the
       * array at the given \c offset.
       *
       * \warning If the host is NOT big-endian the value will
       *          be converted before being written.
       *
       * @param [in] offset  offset to write the word to.
       * @param [in] data    word value to write to the array.
       *
       * @return  number of bytes written (2).
       */
      size_t write (size_t offset, uint16_t data);

      /*!
       * Write a double-word in big endian format into the
       * array at the given \c offset.
       *
       * \warning If the host is NOT big-endian the value will
       *          be converted before being written.
       *
       * @param [in] offset  offset to write the double-word to.
       * @param [in] data    double-word value to write to the array.
       *
       * @return  number of bytes written (4).
       */
      size_t write (size_t offset, uint32_t data);

      //! \see  ByteArray::write (size_t, uint8_t)
      size_t write (size_t offset, bool data)
      {
         return write (offset, static_cast <uint8_t>(data));
      }

      /*!
       * Read the byte at \c offset into \c data.
       *
       * @param [in]  offset  offset to read the byte from.
       * @param [out] data    reference to save the read value to.
       *
       * @return  number of bytes read (1).
       */
      size_t read (size_t offset, uint8_t &data) const;

      /*!
       * Read the word in big-endian format at \c offset into \c data.
       *
       * \warning If the host is NOT big-endian the value will
       *          be converted to the host's Endianness.
       *
       * @param [in]  offset  offset to read the word from.
       * @param [out] data    reference to save the read value to.
       *
       * @return  number of bytes read (2).
       */
      size_t read (size_t offset, uint16_t &data) const;

      /*!
       * Read the double-word in big-endian format at \c offset into \c data.
       *
       * \warning If the host is NOT big-endian the value will
       *          be converted to the host's Endianness.
       *
       * @param [in]  offset  offset to read the double-word from.
       * @param [out] data    reference to save the read value to.
       *
       * @return  number of bytes read (4).
       */
      size_t read (size_t offset, uint32_t &data) const;

      //! \see  ByteArray::read (size_t, uint8_t)
      size_t read (size_t offset, bool &data) const
      {
         uint8_t temp;
         size_t  result = read (offset, temp);

         data = (temp & 0x01) != 0;

         return result;
      }

      using vector::size;
      using vector::operator [];
      using vector::operator =;
      using vector::capacity;
      using vector::empty;
      using vector::clear;
      using vector::front;
      using vector::back;
      using vector::push_back;
      using vector::pop_back;
      using vector::begin;
      using vector::end;
      using vector::swap;
      using vector::data;
      using vector::reserve;

      bool operator ==(const ByteArray &other)
      {
         return (this->size () == other.size () &&
                 std::equal (this->begin (), this->end (), other.begin ()));
      }

      bool operator !=(const ByteArray &other)
      {
         return !(*this == other);
      }
   };

   // =============================================================================
   // Common Interfaces
   // =============================================================================

   /*!
    * This represents the common interface for message serialization.
    */
   class Serializable
   {
      public:

      //! Destructor
      virtual ~Serializable() {}

      /*!
       * Number bytes needed to serialize the message.
       *
       * @return  number of bytes the message requires to be serialized.
       */
      virtual size_t size () const = 0;

      /*!
       * Write the object on to a ByteArray so it can be sent over the network.
       *
       * @param [inout] array   ByteArray reference to write the object to.
       * @param [in]    offset  offset to start writing to.
       *
       * @return  the number of bytes written.
       */
      virtual size_t pack (ByteArray &array, size_t offset = 0) const = 0;

      /*!
       * Read a message from a ByteArray.
       *
       * @param [inout] array   ByteArray reference to read the message from.
       * @param [in]    offset  offset to start reading from.
       *
       * @return  the number of bytes read.
       */
      virtual size_t unpack (const ByteArray &array, size_t offset = 0) = 0;
   };

   /*!
    * This class represents the interface that cloneable objects need
    * to implement.
    */
   struct Cloneable
   {
      /*!
       * Create a clone object of the object where this method is being called.
       *
       * @return  a new object that is a clone of this object.
       */
      virtual Cloneable *clone () const = 0;
   };

   /*!
    * Commands result codes.
    */
   typedef enum
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
   } Result;

   // =============================================================================
   // UID implementation
   // =============================================================================

   /*!
    * Parent UID class.
    */
   struct UID:public Serializable, public Cloneable
   {
      //! Types of UID available.
      enum Type
      {
         NONE = 0x00,         //!< Empty UID.
         RFPI = 0x01,         //!< Radio Fixed Part Identifier.
         IPUI = 0x02,         //!< International Portable User Identifier.
         MAC  = 0x03,         //!< Media Access Control (IEEE-MAC-48)
         URI  = 0x04,         //!< Uniform Resource Identifier.
      };

      //! Type of the UID.
      virtual uint8_t type () const
      {
         return NONE;
      }

      //! \see HF::Serializable::size.
      virtual size_t size () const
      {
         return 1;
      }

      //! \see HF::Serializable::pack.
      virtual size_t pack (ByteArray &array, size_t offset = 0) const
      {
         size_t start = offset;
         offset += array.write (offset, (uint8_t) 0);
         return offset - start;
      }

      //! \see HF::Serializable::unpack.
      virtual size_t unpack (const ByteArray &array, size_t offset = 0)
      {
         uint8_t size;
         size_t  start = offset;
         offset += array.read (offset, size);
         return offset - start;
      }

      UID *clone () const
      {
         return new UID (*this);
      }

      bool operator ==(const HF::UID &other)
      {
         return (this->compare (&other) == 0);
      }

      bool operator !=(const HF::UID &other)
      {
         return !(*this == other);
      }

      /*!
       * Compare the current UID with the given UID.
       *
       * This function returns a value less that 0 if the current UID object
       * order is lower that the given UID, 0 if the UIDs represent the same
       * entity and a value greater that 0 if current UID object is above the given
       * UID.
       *
       * @param [in] other  a pointer to a UID object to compare to.
       *
       * @retval     < 0, the current UID is lower that the given UID.
       * @retval     0  , the current UID is the same as given UID.
       * @retval     > 0, the current UID is greater that the given UID.
       */
      virtual int compare (const UID *other) const
      {
         return (other != nullptr ? type () - other->type () : type ());
      }
   };

   template<UID::Type _type>
   struct AbstractUID:public UID
   {
      uint8_t type () const
      {
         return _type;
      }
   };

   template<typename _Class, uint8_t _size, UID::Type _type>
   struct ByteArrayUID:public AbstractUID <_type>
   {
      uint8_t value[_size];

      //! \see HF::Serializable::size.
      size_t size () const
      {
         return UID::size () + sizeof(value);
      }

      //! \see HF::Serializable::pack.
      size_t pack (ByteArray &array, size_t offset = 0) const
      {
         size_t start = offset;

         offset += array.write (offset, (uint8_t) sizeof(value));

         for (uint8_t i = 0; i < sizeof(value); i++)
         {
            offset += array.write (offset, value[i]);
         }

         return offset - start;
      }

      //! \see HF::Serializable::unpack.
      size_t unpack (const ByteArray &array, size_t offset = 0)
      {
         uint8_t size;
         size_t  start = offset;

         offset += array.read (offset, size);

         for (uint8_t i = 0; size == sizeof(value) && i < size; i++)
         {
            offset += array.read (offset, value[i]);
         }

         return offset - start;
      }

      // ===================================================================
      // API
      // ===================================================================

      int compare (const UID *other) const
      {
         int res = UID::compare (other);
         return (res == 0 ? memcmp (value, ((_Class *) other)->value, sizeof(value)) : res);
      }
   };

   /*!
    * RFPI UID class.
    */
   struct RFPI:public ByteArrayUID <RFPI, 5, UID::RFPI>
   {
      // ===================================================================
      // Cloneable
      // ===================================================================

      RFPI *clone () const
      {
         return new RFPI (*this);
      }
   };

   /*!
    * IPUI UID class.
    */
   struct IPUI:public ByteArrayUID <IPUI, 5, UID::IPUI>
   {
      // ===================================================================
      // Cloneable
      // ===================================================================

      IPUI *clone () const
      {
         return new IPUI (*this);
      }
   };

   /*!
    * IEEE MAC-48b UID class.
    */
   struct MAC:public ByteArrayUID <MAC, 6, UID::MAC>
   {
      // ===================================================================
      // Cloneable
      // ===================================================================

      MAC *clone () const
      {
         return new MAC (*this);
      }
   };

   /*!
    * URI UID class.
    */
   struct URI:public AbstractUID <UID::URI>
   {
      string value;

      URI(string value = ""):value (value) {}

      //! \see HF::Serializable::size.
      size_t size () const
      {
         return UID::size () + value.size ();
      }

      //! \see HF::Serializable::pack.
      size_t pack (ByteArray &array, size_t offset = 0) const
      {
         size_t start = offset;
         size_t size  = value.size ();

         offset += array.write (offset, (uint8_t) size);

         for (uint8_t i = 0; i < (size & 0xFF); i++)
         {
            offset += array.write (offset, (uint8_t) value[i]);
         }

         return offset - start;
      }

      //! \see HF::Serializable::unpack.
      size_t unpack (const ByteArray &array, size_t offset = 0)
      {
         uint8_t size;
         size_t  start = offset;

         offset += array.read (offset, size);

         value   = string (size, 0);

         for (uint8_t i = 0; i < size; i++)
         {
            uint8_t c;
            offset  += array.read (offset, c);
            value[i] = c;
         }

         return offset - start;
      }

      // ===================================================================
      // Cloneable
      // ===================================================================

      URI *clone () const
      {
         return new URI (*this);
      }

      // ===================================================================
      // API
      // ===================================================================

      int compare (const UID *other) const
      {
         int res = UID::compare (other);
         return (res == 0 ? value.compare (((URI *) other)->value) : res);
      }
   };

   // Forward declaration of the Unit's interface.
   struct IUnit;

   // Forward declaration of the protocol namespace.
   namespace Protocol
   {
      // Forward declaration of the protocol packet structure.
      struct Packet;

   }  // namespace Protocol

   /*!
    * This class represents the interface that all devices MUST implement.
    */
   struct IDevice
   {
      /*!
       * Return the device address on the HAN-FUN network, when the device is registered,
       * or \c HF_BROADCAST_ADDR otherwise.
       *
       * @return  the device address on the HAN-FUN network,
       *          \c HF_BROADCAST_ADDR otherwise.
       */
      virtual uint16_t address () const = 0;

      /*!
       * Return the list of units registered in this device.
       *
       * @return     vector containing the device's registered units.
       */
      virtual const vector <IUnit *> &units () const = 0;

      /*!
       * Add unit to devices unit lists.
       *
       * @param unit    pointer to the unit to add to the list.
       */
      virtual void add (IUnit *unit) = 0;

      /*!
       * Remove unit from device's unit list.
       *
       * @param unit    pointer to the unit to remove from the list.
       */
      virtual void remove (IUnit *unit) = 0;

      /*!
       * Send given \c packet into the HAN-FUN network.
       *
       * @param packet  pointer to the packet to send to the network.
       */
      virtual void send (Protocol::Packet *packet) = 0;

      /*!
       * Receive a packet from the HAN-FUN network.
       *
       * @param packet  pointer to the received packet.
       */
      virtual void receive (Protocol::Packet *packet) = 0;
   };

   // =============================================================================
   // API
   // =============================================================================

   /*!
    * Convert a \c Word from the host's format to the network format.
    *
    * @param value   word to convert.
    */
   void hf_hton (uint16_t &value);

   /*!
    * Convert a \c Double-Word from the host's format to the network format.
    *
    * @param value   double-word to convert.
    */
   void hf_hton (uint32_t &value);

   /*!
    * Convert a \c Word from the network format  to the host's format.
    *
    * @param value   word to convert.
    *
    * @return  the word converted.
    */
   void hf_ntoh (uint16_t &value);

   /*!
    * Convert a \c Double-Word from the network format to the host's format.
    *
    * @param value   double-word to convert.
    *
    * @return  the double-word converted.
    */
   void hf_ntoh (uint32_t &value);

}  // namespace HF

/*
 * This provides a template specialization to use in ordered collections
 * containing pointer to UID classes.
 */
namespace std
{
   template<>
   struct less <HF::UID *> :public binary_function <HF::UID *, HF::UID *, bool>
   {
      bool operator ()(HF::UID *lhs, HF::UID *rhs) const
      {
         if (lhs == nullptr)
         {
            return true;
         }
         else if (rhs == nullptr)
         {
            return false;
         }
         else
         {
            return lhs->compare (rhs) < 0;
         }
      }
   };
}

#endif /* HF_COMMON_H */
