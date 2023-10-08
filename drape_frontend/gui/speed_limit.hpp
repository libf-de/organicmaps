#pragma once

#include "drape_frontend/gui/shape.hpp"

namespace gui
{
enum SpeedLimitTextType
{
  SPEED_LIMIT,
  SPEED_LIMIT_OVER,
  CURRENT_SPEED,
  CURRENT_SPEED_OVER
};

class SpeedLimit : public Shape
{
public:
  explicit SpeedLimit(const Position & position)
    : Shape(position)
  {}

  void SetRadius(float radius) { m_pillRadius = radius; }
  void SetAspectRatio(float aspectRatio) { m_pillAspectRatio = aspectRatio; }
  void SetOutlineWidthRatio(float outlineWidthRatio) { m_outlineWidthRatio = outlineWidthRatio; }
  void SetBackgroundColor(dp::Color backgroundColor) { m_pillBackgroundColor = backgroundColor; }
  void SetOutlineColor(dp::Color outlineColor) { m_pillOutlineColor = outlineColor; }
  void SetTextColor(dp::Color textColor) { m_textColor = textColor; }
  drape_ptr<ShapeRenderer> Draw(ref_ptr<dp::GraphicsContext> context, ref_ptr<dp::TextureManager> tex) const;

private:
  void DrawBackground(ref_ptr<dp::GraphicsContext> context, ShapeControl & control) const;
  void DrawText(ref_ptr<dp::GraphicsContext> context, ShapeControl & control, ref_ptr<dp::TextureManager> tex, SpeedLimitTextType type) const;

  float m_pillRadius = 35.0f;
  float m_pillAspectRatio = 1.8f;
  float m_pillBottomOpacity = 0.6f;
  float m_outlineWidthRatio = 0.2f;
  dp::Color m_pillBackgroundColor = dp::Color::White();
  dp::Color m_pillOutlineColor = dp::Color::Red();
  dp::Color m_textColor = dp::Color::Black();
};
}  // namespace gui
