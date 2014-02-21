// =============================================================================
/*!
 * \file       inc/hanfun/core/device_mangement.h
 *
 * This file contains the definitions for the core Device Management Interface
 * of the HAN-FUN protocol.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    x.x.x
 *
 * \copyright  Copyright &copy; &nbsp; 2013 Bithium S.A.
 */
// =============================================================================
#ifndef HF_DEVICE_MANGEMENT_H
#define HF_DEVICE_MANGEMENT_H

#include <algorithm>
#include <map>
#include <vector>

#include "hanfun/common.h"
#include "hanfun/protocol.h"

#include "hanfun/core.h"

namespace HF
{
   namespace Core
   {
      struct DeviceManagementServer;

      IAttribute *create_attribute (DeviceManagementServer * server, uint8_t uid);

      /*!
       * Parent class for the Device Management interface implementation.
       */
      struct DeviceManagement:public Service <Interface::DEVICE_MANAGEMENT>
      {
         static constexpr uint16_t START_ADDR = 0x0001;

         //! Commands.
         typedef enum
         {
            REGISTER_CMD      = 0x01, //!< Register device command.
            DEREGISTER_CMD    = 0x02, //!< De-register device command.
            START_SESSION_CMD = 0x03, //!< Start Session Read Registration Info.
            END_SESSION_CMD   = 0x04, //!< End Session Read Registration Info.
            GET_ENTRIES_CMD   = 0x05, //!< Get Entries Command.
         } CMD;

         //! Attributes.
         typedef enum
         {
            NUMBER_OF_ENTRIES_ATTR = 0x01,   //!< Number of entries attribute.
         } Attributes;

         // =============================================================================

         /*!
          * Optional interface entry.
          */
         struct Interface:public Serializable
         {
            uint16_t role : 1;    //!< Interface role. \see Interface::Role.
            uint16_t uid  : 15;   //!< Interface UID. \see Interface::UID.

            Interface(uint16_t role = 0, uint16_t uid = 0):
               role (role), uid (uid)
            {}

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         /*!
          * Unit Entry
          */
         struct Unit:public Serializable
         {
            uint8_t            id;      //!< Unit Id.
            uint16_t           profile; //!< Unit UID. \see IProfile::UID.

            vector <Interface> opt_ift;    //!< Optional interfaces.

            Unit(uint8_t id = 0, uint16_t profile = 0):
               id (id), profile (profile)
            {}

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         /*!
          * Device Entry.
          */
         struct Device:public Serializable
         {
            uint16_t      address; //!< Device Address.
            vector <Unit> units;   //!< Unit list of the interface.

            uint16_t      emc; //! Device EMC if applicable, 0 otherwise.

            HF::UID       *uid; //! Device UID.

            Device():address (Protocol::BROADCAST_ADDR), emc (0), uid (nullptr) {}

            virtual ~Device()
            {
               delete uid;
            }

            // =============================================================================
            // Serializable API
            // =============================================================================

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);

            // =============================================================================
            // Operators
            // =============================================================================

            bool operator ==(Device &other)
            {
               if (this->address != other.address || this->uid != other.uid)
               {
                  return false;
               }
               else
               {
                  return true;
               }
            }

            bool operator !=(Device &other)
            {
               return !(*this == other);
            }
         };

         // =============================================================================
         // Register Command Messages
         // =============================================================================

         /*!
          * Register command message.
          */
         struct RegisterMessage:public Serializable
         {
            uint16_t      emc;   //! Device EMC if applicable, 0 otherwise.
            vector <Unit> units; //! Device units listing.

            RegisterMessage(uint16_t emc = 0x0000):
               emc (emc), _uid (nullptr)
            {}

            virtual ~RegisterMessage();

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);

            HF::UID *uid () const
            {
               return _uid;
            }

