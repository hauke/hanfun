// =============================================================================
/*!
 * @file       inc/hanfun/common.h
 *
 * This file contains the common defines for the HAN-FUN library.
 *
 * @version    1.1.1
 *
 * @copyright  Copyright &copy; &nbsp; 2014 ULE Alliance
 *
 * For licensing information, please see the file 'LICENSE' in the root folder.
 *
 * Initial development by Bithium S.A. [http://www.bithium.com]
 */
// =============================================================================

#ifndef HF_COMMON_H
#define HF_COMMON_H

#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

#include <assert.h>

#include "hanfun/gcc.h"
#include "hanfun/version.h"
#include "hanfun/config.h"

/*!
 * @addtogroup common  Common
 * This module contains common helper classes, functions and definitions.
 */

// =============================================================================
// Defines
// =============================================================================

/*!
 * @ingroup common
 *
 * Helper macro to remove warning about unused function/method argument.
 */
#define UNUSED(x)   (void) x

// =============================================================================
// API
// =============================================================================

/*!
 * Top-level namespace for the HAN-FUN library.
 */
namespace HF
{
   /*!
    * @addtogroup common Common
    * @{
    */

   constexpr uint8_t CORE_VERSION       = 1; //!< Core Service & Interfaces major version supported.
   constexpr uint8_t PROFILES_VERSION   = 1; //!< Profiles major version supported.
   constexpr uint8_t INTERFACES_VERSION = 1; //!< Interfaces major version supported.

   /*! @} */

   /*!
    * This namespace contains helper classes to be used though out the HAN-FUN
    * implementation.
    */
   namespace Common
   {
      /*!
       * @addtogroup common
       * @{
       */
      // =============================================================================
      // Helper Classes.
      // =============================================================================

      /*!
       * These constants represent precisions that a measurement can be in.
       */
      typedef enum _Precision
      {
         BASE  = 0x00, //!< Base unit.
         MILI  = 0x10, //!< Mili
         MICRO = 0x11, //!< Micro
         NANO  = 0x12, //!< Nano
         PICO  = 0x13, //!< Pico
         KILO  = 0x20, //!< Kilo
         MEGA  = 0x21, //!< Mega
         GIGA  = 0x22, //!< Giga
         TERA  = 0x23, //!< Tera
      } Precision;

      /*!
       * This represents the type of time that is associated with a
       * time measurement.
       */
      typedef enum _Time
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
      struct ByteArray:public std::vector <uint8_t>
      {
         /*!
          * Create a byte array with the given initial size.
          *
          * @param [in] size the initial size of the byte array.
          */
         ByteArray(size_t size = 0);

         /*!
          * Create a byte array with the given initial data.
          *
          * @param [in] data    data to initialize the byte array with.
          * @param [in] size    size in bytes of the data.
          */
         ByteArray(const uint8_t data[], const size_t size);

         /*!
          * Create byte array from the values in the given list.
          *
          * @param [in] raw  values to add to the byte array.
          */
         ByteArray(std::initializer_list <uint8_t> raw):vector (raw)
         {}

         //! Destructor
         virtual ~ByteArray() {}

         /*!
          * Write a byte into the array at the given @c offset.
          *
          * @param [in] offset  offset to write the byte to.
          * @param [in] data    byte value to write to the array.
          *
          * @return  number of bytes written (1).
          */
         size_t write (size_t offset, uint8_t data);

         /*!
          * Write a word in the big endian format into the
          * array at the given @c offset.
          *
          * @param [in] offset  offset to write the word to.
          * @param [in] data    word value to write to the array.
          *
          * @return  number of bytes written (2).
          */
         size_t write (size_t offset, uint16_t data);

         /*!
          * Write a double-word in big endian format into the
          * array at the given @c offset.
          *
          * @param [in] offset  offset to write the double-word to.
          * @param [in] data    double-word value to write to the array.
          *
          * @return  number of bytes written (4).
          */
         size_t write (size_t offset, uint32_t data);

         //! @copydoc ByteArray::write (size_t, uint8_t)
         size_t write (size_t offset, bool data)
         {
            return write (offset, static_cast <uint8_t>(data));
         }

         /*!
          * Read the byte at @c offset into @c data.
          *
          * @param [in]  offset  offset to read the byte from.
          * @param [out] data    reference to save the read value to.
          *
          * @return  number of bytes read (1).
          */
         size_t read (size_t offset, uint8_t &data) const;

