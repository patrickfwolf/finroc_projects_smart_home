//
// You received this file as part of Finroc
// A framework for intelligent robot control
//
// Copyright (C) Patrick Wolf
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    projects/raspberry_pi_heating_control/control/mController.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief Contains mController
 *
 * \b mController
 *
 * control module for the heating control which contains the state machine and interacts with the hardware interface.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__raspberry_pi_heating_control__ventilation__mController_h__
#define __projects__raspberry_pi_heating_control__ventilation__mController_h__

#include "plugins/structure/tSenseControlModule.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/signal_processing/tDerivation.h"
#include "rrlib/si_units/si_units.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace vent_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
enum class tVentilationMode
{
  eAUTOMATIC,
  eOFFLINE,
  eONLINE
};


// hardware abstraction values for relais
const bool cVENTILATION_ONLINE = false;
const bool cVENTILATION_OFFINE = true;


//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * control module for the heating control which contains the state machine and interacts with the hardware interface.
 */
class mController : public structure::tSenseControlModule
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_speicher;
  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_solar;
  tSensorInput<bool> si_pump_online_solar;

  tSensorOutput<rrlib::si_units::tTemperatureChange<double>> so_temperature_velocity;

  // manually disable venting system, if oven is heating
  tControllerInput<tVentilationMode> ci_ventilation_mode;

  // relais control, relais must actively enable ventilation
  tControllerOutput<bool> co_gpio_ventilation;
  tControllerOutput<bool> co_ventilation_online;

  // maximum allowed temperature increase over a period of time
  tParameter<rrlib::si_units::tTemperatureChange<double>> par_temperature_velocity_threshold;

  // maximum allowed duration to retrieve data from control
  tParameter<rrlib::si_units::tTime<double>> par_time_delta_threshold;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mController(core::tFrameworkElement *parent, const std::string &name = "Ventilation");

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of modules is declared protected to avoid accidental deletion. Deleting
   * modules is already handled by the framework.
   */
  ~mController();

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  rrlib::signal_processing::tDerivation<rrlib::si_units::tTemperatureChange<double>, rrlib::si_units::tCelsius<double>> derivation;
  rrlib::si_units::tCelsius<double> current_temperature;
  rrlib::util::tTime current_time;

  virtual void OnParameterChange() override;

  virtual void Sense() override;

  virtual void Control() override;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}



#endif