            void uid (HF::UID *uid)
            {
               if (_uid != nullptr)
               {
                  delete _uid;
               }

               _uid = nullptr;

               if (uid != nullptr)
               {
                  _uid = uid->clone ();
               }
            }

            protected:

            HF::UID *_uid;     //! Device UID.
         };

         /*!
          * Register command response.
          */
         struct RegisterResponse:public Protocol::Response
         {
            uint16_t address;    //!< Address for the device to use.
            uint16_t emc;        //!< HAN-FUN Concentrator EMC if applicable.

            RegisterResponse(uint16_t address = 0, uint16_t emc = 0):
               address (address & Protocol::BROADCAST_ADDR), emc (emc)
            {}

            //! \see HF::Protocol::Response::size.
            size_t size () const;

            //! \see HF::Protocol::Response::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Protocol::Response::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         // =============================================================================
         // De-register Command Messages.
         // =============================================================================

         /*!
          * De-register command message.
          */
         struct DeregisterMessage:public Serializable
         {
            uint16_t address;    //!< Address of the device to de-register.

            DeregisterMessage(uint16_t address = 0):
               address (address)
            {}

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         // =============================================================================
         // Read Session Messages
         // =============================================================================

         /*!
          * Start Read Session Command Message.
          */
         struct StartSessionResponse:public Protocol::Response
         {
            uint16_t count;   //!< Number of device entries.

            StartSessionResponse(uint16_t count = 0):
               count (count)
            {}

            //! \see HF::Protocol::Response::size.
            size_t size () const;

            //! \see HF::Protocol::Response::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Protocol::Response::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         /*!
          * Get Entries Command Message.
          */
         struct GetEntriesMessage:public Serializable
         {
            uint16_t offset; //! Start index for the first entry to be provided.
            uint8_t  count;  //! Number of entries to be sent in the response.

            GetEntriesMessage(uint16_t offset = 0, uint8_t count = 0):
               offset (offset), count (count)
            {}

            //! \see HF::Serializable::size.
            size_t size () const;

            //! \see HF::Serializable::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Serializable::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         struct GetEntriesResponse:public Protocol::Response
         {
            vector <Device> entries;

            //! \see HF::Protocol::Response::size.
            size_t size () const;

            //! \see HF::Protocol::Response::pack.
            size_t pack (ByteArray &array, size_t offset = 0) const;

            //! \see HF::Protocol::Response::unpack.
            size_t unpack (const ByteArray &array, size_t offset = 0);
         };

         // =============================================================================
         // API
         // =============================================================================

         IAttribute * create_attribute(uint8_t uid)
         {
            return Core::create_attribute( (DeviceManagementServer *)nullptr, uid);
         }

         // =============================================================================

         protected:

         DeviceManagement(IDevice *_device):
            Service (_device)
         {}
      };

      /*!
       * Device Management interface : Client side.
       */
      class DeviceManagementClient:public ServiceRole <DeviceManagement, Interface::CLIENT_ROLE>
      {
         protected:

         uint16_t _address;   //! Device HAN-FUN Address.

         public:

         DeviceManagementClient(IDevice *_device):
            ServiceRole (_device), _address (Protocol::BROADCAST_ADDR)
         {}

         /*!
          * Return the address given by the HF Concentrator to the Device.
          *
          * @return  the device HAN-FUN address.
          */
         virtual uint16_t address () const
         {
            return _address;
         }

         // ======================================================================
         // Commands
         // ======================================================================
         //! \name Commands
         //! @{

         /*!
          * Send a register message.
          */
         void register_device ();

         /*!
          * Send a de-register message for the given \c address.
          *
          * @param [in] address    the address of the device to de-register.
          */
         void deregister (uint16_t address);

         void deregister ()
         {
            deregister (_address);
         }

         //! @}
         // ======================================================================

         // ======================================================================
         // Events
         // ======================================================================
         //! \name Events
         //! @{

         /*!
          * This method is called when a response to a registration message
          * is received.
          *
          * @param [in] response  the register response that was received.
          */
         virtual void registered (RegisterResponse &response);

