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
/*!\file    projects/smart_home/data_structures/tPumps.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief   Contains tPumps
 *
 * \b tPumps
 *
 * container class which stores the pump settings (on or off) as return type for the state machine.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__data_structures__tPumps_h__
#define __projects__smart_home__data_structures__tPumps_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

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
namespace shared
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * container class which stores the pump settings (on or off) as return type for the state machine.
 */
class tPumps
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Constructor
   * @param ground ground pump online
   * @param room room pump online
   * @param solar solar online
   */
  tPumps(bool ground, bool room, bool solar):
    ground_(ground),
    room_(room),
    solar_(solar)
  {}

  /*!
   * Destructor
   */
  ~tPumps()
  {}

  /*!
   * Is the ground pump online?
   * @return ground pump
   */
  inline bool IsGroundOnline() const
  {
    return ground_;
  }

  /*!
   * Is solar pump online?
   * @return solar pump
   */
  inline bool IsSolarOnline() const
  {
    return solar_;
  }

  /*!
   * Is room pump online?
   * @return room pump
   */
  inline bool IsRoomOnline() const
  {
    return room_;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  bool ground_;
  bool room_;
  bool solar_;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
