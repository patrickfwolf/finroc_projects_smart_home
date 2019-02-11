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
/*!\file    projects/smart_home/heat_control/mPT.h
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-12
 *
 * \brief Contains mPT
 *
 * \b mPT
 *
 * module that provides a hardware interface, a/d conversion and is capable of setting pumps.
 *
 */
//----------------------------------------------------------------------
#ifndef __projects__smart_home__shared__mPT_h__
#define __projects__smart_home__shared__mPT_h__

#include "plugins/structure/tModule.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "projects/smart_home/shared/tPT.h"

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
 * module that provides a hardware interface, a/d conversion and is capable of setting pumps.
 */
template<size_t TResistance>
class mPT: public structure::tModule
{

//----------------------------------------------------------------------
// Ports (These are the only variables that may be declared public)
//----------------------------------------------------------------------
public:

  tInput<rrlib::si_units::tVoltage<double>> in_voltage;

  tOutput<rrlib::si_units::tCelsius<double>> out_temperature;
  tOutput<rrlib::si_units::tElectricResistance<double>> out_resistance;

  tParameter<rrlib::si_units::tElectricResistance<double>> par_pre_resistance;
  tParameter<rrlib::si_units::tVoltage<double>> par_reference_voltage;
  tParameter<rrlib::si_units::tVoltage<double>> par_supply_voltage;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  mPT(core::tFrameworkElement *parent, const std::string &name = "PT") :
    tModule(parent, name),
    par_pre_resistance(2000.0),
    par_reference_voltage(5.0),
    par_supply_voltage(5.0)
  {}

//----------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------
protected:

  /*! Destructor
   *
   * The destructor of modules is declared protected to avoid accidental deletion. Deleting
   * modules is already handled by the framework.
   */
  ~mPT() {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  inline virtual void Update() override
  {
    if (this->InputChanged())
    {
      auto resistance = GetResistance(in_voltage.Get(), par_reference_voltage.Get(), par_pre_resistance.Get());

      out_resistance.Publish(resistance, in_voltage.GetTimestamp());
      auto temperature = pt_.GetTemperature(resistance);
      if(not std::isnan(temperature.Value()))
      {
    	  out_temperature.Publish(temperature, in_voltage.GetTimestamp());
      }
    }
  }

  /*!
   * Determines resistance value of PT sensor
   * @param voltage A/D voltage
   * @param reference_voltage reference voltage of A/D converter
   * @param pre_resistance pre resistance of PT sensor
   * @return resistance
   */
  inline rrlib::si_units::tElectricResistance<double> GetResistance(
    const rrlib::si_units::tVoltage<double> & voltage_adc,
    const rrlib::si_units::tVoltage<double> & voltage_ref,
    const rrlib::si_units::tElectricResistance<double> & resistance_pre) const
  {
    /*
     * _____ V_ref
     *   |
     *  [ ] R_pre
     *   |____ V_adc
     *   |
     *  [ ] R_pt
     * __|__
     *        V_gnd
     *
     * R_pre / (V_ref - V_adc) = R_pt / (V_adc - V_gnf)
     *
     */
    if (voltage_adc >= voltage_ref)
    {
      return resistance_pre;
    }

    return resistance_pre * voltage_adc / (voltage_ref - voltage_adc);
  }

  shared::tPT<TResistance> pt_;

};

using mPT1000 = mPT<1000>;
using mPT500 = mPT<500>;
using mPT100 = mPT<100>;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
