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
/*!\file    projects/smart_home/heat_control_states/tSolar.h
 *
 * \author  Patrick Wolf
 *
 * \date    2014-05-14
 *
 */

#ifndef __projects__smart_home__heat_control_states__tSolar_h__
#define __projects__smart_home__heat_control_states__tSolar_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control_states/tState.h"

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


//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 *
 */
class tSolar : public tState
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tSolar() :
    tState(tCurrentState::eSOLAR)
  {}

  virtual ~tSolar()
  {}

  /*!
   * Determines next state based on temperature values
   * @param state
   * @param temperatures
   */
  virtual void ComputeControlState(std::unique_ptr<tState> & state, const shared::tTemperatures & temperatures) override;

  /*!
   * Sets the solar pump online and ground and room off-line
   * @return pump values
   */
  virtual inline shared::tPumps GetPumpSettings() const override
  {
    return shared::tPumps(false, false, true);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
protected:


};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