         /*!
          * Read the word in big-endian format at @c offset into @c data.
          *
          * @param [in]  offset  offset to read the word from.
          * @param [out] data    reference to save the read value to.
          *
          * @return  number of bytes read (2).
          */
         size_t read (size_t offset, uint16_t &data) const;

         /*!
          * Read the double-word in big-endian format at @c offset into @c data.
          *
          * @param [in]  offset  offset to read the double-word from.
          * @param [out] data    reference to save the read value to.
          *
          * @return  number of bytes read (4).
          */
         size_t read (size_t offset, uint32_t &data) const;

         //! @copydoc  ByteArray::read (size_t, uint8_t)
         size_t read (size_t offset, bool &data) const
         {
            uint8_t temp;
            size_t  result = read (offset, temp);

            data = (temp & 0x01) != 0;

            return result;
         }

         /*!
          * Check if the array as at least @c expected bytes
          * available from the given @c offset.
          *
          * @param [in] offset     the offset from where to start counting.
          * @param [in] expected   the number of byte required.
          *
          * @retval  true if enough data is available,
          * @retval  false otherwise.
          */
         bool available (size_t offset, size_t expected) const
         {
            return expected <= available (offset);
         }

         /*!
          * Return the number of data bytes available from the given @c offset.
          *
          * @param [in] offset   the offset from where to start counting.
          *
          * @return  number of data bytes available from the given @c offset.
          */
         size_t available (size_t offset) const
         {
            return (size () >= offset ? size () - offset : 0);
         }

         bool operator ==(const ByteArray &other)
         {
            return (this->size () == other.size () &&
                    std::equal (this->begin (), this->end (), other.begin ()));
         }

         bool operator !=(const ByteArray &other)
         {
            return !(*this == other);
         }

         /*!
          * Extend the byte array by the given size.
          *
          * This is the same as calling : array.reserve(array.size() + _size)
          *
          * @param [in] _size number of bytes to extent the array by.
          */
         void extend (size_t _size)
         {
            vector <uint8_t>::reserve (size () + _size);
         }
      };

      // =============================================================================
      // Common Interfaces
      // =============================================================================

      /*!
       * This represents the common interface for message serialization.
       */
      struct Serializable
      {
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
          * @param [in] array   ByteArray reference to read the message from.
          * @param [in] offset  offset to start reading from.
          *
          * @return  the number of bytes read.
          */
         virtual size_t unpack (const ByteArray &array, size_t offset = 0) = 0;
      };

      /*!
       * Wrapper for classes that implement the Serializable concept.
       */
      template<typename T, typename = void>
      struct SerializableHelper:public Serializable
      {
         T data;

         SerializableHelper()
         {
            memset (&data, 0, sizeof(T));
         }

         SerializableHelper(T data):data (data)
         {}

         size_t size () const
         {
            return data.size ();
         }

         size_t pack (ByteArray &array, size_t offset = 0) const
         {
            return data.pack (array, offset);
         }

         size_t unpack (const ByteArray &array, size_t offset = 0)
         {
            return data.unpack (array, offset);
         }

         int compare (const SerializableHelper <T> &other) const
         {
            return data.compare (other.data);
         }

         float changed (const SerializableHelper <T> &other) const
         {
            return data.changed (other.data);
         }
      };

      /*!
       * Wrapper to pointers for classes that implement the Serializable concept.
       */
      template<typename T>
      struct SerializableHelper <T, typename std::enable_if <std::is_pointer <T>::value>::type> :
         public Serializable
      {
         T data;

         SerializableHelper()
         {
            data = nullptr;
         }

         SerializableHelper(T data):data (data)
         {}

         size_t size () const
         {
            return data->size ();
         }

         size_t pack (Common::ByteArray &array, size_t offset = 0) const
         {
            return data->pack (array, offset);
         }

         size_t unpack (const Common::ByteArray &array, size_t offset = 0)
         {
            return data->unpack (array, offset);
         }

         int compare (const SerializableHelper <T> &other) const
         {
            return data->compare (other.data);
         }

         float changed (const SerializableHelper <T> &other) const
         {
            return data->changed (other.data);
         }
      };

      /*!
       * Wrapper for base integer types implementing the  Serializable API.
       */
      template<typename T>
      struct SerializableHelper <T, typename std::enable_if <std::is_integral <typename std::remove_reference <T>::type>::value>::type> :
         public Common::Serializable
      {
         T data;

         SerializableHelper()
         {
            data = 0;
         }

         SerializableHelper(T data):data (data) {}

         size_t size () const
         {
            return sizeof(T);
         }

         size_t pack (Common::ByteArray &array, size_t offset = 0) const
         {
            size_t start = offset;

            offset += array.write (offset, data);

            return offset - start;
         }

         size_t unpack (const Common::ByteArray &array, size_t offset = 0)
         {
            size_t start = offset;

            offset += array.read (offset, data);

            return offset - start;
         }

         int compare (const SerializableHelper <T> &other) const
         {
            return data - other.data;
         }

         float changed (const SerializableHelper <T> &other) const
         {
            return (((float) (data - other.data)) / other.data);
         }
      };

