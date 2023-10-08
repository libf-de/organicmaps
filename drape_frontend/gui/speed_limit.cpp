#include "drape_frontend/gui/speed_limit.hpp"

#include "drape_frontend/batcher_bucket.hpp"
#include "drape_frontend/visual_params.hpp"

#include "drape_frontend/gui/drape_gui.hpp"
#include "drape_frontend/gui/gui_text.hpp"

#include "shaders/programs.hpp"

#include "drape/glsl_types.hpp"

#include <functional>
#include <utility>

using namespace std::placeholders;

namespace gui
{
namespace
{

struct PillVertex
{
  PillVertex() = default;

  PillVertex(glsl::vec2 const & position, glsl::vec3 const & color, glsl::vec3 const & outlineColor, float const radius,
             float const outlineWidthRatio, float const halfPillHeight, float const bottomOpacity)
    : m_position(position)
    , m_color(color)
    , m_outlineColor(outlineColor)
    , m_radius(radius)
    , m_outlineWidthRatio(outlineWidthRatio)
    , m_halfPillHeight(halfPillHeight)
    , m_bottomOpacity(bottomOpacity)
  {
  }

  static dp::BindingInfo GetBindingInfo()
  {
    dp::BindingFiller<PillVertex> filler(7);
    filler.FillDecl<glsl::vec2>("a_position");
    filler.FillDecl<glsl::vec3>("a_color");
    filler.FillDecl<glsl::vec3>("a_outlineColor");
    filler.FillDecl<float>("a_radius");
    filler.FillDecl<float>("a_outlineWidthRatio");
    filler.FillDecl<float>("a_halfPillHeight");
    filler.FillDecl<float>("a_bottomOpacity");
    return filler.m_info;
  }

  glsl::vec2 m_position{};
  glsl::vec3 m_color{};
  glsl::vec3 m_outlineColor{};
  float m_radius{};
  float m_outlineWidthRatio{};
  float m_halfPillHeight{};
  float m_bottomOpacity{};
};
using BackgroundVertexData = buffer_vector<PillVertex, 4>;

BackgroundVertexData CreateBackgroundVertexData(glsl::vec3 color, glsl::vec3 outlineColor, float radius,
                                                float outlineWidthRatio, float halfPillHeight, float bottomOpacity)
{
  BackgroundVertexData data;
  data.emplace_back(glsl::vec2(-1.0, halfPillHeight), color, outlineColor, radius, outlineWidthRatio, halfPillHeight,
                    bottomOpacity);
  data.emplace_back(glsl::vec2(-1.0, -halfPillHeight), color, outlineColor, radius, outlineWidthRatio, halfPillHeight,
                    bottomOpacity);
  data.emplace_back(glsl::vec2(1.0, halfPillHeight), color, outlineColor, radius, outlineWidthRatio, halfPillHeight,
                    bottomOpacity);
  data.emplace_back(glsl::vec2(1.0, -halfPillHeight), color, outlineColor, radius, outlineWidthRatio, halfPillHeight,
                    bottomOpacity);
  return data;
}

class TextHandle : public MutableLabelHandle
{
  using TBase = MutableLabelHandle;

public:
  TextHandle(uint32_t id, dp::Anchor anchor, m2::PointF const & pivot, ref_ptr<dp::TextureManager> textures,
             SpeedLimitTextType type)
    : TBase(id, anchor, pivot), m_type(type), m_pivot(pivot)
  {
    SetTextureManager(textures);
    LOG(LDEBUG, (pivot.x, pivot.y));
  }

private:
  void UpdateSpeeding(SpeedLimitHelper const & helper)
  {
    if (m_type == CURRENT_SPEED || m_type == SPEED_LIMIT)
      SetIsVisible(false);
    else if (m_type == CURRENT_SPEED_OVER)
    {
      SetIsVisible(true);
      SetContent(helper.GetCurrentSpeed());
    }
    else if (m_type == SPEED_LIMIT_OVER)
    {
      SetIsVisible(true);
      SetContent(helper.GetSpeedLimit());
    }
  }

  void UpdateNormal(SpeedLimitHelper const & helper)
  {
    if (m_type == CURRENT_SPEED_OVER || m_type == SPEED_LIMIT_OVER)
      SetIsVisible(false);
    else if (m_type == CURRENT_SPEED)
    {
      SetIsVisible(true);
      SetContent(helper.GetCurrentSpeed());
    }
    else if (m_type == SPEED_LIMIT)
    {
      SetIsVisible(true);
      SetContent(helper.GetSpeedLimit());
    }
  }
  bool Update(ScreenBase const & screen) override
  {
    if (SpeedLimitHelper const & helper = DrapeGui::GetSpeedLimitHelper(); !helper.IsSpeedLimitAvailable())
      SetIsVisible(false);
    else if (helper.IsSpeeding())
      UpdateSpeeding(helper);
    else
      UpdateNormal(helper);

    return TBase::Update(screen);
  }
  void SetPivot(glsl::vec2 const & pivot) override { TBase::SetPivot(pivot + glsl::vec2(m_pivot.x, m_pivot.y)); }

