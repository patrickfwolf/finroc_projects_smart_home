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
/*!\file    projects/smart_home/heat_control/mMCP3008.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief Contains mMCP3008
 *
 * \b mMCP3008
 *
 * MCP 3008 A/D converter, reads raw unsigned voltage and provides SI voltage output.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__shared__mMCP3008_h__
#define __projects__smart_home__shared__mMCP3008_h__

#include "plugins/structure/tModule.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/shared/tMCP3008.h"

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
 * MCP 3008 A/D converter, reads raw unsigned voltage and provides SI voltage output.
 */
template<size_t Tchannels>
class mMCP3008 : public structure::tModule
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  std::vector<tInput<unsigned short>> in_voltage_raw;

  std::vector<tOutput<rrlib::si_units::tVoltage<double>>> out_voltage;

  tParameter<rrlib::si_units::tVoltage<double>> par_reference_voltage;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mMCP3008(core::tFrameworkElement *parent, const std::string &name = "MCP3008"):
    tModule(parent, name),
    par_reference_voltage(5.0),
    mcp3008_(5.0)
  {
    for (std::size_t i = 0; i < Tchannels; i++)
    {
      in_voltage_raw.emplace_back(tInput<unsigned short>("Voltage Raw " + std::to_string(i), this));
      out_voltage.emplace_back(tOutput<rrlib::si_units::tVoltage<double>>("Voltage " + std::to_string(i), this));
    }
  }

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of modules is declared protected to avoid accidental deletion. Deleting
   * modules is already handled by the framework.
   */
  ~mMCP3008() {};

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  inline virtual void OnParameterChange() override
  {
    mcp3008_.SetReferenceVoltage(par_reference_voltage.Get());
  }

  inline virtual void Update() override
  {
    if (this->InputChanged())
    {
      for (std::size_t i = 0; i < Tchannels; i++)
      {
        out_voltage.at(i).Publish(mcp3008_.ConvertADValueToVoltage(in_voltage_raw.at(i).Get()), in_voltage_raw.at(i).GetTimestamp());
      }
    }
  }

  shared::tMCP3008 mcp3008_;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}



#endif
