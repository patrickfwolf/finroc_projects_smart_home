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
/*!\file    projects/smart_home/vent_control/gVentControl.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/vent_control/gVentControl.h"

//----------------------------------------------------------------------
// External includes
//----------------------------------------------------------------------
#ifdef _LIB_WIRING_PI_PRESENT_
#include "libraries/gpio_raspberry_pi/mRaspberryIO.h"
#endif

#include <cassert>

//----------------------------------------------------------------------
// Internal includes
//----------------------------------------------------------------------
#include "projects/smart_home/vent_control/mController.h"

//----------------------------------------------------------------------
// Namespace usage
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

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<gVentControl> cCREATE_ACTION_FOR_G_RASPBERRYPIVENTILATIONCONTROL("VentControl");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// gVentControl constructor
//----------------------------------------------------------------------
gVentControl::gVentControl(core::tFrameworkElement *parent, const std::string &name,
                           const std::string &structure_config_file) :
  tGroup(parent, name, structure_config_file, true)
{
  auto ventilation = new mController(this, "Vent Control");
  ventilation->si_temperature_speicher.ConnectTo(this->in_temperature_speicher);
  ventilation->si_temperature_solar.ConnectTo(this->in_temperature_solar);
  ventilation->si_pump_online_solar.ConnectTo(this->in_solar_online);

#ifdef _LIB_WIRING_PI_PRESENT_
  auto gpio_interface = new gpio_raspberry_pi::mRaspberryIO(this, "Raspberry Pi GPIO Interface");
  gpio_interface->par_configuration_file.Set("$FINROC_PROJECT_HOME/etc/vent_control_gpio_config.xml");
  gpio_interface->Init();
  gpio_interface->GetInputs().ConnectByName(ventilation->GetControllerOutputs(), false);
#endif

}

//----------------------------------------------------------------------
// gVentControl destructor
//----------------------------------------------------------------------
gVentControl::~gVentControl()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
