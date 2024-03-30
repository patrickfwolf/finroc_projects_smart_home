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
/*!\file    projects/smart_home/heat_control/gHeatControl.cpp
 *
 * \author  Patrick Wolf
 *
 * \date    2015-03-11
 *
 */
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/gHeatControl.h"

//----------------------------------------------------------------------
// External includes
//----------------------------------------------------------------------
#include <cassert>

#ifdef _LIB_WIRING_PI_PRESENT_
#include "libraries/gpio_raspberry_pi/mRaspberryIO.h"
#endif

#include "libraries/signal_filters/mExponentialFilter.h"

//----------------------------------------------------------------------
// Internal includes
//----------------------------------------------------------------------
#include "projects/smart_home/heat_control/mController.h"
#include "projects/smart_home/heat_control/mPumpInterface.h"

#include "projects/smart_home/shared/mMCP3008.h"
#include "projects/smart_home/shared/mPT.h"

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
namespace heat_control
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
enum tMCP3008Output
{
  ePT1000_SOLAR = 0,
  ePT100_ROOM,
  ePT1000_BOILER_MIDDLE,
  ePT1000_GROUND,
  ePT100_BOILER_TOP,
  ePT100_BOILER_BOTTOM,
  ePT100_FURNACE,
  ePT100_GARAGE,
  eCOUNT
};
//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
static runtime_construction::tStandardCreateModuleAction<gHeatControl> cCREATE_ACTION_FOR_G_RASPBERRYPIHEATINGCONTROL("HeatControl");

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// gHeatControl constructor
//----------------------------------------------------------------------
gHeatControl::gHeatControl(core::tFrameworkElement *parent, const std::string &name,
                           const std::string &structure_config_file) :
  tSenseControlGroup(parent, name, structure_config_file, true), // change to 'true' to make group's ports shared (so that ports in other processes can connect to its output and/or input ports)
  ci_control_mode(tControlModeType::eAUTOMATIC),
  ci_manual_pump_solar(false),
  ci_manual_pump_room(false),
  ci_manual_pump_ground(false)
{
#ifdef _LIB_WIRING_PI_PRESENT_
  auto gpio_interface = new finroc::gpio_raspberry_pi::mRaspberryIO(this, "Raspberry Pi GPIO Interface", true, 500000);
  gpio_interface->par_configuration_file.Set("$FINROC_PROJECT_HOME/etc/heat_control_gpio_config.xml");
  gpio_interface->Init();
#endif

  auto controller = new mController(this, "Controller");
  controller->par_temperature_set_point_room.Set(23.0);
  this->si_temperature_room_external.ConnectTo(controller->si_temperature_room_external);
  this->ci_control_mode.ConnectTo(controller->ci_control_mode);
  this->ci_manual_pump_ground.ConnectTo(controller->ci_manual_pump_online_ground);
  this->ci_manual_pump_room.ConnectTo(controller->ci_manual_pump_online_room);
  this->ci_manual_pump_solar.ConnectTo(controller->ci_manual_pump_online_solar);
  this->co_control_mode.ConnectTo(controller->co_control_mode);
  this->co_heating_state.ConnectTo(controller->co_heating_state);
  this->so_error_state.ConnectTo(controller->so_error_state);
  this->so_led_green.ConnectTo(controller->co_led_online_green);
  this->so_led_yellow.ConnectTo(controller->co_led_online_yellow);
  this->so_led_red.ConnectTo(controller->co_led_online_red);
  this->so_pump_ground.ConnectTo(controller->co_pump_online_ground);
  this->so_pump_room.ConnectTo(controller->co_pump_online_room);
  this->so_pump_solar.ConnectTo(controller->co_pump_online_solar);
  controller->co_pump_error_ground.ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Input/Gpio Pump Error Ground");
  controller->co_pump_error_solar.ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Input/Gpio Pump Error Solar");
  controller->co_pump_error_room.ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Input/Gpio Pump Error Room");

  auto pump_interface = new mPumpInterface(this, "Pump Interface");
  pump_interface->in_pump_online_ground.ConnectTo(controller->co_pump_online_ground);
  pump_interface->in_pump_online_room.ConnectTo(controller->co_pump_online_room);
  pump_interface->in_pump_online_solar.ConnectTo(controller->co_pump_online_solar);
  pump_interface->out_gpio_pump_online_ground.ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Input/Gpio Pump Online Ground");
  pump_interface->out_gpio_pump_online_solar.ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Input/Gpio Pump Online Solar");
  pump_interface->out_gpio_pump_online_room.ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Input/Gpio Pump Online Room");

  auto mcp_3008 = new shared::mMCP3008<tMCP3008Output::eCOUNT>(this, "MCP3008");
  mcp_3008->par_reference_voltage.Set(5.0);
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT1000_SOLAR).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Solar");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT100_ROOM).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Room");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT1000_BOILER_MIDDLE).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Boiler Middle");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT1000_GROUND).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Ground");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT100_BOILER_TOP).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Boiler Top");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT100_BOILER_BOTTOM).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Boiler Bottom");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT100_FURNACE).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Furnace");
  mcp_3008->in_voltage_raw.at(tMCP3008Output::ePT100_GARAGE).ConnectTo("/Main Thread/HeatControl/Raspberry Pi GPIO Interface/Output/Mcp3008 Ad Voltage Garage");

  auto pt100_room = new shared::mPT100(this, "PT100 Room");
  pt100_room->par_pre_resistance.Set(94.0);
  pt100_room->par_reference_voltage.Set(5.0);
  pt100_room->par_supply_voltage.Set(5.0);
  pt100_room->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT100_ROOM));

  auto filter_room = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT100 Room Filter");
  filter_room->par_number_of_ports.Set(1);
  filter_room->Init();
  filter_room->par_weight.Set(0.001);
  filter_room->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_room->in_input_values.at(0).ConnectTo(pt100_room->out_temperature);
  filter_room->out_filtered_values.at(0).ConnectTo(controller->si_temperature_room);

  auto pt1000_boiler_middle = new shared::mPT1000(this, "PT1000 Boiler Middle");
  pt1000_boiler_middle->par_pre_resistance.Set(993.0);
  pt1000_boiler_middle->par_reference_voltage.Set(5.0);
  pt1000_boiler_middle->par_supply_voltage.Set(5.0);
  pt1000_boiler_middle->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_BOILER_MIDDLE));

  auto filter_boiler_middle = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT1000 Boiler Middle Filter");
  filter_boiler_middle->par_number_of_ports.Set(1);
  filter_boiler_middle->Init();
  filter_boiler_middle->par_weight.Set(0.01);
  filter_boiler_middle->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_boiler_middle->in_input_values.at(0).ConnectTo(pt1000_boiler_middle->out_temperature);
  filter_boiler_middle->out_filtered_values.at(0).ConnectTo(controller->si_temperature_boiler_middle);

  auto pt100_boiler_bottom = new shared::mPT100(this, "PT100 Boiler Bottom");
  pt100_boiler_bottom->par_pre_resistance.Set(92.4);
  pt100_boiler_bottom->par_reference_voltage.Set(5.0);
  pt100_boiler_bottom->par_supply_voltage.Set(5.0);
  pt100_boiler_bottom->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT100_BOILER_BOTTOM));

  auto filter_boiler_bottom = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT100 Boiler Bottom Filter");
  filter_boiler_bottom->par_number_of_ports.Set(1);
  filter_boiler_bottom->Init();
  filter_boiler_bottom->par_weight.Set(0.01);
  filter_boiler_bottom->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_boiler_bottom->in_input_values.at(0).ConnectTo(pt100_boiler_bottom->out_temperature);
  filter_boiler_bottom->out_filtered_values.at(0).ConnectTo(controller->si_temperature_boiler_bottom);

  auto pt100_boiler_top = new shared::mPT100(this, "PT100 Boiler Top");
  pt100_boiler_top->par_pre_resistance.Set(92.6);
  pt100_boiler_top->par_reference_voltage.Set(5.0);
  pt100_boiler_top->par_supply_voltage.Set(5.0);
  pt100_boiler_top->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT100_BOILER_TOP));

  auto filter_boiler_top = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT100 Boiler Top Filter");
  filter_boiler_top->par_number_of_ports.Set(1);
  filter_boiler_top->Init();
  filter_boiler_top->par_weight.Set(0.01);
  filter_boiler_top->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_boiler_top->in_input_values.at(0).ConnectTo(pt100_boiler_top->out_temperature);
  filter_boiler_top->out_filtered_values.at(0).ConnectTo(controller->si_temperature_boiler_top);

  auto pt1000_solar = new shared::mPT1000(this, "PT1000 Solar");
  pt1000_solar->par_pre_resistance.Set(991.0);
  pt1000_solar->par_reference_voltage.Set(5.0);
  pt1000_solar->par_supply_voltage.Set(5.0);
  pt1000_solar->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_SOLAR));

  auto filter_solar = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT1000 Solar Filter");
  filter_solar->par_number_of_ports.Set(1);
  filter_solar->Init();
  filter_solar->par_weight.Set(0.005);
  filter_solar->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_solar->in_input_values.at(0).ConnectTo(pt1000_solar->out_temperature);
  filter_solar->out_filtered_values.at(0).ConnectTo(controller->si_temperature_solar);

  auto pt1000_ground = new shared::mPT1000(this, "PT1000 Ground");
  pt1000_ground->par_pre_resistance.Set(991.0);
  pt1000_ground->par_reference_voltage.Set(5.0);
  pt1000_ground->par_supply_voltage.Set(5.0);
  pt1000_ground->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT1000_GROUND));

  auto filter_ground = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT1000 Ground Filter");
  filter_ground->par_number_of_ports.Set(1);
  filter_ground->Init();
  filter_ground->par_weight.Set(0.005);
  filter_ground->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_ground->in_input_values.at(0).ConnectTo(pt1000_ground->out_temperature);
  filter_ground->out_filtered_values.at(0).ConnectTo(controller->si_temperature_ground);

  auto pt100_furnace = new shared::mPT100(this, "PT100 Furnace");
  pt100_furnace->par_pre_resistance.Set(92.55);
  pt100_furnace->par_reference_voltage.Set(5.0);
  pt100_furnace->par_supply_voltage.Set(5.0);
  pt100_furnace->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT100_FURNACE));

  auto filter_furnace = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT100 Furnace Filter");
  filter_furnace->par_number_of_ports.Set(1);
  filter_furnace->Init();
  filter_furnace->par_weight.Set(0.01);
  filter_furnace->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_furnace->in_input_values.at(0).ConnectTo(pt100_furnace->out_temperature);
  filter_furnace->out_filtered_values.at(0).ConnectTo(controller->si_temperature_furnace);

  auto pt100_garage = new shared::mPT100(this, "PT100 Garage");
  pt100_garage->par_pre_resistance.Set(93.5);
  pt100_garage->par_reference_voltage.Set(5.0);
  pt100_garage->par_supply_voltage.Set(5.0);
  pt100_garage->in_voltage.ConnectTo(mcp_3008->out_voltage.at(tMCP3008Output::ePT100_GARAGE));

  auto filter_garage = new signal_filters::mExponentialFilter<rrlib::si_units::tCelsius<double>>(this, "PT100 Garage Filter");
  filter_garage->par_number_of_ports.Set(1);
  filter_garage->Init();
  filter_garage->par_weight.Set(0.01);
  filter_garage->par_initial_value.Set(rrlib::si_units::tCelsius<double>(20.0));
  filter_garage->in_input_values.at(0).ConnectTo(pt100_garage->out_temperature);
  filter_garage->out_filtered_values.at(0).ConnectTo(controller->si_temperature_garage);

  this->so_temperature_room.ConnectTo(filter_room->out_filtered_values.at(0));
  this->so_temperature_ground.ConnectTo(filter_ground->out_filtered_values.at(0));
  this->so_temperature_solar.ConnectTo(filter_solar->out_filtered_values.at(0));
  this->so_temperature_boiler_middle.ConnectTo(filter_boiler_middle->out_filtered_values.at(0));
  this->so_temperature_boiler_top.ConnectTo(filter_boiler_top->out_filtered_values.at(0));
  this->so_temperature_boiler_bottom.ConnectTo(filter_boiler_bottom->out_filtered_values.at(0));
  this->so_temperature_furnace.ConnectTo(filter_furnace->out_filtered_values.at(0));
  this->so_temperature_garage.ConnectTo(filter_garage->out_filtered_values.at(0));

}

//----------------------------------------------------------------------
// gHeatControl destructor
//----------------------------------------------------------------------
gHeatControl::~gHeatControl()
{}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
