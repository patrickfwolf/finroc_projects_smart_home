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
/*!\file    projects/smart_home/heat_control/tMCP3008Converter.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-05-07
 *
 * \brief   Contains tMCP3008Converter.h
 *
 * \b tMCP3008Converter.h
 *
 * Converter for MCP3008 output.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__heat_control__tMCP3008Converter_h__
#define __projects__smart_home__heat_control__tMCP3008Converter_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
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
namespace shared
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
static const unsigned short cMCP3008_RESOLUTION = 1024;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! SHORT_DESCRIPTION
/*!
 * Converter for MCP3008 output.
 */
class tMCP3008Converter
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Constructor
   * @param reference_voltage reference voltage for MCP3008
   */
  tMCP3008Converter(const rrlib::si_units::tVoltage<double> & reference_voltage):
    reference_voltage_(reference_voltage)
  {}

  /*!
   * Destructor
   */
  ~tMCP3008Converter()
  {}

  /*!
   * Sets the reference voltage of MCP3008
   * @param reference_voltage reference voltage
   */
  inline void SetReferenceVoltage(const rrlib::si_units::tVoltage<double> & reference_voltage)
  {
    reference_voltage_ = reference_voltage;
  }

  /*!
   * Getter for reference voltage
   * @return reference voltage of MCP3008
   */
  inline rrlib::si_units::tVoltage<double> GetReferenceVoltage() const
  {
    return reference_voltage_;
  }

  /*!
   * Converts the value of the MCP3008 output to a voltage
   * @param ad_value value of MCP3008 output
   * @return voltage
   */
  inline rrlib::si_units::tVoltage<double> ConvertADValueToVoltage(unsigned short ad_value) const
  {
    // mcp3008 is 10 bit converter
    if (ad_value + 1 > cMCP3008_RESOLUTION)
    {
      RRLIB_LOG_PRINT(ERROR, "MCP3008 output larger than 10bit. Output has ", std::ceil(std::log10(static_cast<double>(ad_value + 1)) / std::log10(2.0)), " bit.");
      return reference_voltage_;
    }
    double ratio = static_cast<double>(ad_value) / static_cast<double>(cMCP3008_RESOLUTION - 1);
    return reference_voltage_ * ratio;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  rrlib::si_units::tVoltage<double> reference_voltage_;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