         /*!
          * This method is called when a response to a de-registration message
          * is received.
          *
          * @param [in] response    the response received.
          */
         virtual void deregistered (Protocol::Response &response);

         //! @}
         // ======================================================================

         protected:

         //! \see AbstractInterface::payload_size
         size_t payload_size (Protocol::Message::Interface &itf) const;

         //! \see AbstractInterface::handle_command
         Result handle_command (Protocol::Packet &packet, ByteArray &payload, size_t offset);
      };

      /*!
       * Device Management interface : Server side.
       */
      struct DeviceManagementServer:public ServiceRole <DeviceManagement, Interface::SERVER_ROLE>
      {
         /*!
          * Return the Device entry for the given address.
          *
          * @param [in] address    the device address.
          *
          * @retval  a pointer the Device entry associated with the given address,
          * @retval  nullptr if the entry does not exist.
          */
         virtual Device *entry (uint16_t address) = 0;

         /*!
          * Return the Device entry for the given UID.
          *
          * @param [in] UID   the device UID.
          *
          * @retval  a pointer the Device entry associated with the given UID,
          * @retval  nullptr if the entry does not exist.
          */
         virtual Device *entry (HF::UID *uid) = 0;

         /*!
          * Store the given \c device entry to persistent storage.
          *
          * @param [in] device   the device entry to store.
          *
          * @return     if the device entry was saved.
          */
         virtual Result save (Device *device) = 0;

         /*!
          * Remove the given \c device entry from persistent storage.
          *
          * @param [in] device   the device entry to remove.
          *
          * @return     if the device entry was removed.
          */
         virtual Result destroy (Device *device) = 0;

         /*!
          * Return the number of Device entries available.
          *
          * @return  number of Device entries present.
          */
         virtual uint16_t entries_count () const = 0;

         /*!
          * Return the a vector with \c count Device entries starting at \c offset.
          *
          * @param [in] offset   the offset to start at.
          * @param [in] count    the
          * @return
          */
         virtual vector <Device *> entries (uint16_t offset, uint16_t count) = 0;

         /*!
          * Return all device entries starting at \c offset.
          *
          * @param [in] offset   the offset to start at.
          *
          * @return a vector containing the requested entries.
          */
         vector <Device *> entries (uint16_t offset = 0)
         {
            return entries (offset, entries_count () - offset);
         }

         // =============================================================================
         // Interface Attribute API.
         // =============================================================================

         //! \see Interface::attribute
         IAttribute *attribute (uint8_t uid)
         {
            UNUSED(uid);
            return Core::create_attribute(this, uid);
         }

         protected:

         DeviceManagementServer(IDevice *_device):
            ServiceRole (_device)
         {}

         //! \see AbstractInterface::attributes
         attribute_uids_t attributes (bool optional = false) const
         {
            UNUSED (optional);
            return attribute_uids_t ({NUMBER_OF_ENTRIES_ATTR});
         }

         /*!
          * Return next available address for registering a device.
          *
          * @return  the address to use in the next registration.
          */
         virtual uint16_t next_address () = 0;

         // ======================================================================
         // Events
         // ======================================================================
         //! \name Events
         //! @{

         /*!
          * This method is called when a registration message is received.
          *
          * \see DeviceManagementServer::handle
          */
         virtual Result register_device (Protocol::Packet &packet, ByteArray &payload, size_t offset);

         /*!
          * This method is called when a deregistration message is received.
          *
          * \see DeviceManagementServer::handle
          */
         virtual Result deregister_device (Protocol::Packet &packet, ByteArray &payload, size_t offset);

         //! @}
         // ======================================================================

         // ======================================================================
         // Commands
         // ======================================================================
         //! \name Commands
         //! @{

