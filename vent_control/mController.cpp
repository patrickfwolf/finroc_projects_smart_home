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
/*!\file    projects/smart_home/vent_control/mController.cpp
 *
 * \author  Patrick Vatter
 *
 * \date    2015-03-12
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/vent_control/mController.h"

#include <cassert>

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
static runtime_construction::tStandardCreateModuleAction<mController> cCREATE_ACTION_FOR_M_VENTILATION("Ventilation");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// mController constructor
//----------------------------------------------------------------------
mController::mController(core::tFrameworkElement *parent, const std::string &name) :
  structure::tSenseControlModule(parent, name, false),
  ci_ventilation_mode(tVentilationMode::eONLINE),
  co_gpio_ventilation(cVENTILATION_OFFINE),
  co_ventilation(false),
  par_furnace_activity_threshold(50.0),
  furnace_active_(false)
{
}

//----------------------------------------------------------------------
// mController destructor
//----------------------------------------------------------------------
mController::~mController()
{
}

//----------------------------------------------------------------------
// mController Sense
//----------------------------------------------------------------------
void mController::Sense()
{
	if(this->SensorInputChanged())
	{
		(si_temperature_furnace.Get() > par_furnace_activity_threshold.Get()) ? furnace_active_ = true : furnace_active_ = false;
		so_furnace_active.Publish(furnace_active_, si_temperature_furnace.GetTimestamp());
	}
}

//----------------------------------------------------------------------
// mController Control
//----------------------------------------------------------------------
void mController::Control()
{
	if(this->ControllerInputChanged() or ci_ventilation_mode.Get() == tVentilationMode::eAUTOMATIC)
	{
	  // ventilation manually disabled
	  if (ci_ventilation_mode.Get() == tVentilationMode::eOFFLINE)
	  {
	    co_gpio_ventilation.Publish(cVENTILATION_OFFINE);
	    co_ventilation.Publish(false);
	  }
	  else if (ci_ventilation_mode.Get() == tVentilationMode::eONLINE)
	  {
	    co_gpio_ventilation.Publish(cVENTILATION_ONLINE);
	    co_ventilation.Publish(true);
	  }
	  else if (ci_ventilation_mode.Get() == tVentilationMode::eAUTOMATIC)
	  {
		  // TODO: implement
	  }
	}
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
