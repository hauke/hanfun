// =============================================================================
/*!
 * \file       src/interfaces/simple_power_meter_server.cpp
 *
 * This file contains the implementation of the Simple Power Meter interface :
 * Server role.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    0.1.0
 *
 * \copyright  Copyright &copy; &nbsp; 2013 Bithium S.A.
 */
// =============================================================================

#include <cmath>
#include <cstring>

#include "hanfun/interfaces/simple_power_meter.h"

using namespace HF;
using namespace HF::Interfaces;
using namespace HF::Interfaces::SimplePowerMeter;

// =============================================================================
// SimplePowerMeter::Server
// =============================================================================

SimplePowerMeter::Server::Server()
{
#if HF_IFT_SPM_ENERGY_ATTR
   memset (&_energy, 0, sizeof(Measurement));          // Energy measurement.
#endif

#if HF_ITF_SPM_ENERGY_AT_RESET_ATTR
   memset (&_last_energy, 0, sizeof(Measurement));     // Energy measurement at last reset.
#endif

#if HF_ITF_SPM_TIME_AT_RESET_ATTR
   memset (&_last_time, 0, sizeof(Measurement));       // Device time measurement at last reset.
#endif

#if HF_ITF_SPM_POWER_ATTR
   memset (&_power, 0, sizeof(Measurement));           // Instantaneous Power measurement.
#endif

#if HF_ITF_SPM_AVG_POWER_ATTR
   memset (&_avg_power, 0, sizeof(Measurement));       // Average Power measurement.
#endif

#if HF_ITF_SPM_AVG_POWER_INTERVAL_ATTR
   _avg_power_interval = 0;                              // Average Power Interval.
#endif

#if HF_ITF_SPM_POWER_FACTOR_ATTR
   _power_factor = 0;                                    // Power Factor.
#endif

#if HF_ITF_SPM_VOLTAGE_ATTR
   memset (&_voltage, 0, sizeof(Measurement));         // Voltage measurement.
#endif

#if HF_ITF_SPM_CURRENT_ATTR
   memset (&_current, 0, sizeof(Measurement));         // Current measurement.
#endif

#if HF_ITF_SPM_FREQUENCY_ATTR
   memset (&_frequency, 0, sizeof(Measurement));       // Frequency measurement.
#endif

#if HF_ITF_SPM_REPORT_CMD
   _report_interval = 0;
   _last_periodic   = 0;
#endif
}

// =============================================================================
// SimplePowerMeter::Server::report
// =============================================================================
/*!
 *
 */
// =============================================================================
SimplePowerMeter::Report *SimplePowerMeter::Server::report ()
{
   SimplePowerMeter::Report *report = new (nothrow) SimplePowerMeter::Report ();

   if (report != nullptr)
   {
#if HF_ITF_SPM_ENERGY_ATTR
      report->energy                                 = this->energy (); // Energy.
      report->enabled[SimplePowerMeter::ENERGY_ATTR] = true;
#endif

#if HF_ITF_SPM_ENERGY_AT_RESET_ATTR
      report->last_energy                                     = this->last_energy (); // Energy at Last Reset.
      report->enabled[SimplePowerMeter::ENERGY_AT_RESET_ATTR] = true;
#endif

#if HF_ITF_SPM_TIME_AT_RESET_ATTR
      report->last_time                                     = this->last_time (); // Time at Last Reset.
      report->enabled[SimplePowerMeter::TIME_AT_RESET_ATTR] = true;
#endif

#if HF_ITF_SPM_POWER_ATTR
      report->power                                 = this->power (); // Instantaneous Power.
      report->enabled[SimplePowerMeter::POWER_ATTR] = true;
#endif

#if HF_ITF_SPM_AVG_POWER_ATTR
      report->avg_power                                 = this->avg_power (); // Average Power.
      report->enabled[SimplePowerMeter::AVG_POWER_ATTR] = true;
#endif

#if HF_ITF_SPM_AVG_POWER_INTERVAL_ATTR
      // Average Power Interval.
      report->avg_power_interval                                 = this->avg_power_interval ();
      report->enabled[SimplePowerMeter::AVG_POWER_INTERVAL_ATTR] = true;
#endif

#if HF_ITF_SPM_VOLTAGE_ATTR
      report->voltage                                 = this->voltage (); // Voltage.
      report->enabled[SimplePowerMeter::VOLTAGE_ATTR] = true;
#endif

#if HF_ITF_SPM_CURRENT_ATTR
      report->current                                 = this->current (); // Current.
      report->enabled[SimplePowerMeter::CURRENT_ATTR] = true;
#endif

#if HF_ITF_SPM_FREQUENCY_ATTR
      report->frequency                                 = this->frequency (); // Frequency.
      report->enabled[SimplePowerMeter::FREQUENCY_ATTR] = true;
#endif

#if HF_ITF_SPM_POWER_FACTOR_ATTR
      report->power_factor                                 = this->power_factor (); // Power Factor.
      report->enabled[SimplePowerMeter::POWER_FACTOR_ATTR] = true;
#endif
   }

   return report;
}