         /*!
          * This method serves to indicate if a given \c member of the interface
          * can be used by the \c source device affecting the \c destination
          * device configuration on the system.
          *
          * @param [in] member       interface member UID.
          * @param [in] source       device entry for the requesting device.
          * @param [in] destination  device entry for the affected device.
          *
          * @retval  true     the operation is allowed,
          * @retval  false    otherwise.
          */
         virtual bool authorized (uint8_t member, Device *source, Device *destination) = 0;

         //! @}
         // ======================================================================

         //! \see AbstractInterface::payload_size
         size_t payload_size (Protocol::Message::Interface &itf) const;

         //! \see AbstractInterface::handle_command
         Result handle_command (Protocol::Packet &packet, ByteArray &payload, size_t offset);
      };

      // =========================================================================
      // Default API Implementations
      // =========================================================================

      /*!
       * This class provide a simple RAM based implementation of the DeviceManagementServer
       * interface.
       */
      struct DefaultDeviceManagementServer:public DeviceManagementServer
      {
         DefaultDeviceManagementServer(IDevice *_device):DeviceManagementServer (_device)
         {}

         virtual ~DefaultDeviceManagementServer();

         // =============================================================================
         // API
         // =============================================================================

         virtual Device *entry (uint16_t address);

         virtual Device *entry (HF::UID *uid);

         virtual Result save (Device *device);

         virtual Result destroy (Device *device);

         uint16_t entries_count () const
         {
            return _entries.size ();
         }

         vector <DeviceManagement::Device *> entries (uint16_t offset, uint16_t count);

         using DeviceManagementServer::entries;

         protected:

         vector <Device *>         _entries;

         map <uint16_t, Device *>  _addr2device;
         map <HF::UID *, Device *> _uid2device;


         uint16_t next_address ()
         {
            return _entries.size () + 1;
         }

         virtual bool authorized (uint8_t member, Device *source, Device *destination);
      };

   } // namespace Core

} // namespace HF

// =============================================================================
// Helper Functions
// =============================================================================

inline bool operator ==(const HF::Core::DeviceManagement::Interface &lhs,
                        const HF::Core::DeviceManagement::Interface &rhs)
{
   return (lhs.role == rhs.role) && (lhs.uid == lhs.uid);
}

inline bool operator !=(const HF::Core::DeviceManagement::Interface &lhs,
                        const HF::Core::DeviceManagement::Interface &rhs)
{
   return !(lhs == rhs);
}

inline bool operator ==(const HF::Core::DeviceManagement::Unit &lhs,
                        const HF::Core::DeviceManagement::Unit &rhs)
{
   if (lhs.id != rhs.id)
   {
      return false;
   }

   if (lhs.profile != rhs.profile)
   {
      return false;
   }

   if (lhs.opt_ift.size () != rhs.opt_ift.size ())
   {
      return false;
   }

   for (uint8_t i = 0; i < lhs.opt_ift.size (); i++)
   {
      if (lhs.opt_ift[i] != rhs.opt_ift[i])
      {
         return false;
      }
   }

   return true;
}

inline bool operator !=(const HF::Core::DeviceManagement::Unit &lhs,
                        const HF::Core::DeviceManagement::Unit &rhs)
{
   return !(lhs == rhs);
}

inline bool operator ==(const HF::Core::DeviceManagement::Device &lhs,
                        const HF::Core::DeviceManagement::Device &rhs)
{
   if (lhs.address != rhs.address)
   {
      return false;
   }

   if (lhs.units.size () != rhs.units.size ())
   {
      return false;
   }

   for (uint8_t i = 0; i < lhs.units.size (); i++)
   {
      if (lhs.units[i] != rhs.units[i])
      {
         return false;
      }
   }

   return true;
}

inline bool operator !=(const HF::Core::DeviceManagement::Device &lhs,
                        const HF::Core::DeviceManagement::Device &rhs)
{
   return !(lhs == rhs);
}

#endif /* HF_DEVICE_MANGEMENT_H */