  SpeedLimitTextType m_type;
  m2::PointF m_pivot;
  float textOffset = 0;
};

class BackgroundHandle : public Handle
{
public:
  BackgroundHandle(uint32_t id, dp::Anchor anchor, m2::PointF const & pivot) : Handle(id, anchor, pivot) {}

  bool Update(const ScreenBase & screen) override
  {
    SpeedLimitHelper const & helper = gui::DrapeGui::GetSpeedLimitHelper();
    SetIsVisible(helper.IsSpeedLimitAvailable());

    return Handle::Update(screen);
  }
};
}  // namespace

drape_ptr<ShapeRenderer> SpeedLimit::Draw(ref_ptr<dp::GraphicsContext> context, ref_ptr<dp::TextureManager> tex) const
{
  ShapeControl control;
  DrawBackground(context, control);
  DrawText(context, control, tex, SPEED_LIMIT);
  DrawText(context, control, tex, SPEED_LIMIT_OVER);
  DrawText(context, control, tex, CURRENT_SPEED);
  DrawText(context, control, tex, CURRENT_SPEED_OVER);

  drape_ptr<ShapeRenderer> renderer = make_unique_dp<ShapeRenderer>();
  renderer->AddShapeControl(std::move(control));
  return renderer;
}

void SpeedLimit::DrawBackground(ref_ptr<dp::GraphicsContext> context, ShapeControl & control) const
{
  ASSERT_EQUAL(m_position.m_anchor, dp::Center, ("Only dp::Center is supported for Pill."));
  ASSERT_NOT_EQUAL(m_pillRadius, 0.0f, ("halfPillWidth (Radius) must be set."));
  ASSERT_GREATER_OR_EQUAL(m_outlineWidthRatio, 0.0f, ("Outline width ratio must be in the range [0.0, 1.0]."));
  ASSERT_LESS_OR_EQUAL(m_outlineWidthRatio, 1.0f, ("Outline width ratio must be in the range [0.0, 1.0]."));
  ASSERT_NOT_EQUAL(m_pillBackgroundColor, dp::Color::Transparent(), ("Color must be set."));

  float const radiusInPixels = m_pillRadius * df::VisualParams::Instance().GetVisualScale();
  BackgroundVertexData data =
      CreateBackgroundVertexData(glsl::ToVec3(m_pillBackgroundColor), glsl::ToVec3(m_pillOutlineColor), radiusInPixels,
                                 m_outlineWidthRatio, m_pillAspectRatio, m_pillBottomOpacity);

  auto state = df::CreateRenderState(gpu::Program::SpeedLimitPill, df::DepthLayer::GuiLayer);
  state.SetDepthTestEnabled(false);

  dp::AttributeProvider provider(1, 4);
  provider.InitStream(0, PillVertex::GetBindingInfo(), make_ref(data.data()));
  drape_ptr<dp::OverlayHandle> handle =
      make_unique_dp<BackgroundHandle>(EGuiHandle::GuiHandleSpeedSign, m_position.m_anchor, m_position.m_pixelPivot);

  dp::Batcher batcher(dp::Batcher::IndexPerQuad, dp::Batcher::VertexPerQuad);
  batcher.SetBatcherHash(static_cast<uint64_t>(df::BatcherBucket::Default));
  dp::SessionGuard guard(context, batcher, std::bind(&ShapeControl::AddShape, &control, _1, _2));
  batcher.InsertTriangleStrip(context, state, make_ref(&provider), std::move(handle));
}

void SpeedLimit::DrawText(ref_ptr<dp::GraphicsContext> context, ShapeControl & control, ref_ptr<dp::TextureManager> tex,
                          SpeedLimitTextType type) const
{
  ASSERT_EQUAL(m_position.m_anchor, dp::Center, ());

  MutableLabelDrawer::Params params;
  params.m_anchor = m_position.m_anchor;
  params.m_alphabet = "0123456789";
  params.m_maxLength = 3;
  params.m_font = DrapeGui::GetGuiTextFont();
  // params.m_font.m_size *= df::VisualParams::Instance().GetVisualScale() * 0.8f;
  params.m_font.m_size *= 2.0f;

  float topCircleCenter = m_pillRadius * df::VisualParams::Instance().GetVisualScale() * (m_pillAspectRatio - 1.0f);

  if (type == SPEED_LIMIT || type == CURRENT_SPEED || type == SPEED_LIMIT_OVER)
  {
    params.m_font.m_color = m_textColor;
    params.m_font.m_outlineColor = m_textColor;
  }
  else
  {
    params.m_font.m_color = m_pillOutlineColor;
    params.m_font.m_outlineColor = m_pillOutlineColor;
  }

  if (type == SPEED_LIMIT || type == SPEED_LIMIT_OVER)
    params.m_pivot = m2::PointF(0.0f, -topCircleCenter);
  else
    params.m_pivot = m2::PointF(0.0f, topCircleCenter);

  params.m_handleCreator = [tex, type](dp::Anchor anchor, m2::PointF const & pivot)
  { return make_unique_dp<TextHandle>(EGuiHandle::GuiHandleSpeedSignLabel, anchor, pivot, tex, type); };

  MutableLabelDrawer::Draw(context, params, tex, std::bind(&ShapeControl::AddShape, &control, _1, _2));
}

}  // namespace gui
