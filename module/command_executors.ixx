// pokebot/api/command_executors.ixx
export module pokebot.api.command_executors;

import pokebot.game.util;
export namespace pokebot::api {

    // �R���\�[���R�}���h��Pokebot�N���X�Ɏ��s���˗����鑀����`����C���^�[�t�F�[�X
    class BotCommandExecutor {
    public:
        virtual ~BotCommandExecutor() = default;
        virtual void AddBot(const std::string_view& botName, pokebot::game::Team team, pokebot::game::Model model) = 0;
        virtual bool IsPlayable() = 0;
    };

    // IBotCommandExecutor �̃C���X�^���X�ɃA�N�Z�X���邽�߂̃O���[�o���ȃ|�C���^
    // ����͊O������ݒ肳��܂��iPokebot���W���[������j�B
    // inline ���g�����ƂŁA�����̖|��P�ʂŒ�`����Ă������N�G���[�ɂȂ�Ȃ��B
    inline std::unique_ptr<BotCommandExecutor> command_executor = nullptr;
}