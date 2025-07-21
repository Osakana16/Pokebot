export module pokebot.api.command_executors;
import pokebot.game.cs.team;
import pokebot.game.cs.model;

import pokebot.game.util;
export namespace pokebot::api {
    class BotCommandExecutor {
    public:
        virtual ~BotCommandExecutor() = default;
        virtual void AddBot(const std::string_view& botName, pokebot::game::Team team, pokebot::game::Model model) = 0;
        virtual bool IsPlayable() = 0;
    };

    inline std::unique_ptr<BotCommandExecutor> command_executor = nullptr;
}