#pragma once

#include <gf/Entity.h>
#include <gf/Font.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>
#include <gf/Text.h>
#include <vector>

#include "../common/Protocol.h"

enum class EndAction {
    None,
    BackToLobby
};


class EndEntity : public gf::Entity {
public:
    explicit EndEntity();

    void pointTo(gf::Vector2f position);
    void triggerAction();

    void setEndData(GameEndReason reason, int lastScore);

    EndAction getAndResetLastAction();

    void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

private:
    gf::Font m_font;

    gf::WidgetContainer m_container;
    gf::TextButtonWidget m_enterWidget;

    EndAction m_lastAction = EndAction::None;

    GameEndReason m_endReason = GameEndReason::TIME_OUT; //vl par defaut
    int m_lastScore = 0;
};
