export module pokebot.bot.event;
import pokebot.common.event_handler;
import pokebot.plugin.observables;
import pokebot.util;

export namespace pokebot::bot {
	template<typename Event>
	using BotObservable = common::MapObservable<Event, util::PlayerName, util::PlayerName::Hash>;
}