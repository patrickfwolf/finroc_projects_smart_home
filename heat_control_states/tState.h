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

//----------------------------------------------------------------------
/*!\file    ControlModeState/ControlModeState.h/
 *
 * \author  Patrick Wolf
 *
 * \date    2014-05-14
 *
 */

#ifndef __projects__smart_home__heat_control_states__tState_h__
#define __projects__smart_home__heat_control_states__tState_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <memory>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/shared/tTemperatures.h"
#include "projects/smart_home/shared/tPumps.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace smart_home
{
namespace heat_control_states
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
enum class tCurrentState
{
  eREADY = 0,
  eSOLAR,
  eROOM,
  eGROUND,
  eSOLAR_ROOM,
  eSOLAR_GROUND,
  eROOM_GROUND,
  eSOLAR_ROOM_GROUND
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 *
 */
class tState
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Constructor
   * @param current_state state
   */
  tState(const tCurrentState & current_state):
      state_has_changed_(false),
      current_state_(current_state)
  {}

  /*!
   * Destructor
   */
  virtual ~tState()
  {}

  /*!
   * Abstract function to compute current control state
   * @param state current state
   * @param temperatures sensed temperatures
   */
  virtual void ComputeControlState(std::unique_ptr<tState> & state, const shared::tTemperatures & temperatures) = 0;

  /*!
   * Abstract function to determine the pump settings
   * @return desired pump settings
   */
  virtual shared::tPumps GetPumpSettings() const = 0;

  /*!
   * Has the state changed
   * @return state changed
   */
  virtual bool HasChanged() const
  {
    return state_has_changed_;
  }

  /*!
   * Returns the current state
   * @return current state
   */
  virtual tCurrentState GetCurrentState() const
  {
    return this->current_state_;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
protected:

  /*!
   * Set the state to changed or unchanged
   * @param state_has_changed changed
   */
  virtual void SetChanged(bool state_has_changed)
  {
    this->state_has_changed_ = state_has_changed;
  }

  bool state_has_changed_;
  tCurrentState current_state_;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