      template<>
      struct SerializableHelper <Common::ByteArray> :
         public Common::Serializable
      {
         Common::ByteArray data;

         SerializableHelper()
         {}

         SerializableHelper(Common::ByteArray _data):data (_data) {}

         size_t size () const
         {
            return sizeof(uint8_t) + data.size ();
         }

         size_t pack (Common::ByteArray &array, size_t offset = 0) const
         {
            size_t start = offset;

            offset += array.write (offset, (uint8_t) data.size ());

            auto it = array.begin ();
            std::advance (it, offset);

            std::copy (data.begin (), data.end (), it);

            offset += data.size ();

            return offset - start;
         }

         size_t unpack (const Common::ByteArray &array, size_t offset = 0)
         {
            size_t  start = offset;

            uint8_t _size = 0;
            offset += array.read (offset, _size);

            auto it = array.begin ();
            std::advance (it, offset);

            std::copy_n (it, _size, data.begin ());

            offset += _size;

            return offset - start;
         }

         int compare (const SerializableHelper <Common::ByteArray> &other) const
         {
            int res = data.size () - other.size ();

            if (res == 0)
            {
               return memcmp (data.data (), other.data.data (), data.size ());
            }

            return res;
         }

         float changed (const SerializableHelper <Common::ByteArray> &other) const
         {
            UNUSED (other);
            return 0.0;
         }
      };

      template<>
      struct SerializableHelper <std::string> :
         public Common::Serializable
      {
         std::string data;

         SerializableHelper()
         {}

         SerializableHelper(std::string _data):data (_data) {}

         size_t size () const
         {
            return sizeof(uint8_t) + data.size ();
         }

         size_t pack (Common::ByteArray &array, size_t offset = 0) const
         {
            size_t start = offset;

            offset += array.write (offset, (uint8_t) data.size ());

            auto it = array.begin ();
            std::advance (it, offset);

            std::copy (data.begin (), data.end (), it);

            offset += data.size ();

            return offset - start;
         }

         size_t unpack (const Common::ByteArray &array, size_t offset = 0)
         {
            size_t  start = offset;

            uint8_t _size = 0;
            offset += array.read (offset, _size);

            auto it = array.begin ();
            std::advance (it, offset);

            data.resize (_size);

            std::copy_n (it, _size, data.begin ());

            offset += _size;

            return offset - start;
         }

         int compare (const SerializableHelper <std::string> &other) const
         {
            return strcmp (data.data (), other.data.data ());
         }

         float changed (const SerializableHelper <std::string> &other) const
         {
            UNUSED (other);
            return 0.0;
         }
      };

      /*!
       * This class represents the interface that cloneable objects need
       * to implement.
       */
      template<typename T>
      struct Cloneable
      {
         /*!
          * Create a clone object of the object where this method is being called.
          *
          * @return  a new object that is a clone of this object.
          */
         virtual T *clone () const = 0;
      };

      /*!
       * Commands result codes.
       */
      typedef enum _Result
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

      /*!
       * Interface UID.
       */
      struct Interface
      {
         uint16_t role : 1;         //!< Interface role : Server or Client.
         uint16_t id   : 15;        //!< Identifier of the interface. @see Interface::UID.

         /*!
          * Constructor
          *
          * @param [in] id    interface UID.
          * @param [in] role  interface role.
          */
         Interface(uint16_t id = 0, uint8_t role = 0):
            role (role), id (id)
         {}

         //! @copydoc HF::Common::Serializable::size
         size_t size () const;

         //! @copydoc HF::Common::Serializable::pack
         size_t pack (Common::ByteArray &array, size_t offset = 0) const;

         //! @copydoc HF::Common::Serializable::unpack
         size_t unpack (const Common::ByteArray &array, size_t offset = 0);
      };

      // =============================================================================
      // Operators
      // =============================================================================

      inline bool operator ==(const Interface &lhs, const Interface &rhs)
      {
         return (lhs.role == rhs.role) && (lhs.id == rhs.id);
      }

      inline bool operator !=(const Interface &lhs, const Interface &rhs)
      {
         return !(lhs == rhs);
      }

