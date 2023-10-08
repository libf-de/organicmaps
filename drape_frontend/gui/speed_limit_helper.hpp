#pragma once

#include "platform/measurement_utils.hpp"

#include <string>

namespace gui
{
class SpeedLimitHelper
{
public:
  void SetSpeedLimit(double const speedLimitMps) { m_speedLimitMps = speedLimitMps; }
  bool IsSpeedLimitAvailable() const { return m_speedLimitMps > 0; }
  std::string GetSpeedLimit() const
  {
    return FormatSpeedNumeric(m_speedLimitMps, measurement_utils::GetMeasurementUnits());
  }

  void SetCurrentSpeed(double const currentSpeedMps) { m_currentSpeedMps = currentSpeedMps; }
  std::string GetCurrentSpeed() const
  {
    return FormatSpeedNumeric(m_currentSpeedMps, measurement_utils::GetMeasurementUnits());
  }

  void SetSpeedingTolerance(double const toleranceMps) { m_speedingToleranceMps = toleranceMps; }
  double GetSpeedingTolerance() const { return m_speedingToleranceMps; }
  bool IsSpeeding() const
  {
    return IsSpeedLimitAvailable() && m_currentSpeedMps > (m_speedLimitMps + m_speedingToleranceMps);
  }

private:
#ifdef DEBUG
  double m_speedLimitMps = 34.16;
#else
  double m_speedLimitMps = 0.0;
#endif

  double m_currentSpeedMps = 0.0;
  double m_speedingToleranceMps = 0.0;
};
}  // namespace gui
