export module pokebot.plugin.console.command: console_args;

void get_console_args(std::vector<std::string_view>* args) {
    for (int i = 1; ; i++) {
        const char* const arg = CMD_ARGV(i);
        if (arg == nullptr || strlen(arg) <= 0) {
            break;
        }
        args->push_back(arg);
    }
}