// =============================================================================
// SimplePowerMeter::Server::periodic
// =============================================================================
/*!
 *
 */
// =============================================================================
void SimplePowerMeter::Server::periodic (uint32_t time)
{
   UNUSED (time);

#if HF_ITF_SPM_REPORT_CMD

   if (_report_interval > 0 && abs ((int64_t) _last_periodic - time) >= _report_interval)
   {
      Protocol::Address addr;
      Protocol::Message message;

      message.itf.role   = CLIENT_ROLE;
      message.itf.uid    = SimplePowerMeter::Server::uid ();
      message.itf.member = REPORT_CMD;

      message.payload    = report ();

      sendMessage (addr, message);

      _last_periodic = time;
   }

#endif
}

// =============================================================================
// SimplePowerMeter::Server::attribute
// =============================================================================
/*!
 *
 */
// =============================================================================
HF::Attributes::IAttribute *SimplePowerMeter::Server::attribute (uint8_t uid)
{
   Attributes attr = static_cast <Attributes>(uid);

   switch (attr)
   {
#if HF_ITF_SPM_ENERGY_ATTR
      case ENERGY_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_ENERGY_AT_RESET_ATTR
      case ENERGY_AT_RESET_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_TIME_AT_RESET_ATTR
      case TIME_AT_RESET_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_POWER_ATTR
      case POWER_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_AVG_POWER_ATTR
      case AVG_POWER_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_AVG_POWER_INTERVAL_ATTR
      case AVG_POWER_INTERVAL_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_VOLTAGE_ATTR
      case VOLTAGE_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_CURRENT_ATTR
      case CURRENT_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_FREQUENCY_ATTR
      case FREQUENCY_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_POWER_FACTOR_ATTR
      case POWER_FACTOR_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif

#if HF_ITF_SPM_REPORT_INTERVAL_ATTR
      case REPORT_INTERVAL_ATTR:
      {
         return Interfaces::create_attribute (this, uid);
      }
#endif
      default:
         return nullptr;
   }
}

// =============================================================================
// SimplePowerMeter::Server::attribute_uids
// =============================================================================
/*!
 *
 */
// =============================================================================
HF::Attributes::uids_t SimplePowerMeter::Server::attributes (uint8_t pack_id) const
{
   HF::Attributes::uids_t result;

   if (pack_id == HF::Attributes::Pack::ALL)
   {
#if HF_ITF_SPM_POWER_ATTR
      result.push_back (SimplePowerMeter::POWER_ATTR);
#endif
#if HF_ITF_SPM_AVG_POWER_ATTR
      result.push_back (SimplePowerMeter::AVG_POWER_ATTR);
#endif
#if HF_ITF_SPM_AVG_POWER_INTERVAL_ATTR
      result.push_back (SimplePowerMeter::AVG_POWER_INTERVAL_ATTR);
#endif
#if HF_ITF_SPM_VOLTAGE_ATTR
      result.push_back (SimplePowerMeter::VOLTAGE_ATTR);
#endif
#if HF_ITF_SPM_CURRENT_ATTR
      result.push_back (SimplePowerMeter::CURRENT_ATTR);
#endif
#if HF_ITF_SPM_FREQUENCY_ATTR
      result.push_back (SimplePowerMeter::FREQUENCY_ATTR);
#endif
#if HF_ITF_SPM_POWER_FACTOR_ATTR
      result.push_back (SimplePowerMeter::POWER_FACTOR_ATTR);
#endif
   }

#if HF_ITF_SPM_ENERGY_ATTR
   #if !HF_ITF_SPM_RESET_CMD

   if (pack_id == AttributePack::ALL)
   #endif
   {
      result.push_back (SimplePowerMeter::ENERGY_ATTR);
   }

#endif

#if HF_ITF_SPM_ENERGY_AT_RESET_ATTR
   #if !HF_ITF_SPM_RESET_CMD

   if (pack_id == AttributePack::ALL)
   #endif
   {
      result.push_back (SimplePowerMeter::ENERGY_AT_RESET_ATTR);
   }

#endif

#if HF_ITF_SPM_TIME_AT_RESET_ATTR
   #if !HF_ITF_SPM_RESET_CMD

   if (pack_id == AttributePack::ALL)
   #endif
   {
      result.push_back (SimplePowerMeter::TIME_AT_RESET_ATTR);
   }

#endif

#if HF_ITF_SPM_REPORT_INTERVAL_ATTR
   #if !HF_ITF_SPM_REPORT_CMD

   if (pack_id == AttributePack::ALL)
   #endif
   {
      result.push_back (SimplePowerMeter::REPORT_INTERVAL_ATTR);
   }

#endif

   return result;
}
