#include "BotManager.h"

#include "Game.h"
#include "Player.h"
#include "Controller.h"
#include "InputQueue.h"

BotManager::BotManager(Game& g, InputQueue& q) : game(g), inputQueue(q) {}

void BotManager::update() {
    for (uint32_t botId : botIds) {
        Player& p = game.getPlayerInfo(botId);

        if (!p.controller) continue;

        auto dirOpt = p.controller->update(game);
        if (!dirOpt) continue;

        PlayerInput input;
        input.playerId = botId;
        input.dir = *dirOpt;

        inputQueue.push(input); 
    }
}