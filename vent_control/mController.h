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

  tSensorInput<rrlib::si_units::tCelsius<double>> si_temperature_furnace;
  tSensorInput<rrlib::si_units::tPressure<double>> si_air_pressure_room;
  tSensorInput<rrlib::si_units::tAmountOfSubstance<double>> si_carbon_monoxid_room;
  tSensorInput<bool> si_carbon_monoxid_warning_room;

  tSensorOutput<bool> so_furnace_active;

  // manually disable venting system, if oven is heating
  tControllerInput<tVentilationMode> ci_ventilation_mode;

  // relais control, relais must actively enable ventilation
  tControllerOutput<bool> co_gpio_ventilation;
  tControllerOutput<bool> co_ventilation;

  tParameter<rrlib::si_units::tCelsius<double>> par_furnace_activity_threshold;


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

  bool furnace_active_;

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
