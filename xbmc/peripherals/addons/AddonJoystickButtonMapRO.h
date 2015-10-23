/*
 *      Copyright (C) 2014-2015 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "addons/include/kodi_peripheral_types.h"
#include "input/joysticks/JoystickDriverPrimitive.h"
#include "peripherals/addons/PeripheralAddon.h"

#include <map>
#include <string>

namespace PERIPHERALS
{
  class CAddonJoystickButtonMapRO
  {
  public:
    CAddonJoystickButtonMapRO(CPeripheral* device, const PeripheralAddonPtr& addon, const std::string& strControllerId);

    std::string ControllerID(void) const { return m_strControllerId; }
    bool Load(void);
    bool GetFeature(const CJoystickDriverPrimitive& primitive, std::string& feature);
    bool GetButton(const std::string& feature, CJoystickDriverPrimitive& button);
    bool GetAnalogStick(const std::string& feature,
                        CJoystickDriverPrimitive& up,
                        CJoystickDriverPrimitive& down,
                        CJoystickDriverPrimitive& right,
                        CJoystickDriverPrimitive& left);
    bool GetAccelerometer(const std::string& feature,
                          int& xIndex, bool& xInverted,
                          int& yIndex, bool& yInverted,
                          int& zIndex, bool& zInverted);

  private:
    typedef std::string Feature;
    typedef std::map<CJoystickDriverPrimitive, Feature> DriverMap;

    // Utility functions
    static HatDirection       ToHatDirection(JOYSTICK_DRIVER_HAT_DIRECTION driverDirection);
    static SemiAxisDirection  ToSemiAxisDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir);
    static DriverMap          ToDriverMap(const JoystickFeatureMap& features);
    static void               AddToDriverMap(DriverMap& driverMap, const ADDON::JoystickFeature* feature, const std::string& featureName);
    static CJoystickDriverPrimitive ToPrimitive(const ADDON::JoystickFeature* feature);

    CPeripheral* const  m_device;
    PeripheralAddonPtr  m_addon;
    const std::string   m_strControllerId;
    JoystickFeatureMap  m_features;
    DriverMap           m_driverMap;
  };
}