      inline bool operator <(Interface const &lhs, Interface const &rhs)
      {
         return (lhs.role < rhs.role) || (lhs.role == rhs.role && lhs.id < rhs.id);
      }

      // =============================================================================
      // Helper Classes
      // =============================================================================

      /*!
       * Simple raw pointer wrapper.
       */
      template<typename T>
      class Pointer
      {
         T *pointer;

         bool owner;

         public:

         /*!
          * Constructor
          *
          * @param [in] _pointer    pointer to wrap.
          * @param [in] _owner      boolean indicating if the underling pointer should
          *                         be deleted when this object is deleted.
          */
         Pointer(T *_pointer = nullptr, bool _owner = false):
            pointer (_pointer), owner (_owner)
         {}

         /*!
          * Constructor.
          *
          * @param [in] _pointer    pointer to wrap.
          */
         Pointer(T &_pointer):
            pointer (&_pointer), owner (false)
         {}

         /*!
          * Move constructor.
          *
          * @param [in] other original object.
          */
         Pointer(Pointer <T> &&other):pointer (nullptr), owner (false)
         {
            std::swap (this->pointer, other.pointer);
            std::swap (this->owner, other.owner);
         }

         /*!
          * Copy constructor.
          *
          * @param [in] other original object.
          */
         Pointer(Pointer <T> &other):pointer (other.pointer), owner (other.owner)
         {
            other.owner = false;
         }

         ~Pointer()
         {
            if (owner)
            {
               delete pointer;
            }
         }

         T &operator *() const
         {
            return *pointer;
         }

         T *operator ->() const
         {
            return pointer;
         }

         bool operator ==(const Pointer <T> &other) const
         {
            return pointer == other.pointer;
         }

         bool operator !=(const Pointer <T> &other) const
         {
            return !(*this == other);
         }

         bool operator ==(const T *other) const
         {
            return pointer == other;
         }

         bool operator !=(const T *other) const
         {
            return pointer != other;
         }

         bool operator ==(const T &other) const
         {
            return pointer == &other;
         }

         bool operator !=(const T &other) const
         {
            return pointer != &other;
         }

         /*!
          * Assignment move operator.
          *
          * @param [in] other object to move from.
          *
          * @return  reference to this object.
          */
         Pointer <T> &operator =(Pointer <T> &&other)
         {
            if (this->owner)
            {
               delete this->pointer;
            }

            this->pointer = other.pointer;
            this->owner   = other.owner;

            other.owner   = false;

            return *this;
         }
      };

      /*!
       * Basic API for persistent storage implementations.
       */
      template<typename T>
      struct IEntries
      {
         // =============================================================================
         // API
         // =============================================================================

         /*!
          * Return the number of entries in the container.
          *
          * @return  the number of entries in the container.
          */
         virtual size_t size () const = 0;

         /*!
          * Store the given bind @c entry to persistent storage.
          *
          * @param [in] entry the bind entry to store.
          *
          * @retval  Common::Result::OK if the bind entry was saved,
          * @retval  Common::Result::FAIL_UNKNOWN otherwise.
          */
         virtual Result save (const T &entry) = 0;

         /*!
          * Destroy the given @c entry in the persistent storage.
          *
          * @param [in] entry   reference to the bind entry to erase.
          *
          * @retval  Common::Result::OK, if the entry was destroyed.
          * @retval  Common::Result::FAIL_ARG otherwise.
          */
         virtual Result destroy (const T &entry) = 0;
      };

      /*!
       * Convert a percentage value into the [0,std::numeric_limits<T>::max()] range.
       *
       * @param [in] value    the percentage value to convert.
       * @tparam T   integer type to convert to.
       *
       * @return  the value equivalent to the percentage in the
       *          [0,std::numeric_limits<T>::max()] range.
       */
      template<typename T, typename P = float>
      T from_percent (P value)
      {
         return static_cast <T>(round ((static_cast <float>(value) * std::numeric_limits <T>::max ()) / 100.0));
      }

      /*!
       * Convert a value in the [0,std::numeric_limits<T>::max()] range into a
       * percentage.
       *
       * @param [in] value    the value to convert to the percentage.
       * @tparam T   integer type to convert from.
       *
       * @return  the percentage value.
       */
      template<typename T, typename P = float>
      P to_percent (T value)
      {
         return static_cast <P>(round ((static_cast <float>(value) * 100.0) / std::numeric_limits <T>::max ()));
      }

      /*! @} */
   }  // namespace Common

}  // namespace HF


#endif /* HF_COMMON_H */
