// pokebot/api/command_executors.ixx
export module pokebot.api.command_executors;

import pokebot.game.util;
export namespace pokebot::api {

    // コンソールコマンドがPokebotクラスに実行を依頼する操作を定義するインターフェース
    class BotCommandExecutor {
    public:
        virtual ~BotCommandExecutor() = default;
        virtual void AddBot(const std::string_view& botName, pokebot::game::Team team, pokebot::game::Model model) = 0;
        virtual bool IsPlayable() = 0;
    };

    // IBotCommandExecutor のインスタンスにアクセスするためのグローバルなポインタ
    // これは外部から設定されます（Pokebotモジュールから）。
    // inline を使うことで、複数の翻訳単位で定義されてもリンクエラーにならない。
    inline std::unique_ptr<BotCommandExecutor> command_executor = nullptr;
}