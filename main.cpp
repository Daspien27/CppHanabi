#define NOMINMAX

#include <random>
#include <variant>
#include <array>
#include <tuple>
#include <optional>
#include <iostream>
#include <termcolor/termcolor.hpp>
#include <string_view>

template <typename T>
struct dependent_false : std::false_type {};

namespace hanabi
{
	constexpr std::string_view rank_name(size_t n)
	{
		using namespace std::string_view_literals;
		switch (n)
		{
		case 0: return "1"sv;
		case 1: return "2"sv;
		case 2: return "3"sv;
		case 3: return "4"sv;
		case 4: return "5"sv;
		default: throw std::runtime_error("Unexpected rank.");
		}
	}

	template <size_t N>
	struct rank 
	{
		constexpr static std::string_view display_rank()
		{
			return rank_name(N);
		}
	};

	constexpr auto termcolor_color(size_t n)
	{
		switch (n)
		{
		case 0: return termcolor::red;
		case 1: return termcolor::green;
		case 2: return termcolor::yellow;
		case 3: return termcolor::cyan;
		case 4: return termcolor::white;
		default: throw std::runtime_error("Unexpected color.");
		}
	}

	constexpr auto termcolor_on_color(size_t n)
	{
		switch (n)
		{
		case 0: return termcolor::on_red;
		case 1: return termcolor::on_green;
		case 2: return termcolor::on_yellow;
		case 3: return termcolor::on_cyan;
		case 4: return termcolor::on_white;
		default: throw std::runtime_error("Unexpected color.");
		}
	}

	constexpr auto color_name(size_t n)
	{
		using namespace std::string_view_literals;
		switch (n)
		{
		case 0: return "red"sv;
		case 1: return "green"sv;
		case 2: return "yellow"sv;
		case 3: return "cyan"sv;
		case 4: return "white"sv;
		default: throw std::runtime_error("Unexpected color.");
		}
	}


	template <size_t N> 
	struct color 
	{
		constexpr static auto display_name()
		{
			return color_name(N);
		}

		constexpr static auto display_color()
		{
			return termcolor_color(N);
		}

		constexpr static auto display_on_color()
		{
			return termcolor_on_color(N);
		}
	};

	namespace location
	{
		struct draw_pile {};
		struct discard_pile {};
		struct in_play {};
		struct hand { int player_; };
	}

	template <typename Action> struct action;
	struct play;
	template <typename Property> struct hint;
	struct discard;

	template <typename CardInfo, typename Is>
	struct card_frequency_impl;

	template <typename CardInfo, size_t... Ns>
	struct card_frequency_impl <CardInfo, std::index_sequence<Ns...>>
	{
		using tuple = std::tuple<std::enable_if_t<Ns == Ns, CardInfo>...>;
	};

	template <typename Color, typename Rank>
	struct card_info {};

	template <typename Color, typename Rank, size_t Freq> 
	struct card_frequency 
	{
		using tuple = typename card_frequency_impl<card_info<Color, Rank>, std::make_index_sequence<Freq>>::tuple;
	};

	namespace configuration
	{
		template <typename Configuration>
		struct configuration_traits
		{
			static constexpr size_t num_players = Configuration::num_players;
			static constexpr size_t hand_size = Configuration::hand_size;
			static constexpr int max_num_hints = Configuration::max_num_hints;
			static constexpr int starting_num_hints = Configuration::starting_num_hints;
			static constexpr int max_num_mistakes = Configuration::max_num_mistakes;

			template <template <typename... TArgs> typename Container>
			using ranks = typename Configuration::template ranks<Container>;

			template <template <typename... TArgs> typename Container>
			using colors = typename Configuration::template colors<Container>;

			template <template <typename... TArgs> typename Container>
			using locations = typename Configuration::template locations<Container>;

			template <template <typename... TArgs> typename Container>
			using actions = typename Configuration::template actions<Container>;

			using card_frequencies = typename Configuration::card_frequencies;

			static constexpr size_t deck_size = []()
			{
				return [] <typename... Colors, typename... Ranks, size_t... Freqs> (std::tuple<card_frequency<Colors, Ranks, Freqs>...>&&)
				{
					return (Freqs + ...);
				} (card_frequencies{});
			}();
		};

		struct default_t
		{
			static constexpr auto num_players = 2;
			static constexpr auto hand_size = 5;
			static constexpr auto max_num_hints = 8;
			static constexpr auto starting_num_hints = 8;
			static constexpr auto max_num_mistakes = 3;


			template <template <typename... TArgs> typename Container> //for use with std::variant, std::tuple, etc.
			using ranks = Container<rank<0>, rank<1>, rank<2>, rank<3>, rank<4>>;
	
			template <template <typename... TArgs> typename Container>
			using colors = Container<color<0>, color<1>, color<2>, color<3>, color<4>>;

			template <template <typename... TArgs> typename Container>
			using locations = Container<location::draw_pile, location::hand, location::discard_pile, location::in_play>;

			template <template <typename... TArgs> typename Container>
			using actions = Container<action<play>, action<discard>,
				hint<color<0>>, hint<color<1>>, hint<color<2>>, hint<color<3>>, hint<color<4>>,
				hint<rank<0>>, hint<rank<1>>, hint<rank<2>>, hint<rank<3>>, hint<rank<4>>>;

			using card_frequencies = std::tuple<

				card_frequency<color<0>, rank<0>, 3>, card_frequency<color<0>, rank<1>, 2>, card_frequency<color<0>, rank<2>, 2>,
				card_frequency<color<0>, rank<3>, 2>, card_frequency<color<0>, rank<4>, 1>,
				
				card_frequency<color<1>, rank<0>, 3>, card_frequency<color<1>, rank<1>, 2>, card_frequency<color<1>, rank<2>, 2>,
				card_frequency<color<1>, rank<3>, 2>, card_frequency<color<1>, rank<4>, 1>,
				
				card_frequency<color<2>, rank<0>, 3>, card_frequency<color<2>, rank<1>, 2>, card_frequency<color<2>, rank<2>, 2>,
				card_frequency<color<2>, rank<3>, 2>, card_frequency<color<2>, rank<4>, 1>,
				
				card_frequency<color<3>, rank<0>, 3>, card_frequency<color<3>, rank<1>, 2>, card_frequency<color<3>, rank<2>, 2>,
				card_frequency<color<3>, rank<3>, 2>, card_frequency<color<3>, rank<4>, 1>,
				
				card_frequency<color<4>, rank<0>, 3>, card_frequency<color<4>, rank<1>, 2>, card_frequency<color<4>, rank<2>, 2>,
				card_frequency<color<4>, rank<3>, 2>, card_frequency<color<4>, rank<4>, 1>
			>;
		};
	}

	template <typename Configuration>
	struct card
	{
		typename Configuration::template colors<std::variant> color_;
		typename Configuration::template ranks<std::variant> rank_;

		std::ostream& display_card(std::ostream& stream) const
		{
			const auto rank_display = std::visit([](const auto& r) { return r.display_rank(); }, rank_);
			const auto color_on_display = std::visit([](const auto& c) { return c.display_on_color(); }, color_);

			stream << color_on_display << termcolor::grey << rank_display << termcolor::reset;

			return stream;
		}
	};

	template <typename Property, bool default_v = true>
	struct possibility
	{
		bool possible_ = default_v;
	};

	template <typename... Properties>
	using possibility_tuple = std::tuple<possibility<Properties>...>;
	
	template <typename Configuration>
	struct knowledge
	{
		typename Configuration::template colors<possibility_tuple> hinted_colors_{};
		typename Configuration::template ranks<possibility_tuple> hinted_ranks_{};
	};

	template <typename Configuration>
	struct card_state
	{
		card<Configuration> card_;
		typename Configuration::template locations<std::variant> location_;
		knowledge<Configuration> knowledge_;
		int age_;
	};

	template <typename Configuration>
	struct deck_state
	{
		using configuration_t = typename configuration::configuration_traits<Configuration>;
		std::array<card_state<Configuration>, configuration_t::deck_size> cards_;
	};

	template <typename Configuration>
	struct game_state
	{
		deck_state<Configuration> deck_;
		int player_turn_;
		int num_available_hints_;
		int num_mistakes_;
		std::optional<int> next_card_to_draw_;
		std::optional<int> last_player_to_play_;
		bool last_player_has_played_;
	};

	template <typename Action> 
	struct action
	{
		Action a_;

		template <typename Configuration>
		constexpr bool validate(const game_state<Configuration>& check) const
		{
			return a_.validate(check);
		}

		template <typename Configuration>
		constexpr game_state<Configuration> perform (const game_state<Configuration>& source) const
		{
			if (!validate(source)) throw std::runtime_error("Action is not valid.");

			return a_.perform(source);
		}

		template <typename Configuration>
		std::ostream& display_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			return a_.display_action(stream, state);
		}

		template <typename Configuration>
		std::ostream& display_hidden_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			return a_.display_hidden_action(stream, state);
		}
	};

	template <typename Action>
	action(Action&&)->action<Action>;

	struct play
	{ 
		int card_;

		template <typename Configuration>
		constexpr bool is_playable(const game_state<Configuration>& source) const
		{
			const auto& this_card = source.deck_.cards_[card_].card_;
			const auto& this_color = this_card.color_;
			typename Configuration::template ranks<possibility_tuple> played_ranks_this_color;

			for (const auto& card_in_deck : source.deck_.cards_)
			{
				if (card_in_deck.card_.color_.index() == this_color.index() && std::holds_alternative<location::in_play> (card_in_deck.location_))
				{
					std::visit([&](const auto& card_rank)
						{
							std::get<possibility<std::remove_cv_t<
													std::remove_reference_t<decltype(card_rank)>>>>(played_ranks_this_color).possible_ = false;
						}, card_in_deck.card_.rank_);
				}
			}

			return std::apply([&]<typename... Ranks> (const possibility<Ranks>&... ranks)
			{
			    const auto idx = this_card.rank_.index ();
			    std::remove_cv_t<decltype(idx)> j = 0;

			    const bool consume = ((j+=1, !ranks.possible_) && ...);

			    return idx+1==j && !consume;
			}, played_ranks_this_color);
		}

		template <typename Configuration>
		constexpr bool validate(const game_state<Configuration>& check) const
		{
			auto card_location = std::get_if<location::hand>(&check.deck_.cards_[card_].location_);
			return card_location && card_location->player_ == check.player_turn_;
		}

		template <typename Configuration>
		constexpr game_state<Configuration> perform(const game_state<Configuration>& source) const
		{
			auto target = source;

			if (is_playable(source))
			{
				target.deck_.cards_[card_].location_ = location::in_play{};

				if (std::holds_alternative<rank<4>>(target.deck_.cards_[card_].card_.rank_)
					&& target.num_available_hints_ < Configuration::max_num_hints)
				{
					++target.num_available_hints_;
				}
			}
			else
			{
				target.deck_.cards_[card_].location_ = location::discard_pile{};
				++target.num_mistakes_;
			}

			if (target.next_card_to_draw_.has_value())
			{
				if (!std::holds_alternative<location::draw_pile>(target.deck_.cards_[target.next_card_to_draw_.value()].location_))
				{
					throw std::runtime_error("Next card to draw was not in the deck");
				}

				target.deck_.cards_[target.next_card_to_draw_.value()].location_ = location::hand{ target.player_turn_ };

				++target.next_card_to_draw_.value();
				if (target.next_card_to_draw_.value() >= target.deck_.cards_.size())
				{
					target.next_card_to_draw_ = std::nullopt;
					target.last_player_to_play_ = target.player_turn_;
				}
			}

			if (target.last_player_to_play_.has_value() && target.last_player_to_play_.value() == target.player_turn_) target.last_player_has_played_ = true;

			++target.player_turn_;
			target.player_turn_ %= 2;
			return target;
		}

		template <typename Configuration>
		std::ostream& display_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			stream << "playing ";
			state.deck_.cards_[card_].card_.display_card(stream) << (is_playable(state) ? " it worked!\n" : " it was a mistake.\n");
			return stream;
		}

		template <typename Configuration>
		std::ostream& display_hidden_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			stream << "playing card #" << card_ << '\n';
			return stream;
		}
	};

	template <typename Property>
	struct contains_this_property
	{
		template <typename... Properties>
		using type = std::disjunction<std::is_same<Property, Properties>...>;
	};

	template <typename Property, typename Configuration>
	using is_property_a_color = typename Configuration::template colors<contains_this_property<Property>::template type>;

	template <typename Property, typename Configuration>
	inline constexpr bool is_property_a_color_v = is_property_a_color<Property, Configuration>::value;

	template <typename Property, typename Configuration>
	using is_property_a_rank = typename Configuration::template ranks<contains_this_property<Property>::template type>;
	
	template <typename Property, typename Configuration>
	inline constexpr bool is_property_a_rank_v = is_property_a_rank<Property, Configuration>::value;

	template <typename Property> //card color or rank
	struct hint 
	{ 
		int player_; 

		template <typename Configuration>
		constexpr bool validate(const game_state<Configuration>& check) const noexcept
		{
			const bool can_hint = check.num_available_hints_ > 0;
			const bool can_hint_player = check.player_turn_ != player_; //cannot hint oneself
			
			if (!can_hint || !can_hint_player) return false;

			for (const auto& card_in_deck : check.deck_.cards_)
			{

				const auto holds_this_property = [&]()
				{
					if constexpr (is_property_a_color_v<Property, Configuration> != is_property_a_rank_v<Property, Configuration>)
					{
						if constexpr (is_property_a_color_v<Property, Configuration>)
						{
							return (std::holds_alternative<Property>(card_in_deck.card_.color_));
						}
						else
						{
							return (std::holds_alternative<Property>(card_in_deck.card_.rank_));
						}
					}
					else
					{
						static_assert(dependent_false<Configuration>::value, "Hint for this property is ill-formed. Hint can neither be for both rank or color (and must be at least one).");
					}
				}();

				if (auto maybe_in_hand = std::get_if<location::hand>(&card_in_deck.location_); 
					holds_this_property && maybe_in_hand && maybe_in_hand->player_ == player_)
				{
					return true;
				}
			}

			return false;
		}

		template <typename Configuration>
		constexpr game_state<Configuration> perform(const game_state<Configuration>& source) const
		{
			auto target = source;
			--target.num_available_hints_;

			for (auto& card_in_deck : target.deck_.cards_)
			{
				if (auto this_player = std::get_if<location::hand>(&card_in_deck.location_); this_player&& this_player->player_ == player_)
				{
					if constexpr (is_property_a_color_v<Property, Configuration> != is_property_a_rank_v<Property, Configuration>)
					{
						if constexpr (is_property_a_color_v<Property, Configuration>)
						{
							if (std::holds_alternative<Property>(card_in_deck.card_.color_))
							{
								std::apply([&]<typename... Colors> (possibility<Colors> &... colors)
								{
									((colors.possible_ = false), ...);
								}, card_in_deck.knowledge_.hinted_colors_);

								std::get<possibility<Property>>(card_in_deck.knowledge_.hinted_colors_).possible_ = true;
							}
							else
							{
								std::get<possibility<Property>>(card_in_deck.knowledge_.hinted_colors_).possible_ = false;
							}
						}
						else
						{
							if (std::holds_alternative<Property>(card_in_deck.card_.rank_))
							{
								std::apply([&]<typename... Ranks> (possibility<Ranks>&... ranks)
								{
									((ranks.possible_ = false), ...);
								}, card_in_deck.knowledge_.hinted_ranks_);

								std::get<possibility<Property>>(card_in_deck.knowledge_.hinted_ranks_).possible_ = true;
							}
							else
							{
								std::get<possibility<Property>>(card_in_deck.knowledge_.hinted_ranks_).possible_ = false;
							}
						}
					}
					else
					{
						static_assert(dependent_false<Configuration>::value, "Hint for this property is ill-formed. Hint can neither be for both rank or color (and must be at least one).");
					}
				}
			}

			if (target.last_player_to_play_.has_value() && target.last_player_to_play_.value() == target.player_turn_) target.last_player_has_played_ = true;

			++target.player_turn_;
			target.player_turn_ %= 2;
			return target;
		}

		template <typename Configuration>
		std::ostream& display_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			if constexpr (is_property_a_color_v<Property, Configuration> != is_property_a_rank_v<Property, Configuration>)
			{
				stream << "hinting about all of the ";
				
				if constexpr (is_property_a_color_v<Property, Configuration>)
				{
					stream << Property::display_color() << termcolor::bold << Property::display_name() << termcolor::reset << " cards.\n";
				}
				else
				{
					stream << termcolor::magenta << termcolor::bold << Property::display_rank() << termcolor::reset << "s in hand.\n";
				}
			}
			else
			{
				static_assert(dependent_false<Configuration>::value, "Hint for this property is ill-formed. Hint can neither be for both rank or color (and must be at least one).");
			}

			return stream;
		}

		template <typename Configuration>
		std::ostream& display_hidden_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			return display_action(stream, state);
		}
	};
	
	struct discard
	{
		int card_; 

		template <typename Configuration>
		constexpr bool validate(const game_state<Configuration>& check) const
		{
			auto card_location = std::get_if<location::hand> (&check.deck_.cards_[card_].location_);
			return card_location && card_location->player_ == check.player_turn_;
		}

		template <typename Configuration>
		constexpr game_state<Configuration> perform(const game_state<Configuration>& source) const
		{
			auto target = source;

			target.deck_.cards_[card_].location_ = location::discard_pile{};
				
			if (target.num_available_hints_ < Configuration::max_num_hints)
			{
				++target.num_available_hints_;
			}

			if (target.next_card_to_draw_.has_value())
			{
				if (!std::holds_alternative<location::draw_pile>(target.deck_.cards_[target.next_card_to_draw_.value()].location_))
				{
					throw std::runtime_error("Next card to draw was not in the deck");
				}

				target.deck_.cards_[target.next_card_to_draw_.value()].location_ = location::hand{ target.player_turn_ };

				++target.next_card_to_draw_.value();
				if (target.next_card_to_draw_.value() >= target.deck_.cards_.size())
				{
					target.next_card_to_draw_ = std::nullopt;
					target.last_player_to_play_ = target.player_turn_;
				}
			}

			++target.player_turn_;
			target.player_turn_ %= 2;
			return target;
		}

		template <typename Configuration>
		std::ostream& display_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			stream << "discarding ";
			state.deck_.cards_[card_].card_.display_card(stream) << '\n';
			return stream;
		}

		template <typename Configuration>
		std::ostream& display_hidden_action(std::ostream& stream, const game_state<Configuration>& state) const
		{
			stream << "discarding card #" << card_ << '\n';
			return stream;
		}
	};

	template <typename... Properties>
	using possible_hints = std::tuple<possibility<Properties, false>...>;

	template <typename Configuration>
	struct possible_hint_options
	{
		typename Configuration::template colors<possible_hints> possible_colors_{};
		typename Configuration::template ranks<possible_hints> possible_ranks_{};
	};

	template <typename Configuration>
	constexpr auto find_all_possible_actions(const game_state<Configuration>& state)
	{
		std::vector<typename Configuration::template actions<std::variant>> possible_actions;

		possible_hint_options<Configuration> possible_hints_choices{};

		for (int i = 0; i < state.deck_.cards_.size(); ++i)
		{

			const auto& card_in_deck = state.deck_.cards_[i];

			if (auto card_in_hand = std::get_if<location::hand>(&card_in_deck.location_); card_in_hand)
			{
				if (card_in_hand->player_ == state.player_turn_)
				{
					possible_actions.emplace_back(action{ play{ i } });
					possible_actions.emplace_back(action{ discard{ i } });
				}
				else
				{
					std::visit([&](const auto& card_color)
						{
							std::get<possibility<std::remove_cvref_t<decltype(card_color)>, false>>(possible_hints_choices.possible_colors_).possible_ = true;
						}, card_in_deck.card_.color_);

					std::visit([&](const auto& card_rank)
						{
							std::get<possibility<std::remove_cvref_t<decltype(card_rank)>, false>>(possible_hints_choices.possible_ranks_).possible_ = true;
						}, card_in_deck.card_.rank_);
				}
			}
		}

		int opposite_player = (state.player_turn_ + 1) % 2;
		std::apply([&] <typename... Colors> (possibility<Colors, false>... possible_colors)
		{
			[[maybe_unused]] void* consume;
			[[maybe_unused]] auto dummy = ((consume = static_cast<void*> (((possible_colors.possible_) ? &possible_actions.emplace_back(hint<Colors> {opposite_player}) : nullptr))), ..., 0);
		}, possible_hints_choices.possible_colors_);

		std::apply([&] <typename... Ranks> (possibility<Ranks, false>... possible_ranks)
		{
			[[maybe_unused]] void* consume;
			[[maybe_unused]] auto dummy = ((consume = static_cast<void*> (((possible_ranks.possible_) ? &possible_actions.emplace_back(hint<Ranks> {opposite_player}) : nullptr))), ..., 0);
		}, possible_hints_choices.possible_ranks_);

		return possible_actions;
	}


	namespace controller
	{
		template <typename Controller>
		struct player_controller
		{
			Controller control_;

			template <typename... TArgs>
				player_controller(TArgs &&... args) : control_(std::in_place, std::forward<TArgs>(args)...)
			{
			}

			template <typename GameState>
			auto perform(const GameState& state)
			{
				return control_.perform(state);
			}
		};

		template <typename Configuration>
		struct human
		{
			std::mt19937& gen_;

			human(std::in_place_t, std::mt19937& gen) : gen_(gen)
			{

			}

			void print_player_know(const game_state<Configuration>& state, int player)
			{
				std::cout << "    \t| " << "rank      " << " | " << "color     " << '\n';

				std::cout << "card\t| ";

				std::apply([] <typename... Ranks> (Ranks &&...)
				{
					[[maybe_unused]] char dummy = ((std::cout << Ranks::display_rank() << ' ', 0), ...);
				}, typename Configuration::template ranks<std::tuple>{});

				std::cout << " | ";

				std::apply([] <typename... Colors> (Colors &&...)
				{
					[[maybe_unused]] char dummy = ((std::cout << Colors::display_color() << Colors::display_name()[0] << termcolor::reset << ' ', 0), ...);
				}, typename Configuration::template colors<std::tuple>{});

				std::cout << '\n';

				std::cout << "--------|------------|-----------\n";

				for (int i = 0; i < state.deck_.cards_.size(); ++i)
				{
					const auto& card_in_deck = state.deck_.cards_[i];

					if (const auto in_hand = std::get_if<location::hand>(&card_in_deck.location_); in_hand&& in_hand->player_ == player)
					{
						std::cout << '#' << i << ' ';

						if (player != state.player_turn_)
						{
							card_in_deck.card_.display_card(std::cout);
						}

						std::cout << "\t| ";

						std::apply([] <typename... Ts> (const possibility<Ts> &... p)
						{
							[[maybe_unused]] char dummy = ((std::cout << p.possible_ << ' ', 0), ...);
						}, card_in_deck.knowledge_.hinted_ranks_);

						std::cout << " | ";

						std::apply([] <typename... Ts> (const possibility<Ts> &... p)
						{
							[[maybe_unused]] char dummy = ((std::cout << p.possible_ << ' ', 0), ...);
						}, card_in_deck.knowledge_.hinted_colors_);

						std::cout << '\n';
					}
				}
			}
			void print_know(const game_state<Configuration>& state)
			{
				std::cout << "your know:\n";
				print_player_know(state, state.player_turn_);

				std::cout << "partners know:\n";
				print_player_know(state, (1 + state.player_turn_) % 2);
			}

			typename Configuration::template actions<std::variant> perform(const game_state<Configuration>& state)
			{
				auto possible_actions = find_all_possible_actions(state);
				
				print_know(state);

				std::cout << "You may take one of " << possible_actions.size() << " actions.\n";

				for (int i = 0; i < possible_actions.size(); ++i)
				{
					std::visit([&](const auto& action)
						{
							std::cout << '\t' << i << ". ";
							action.display_hidden_action(std::cout, state);
						}, possible_actions[i]);
				}

				std::uniform_int_distribution<size_t> dis(0, possible_actions.size() - 1);
				auto choice = dis(gen_);
				std::cout << "I will randomly play for you. " << "Your choice is " << choice << ".\n";
			
				return possible_actions[choice];
			}

		};

		template <typename Configuration>
		struct random_ai
		{
			std::mt19937& gen_;

			random_ai(std::in_place_t, std::mt19937& gen) : gen_(gen)
			{
			}

			typename Configuration::template actions<std::variant> perform(const game_state<Configuration>& state)
			{
				auto possible_actions = find_all_possible_actions(state);

				std::uniform_int_distribution<size_t> dis(0, possible_actions.size() - 1);

				return possible_actions[dis(gen_)];
			}

		};


		template <typename Configuration, size_t... Ns, typename... Controllers>
		auto choose_player_controller_action_impl(const game_state<Configuration>& state, std::index_sequence<Ns...>, std::tuple<player_controller<Controllers>...> player_controllers)
		{
			
			std::optional<typename Configuration::template actions<std::variant>> action;

			[[maybe_unused]] bool dummy = ((action == std::nullopt && (((state.player_turn_ == Ns) ? action = std::get<Ns>(player_controllers).perform(state) : action = std::nullopt), true)) && ...);

			return action.value();
		}

		template <typename Configuration, typename... Controllers>
		auto choose_player_controller_action(const game_state<Configuration>& state, std::tuple<player_controller<Controllers>...> player_controllers)
		{
			return choose_player_controller_action_impl(state, std::make_index_sequence<sizeof...(Controllers)>{}, player_controllers);
		}
	}

	template<typename Configuration = hanabi::configuration::default_t>
	class game
	{
	public:

		using configuration_t = typename configuration::configuration_traits<Configuration>;

		template <typename Gen>
		void init(Gen& gen)
		{
			auto initial_card_list = std::apply([] <typename... Colors, typename... Ranks, size_t... Freqs> (card_frequency<Colors, Ranks, Freqs>&&...)
			{
				return std::apply([] <typename... Colors, typename... Ranks> (card_info<Colors, Ranks>&&...)
				{
					return std::array{ card<Configuration>{Colors{}, Ranks{}}... };
				}, std::tuple_cat(typename card_frequency<Colors, Ranks, Freqs>::tuple{}...));
			}, typename configuration_t::card_frequencies{});

			std::shuffle(initial_card_list.begin(), initial_card_list.end(), gen);

			game_state<Configuration> init_state;

			std::transform(initial_card_list.begin(), initial_card_list.end(), init_state.deck_.cards_.begin(), [](const auto& card_info)
				{
					card_state<Configuration> init_state{};
					init_state.card_ = card_info;

					return init_state;
				});


			init_state.num_available_hints_ = configuration_t::max_num_hints;
			init_state.num_mistakes_ = 0;
			init_state.player_turn_ = 0;

			auto deal_out_hand_to_player = [player=0](const auto begin) mutable
			{
				return std::for_each_n(begin, configuration_t::hand_size, [p = player++](auto& dealt_card)
					{
						dealt_card.location_ = location::hand{ p };
					});
			};
			auto start_for_second_player = deal_out_hand_to_player(init_state.deck_.cards_.begin());
			auto start_of_deck = deal_out_hand_to_player(start_for_second_player);
			
			std::for_each(start_of_deck, init_state.deck_.cards_.end(), [age = 1](auto& card_info) mutable
			{
				card_info.age_ = age++;
			});

			init_state.next_card_to_draw_ = configuration_t::hand_size * 2;
			init_state.last_player_to_play_ = std::nullopt;
			init_state.last_player_has_played_ = false;

			game_states_.emplace_back(init_state, std::nullopt);
		}

		static constexpr bool game_is_over(const game_state<Configuration>& state)
		{
			const bool too_many_mistakes = state.num_mistakes_ >= configuration_t::max_num_mistakes;
			const bool last_turn_has_happened = state.last_player_has_played_;
			const bool no_more_possible_moves = false;

			return too_many_mistakes || last_turn_has_happened || no_more_possible_moves;
		}

		static constexpr int score_of(const game_state<Configuration>& state)
		{
			return std::apply([&] <typename... Colors> (const Colors&... colors)
			{
				auto highest_rank_this_color = [&]<typename Color> (const Color& this_color)
				{
					const auto max = std::max_element(state.deck_.cards_.begin(), state.deck_.cards_.end(), [&](const auto& Lhs, const auto& Rhs)
						{
							const auto LhsVal = (std::holds_alternative<Color>(Lhs.card_.color_) && std::holds_alternative<location::in_play>(Lhs.location_)) ? std::make_optional<int>(Lhs.card_.rank_.index() + 1) : std::nullopt;
							const auto RhsVal = (std::holds_alternative<Color>(Rhs.card_.color_) && std::holds_alternative<location::in_play>(Rhs.location_)) ? std::make_optional<int>(Rhs.card_.rank_.index() + 1) : std::nullopt;
							return LhsVal < RhsVal;
						});

					return (std::holds_alternative<Color> (max->card_.color_) && std::holds_alternative<location::in_play>(max->location_)) ? max->card_.rank_.index() + 1 : 0;
				};

				return ((highest_rank_this_color(colors)) + ...);
			}, typename Configuration::template colors<std::tuple>{});
		}

		static void display_hand_of(const game_state<Configuration>& state_to_display, int player)
		{
			std::cout << "player " << player << "s hand: ";

			for (const auto& card_in_deck : state_to_display.deck_.cards_)
			{
				if (auto card_in_hand = std::get_if<location::hand>(&card_in_deck.location_); card_in_hand&& card_in_hand->player_ == player)
				{
					card_in_deck.card_.display_card(std::cout) << ' ';
				}
			}

			std::cout << std::endl;
		}

		static void display_played_cards(const game_state<Configuration>& state_to_display)
		{
			std::cout << "played: ";

			std::apply([&] <typename... Colors> (const Colors&... colors)
			{
				auto display_on_color_or_blank = [&] <typename Color> (const Color& this_color)
				{
					const auto max = std::max_element(state_to_display.deck_.cards_.begin(), state_to_display.deck_.cards_.end(), [&](const auto& Lhs, const auto& Rhs)
						{
							const auto LhsVal = (std::holds_alternative<Color>(Lhs.card_.color_) && std::holds_alternative<location::in_play>(Lhs.location_)) ? std::make_optional<int>(Lhs.card_.rank_.index() + 1) : std::nullopt;
							const auto RhsVal = (std::holds_alternative<Color>(Rhs.card_.color_) && std::holds_alternative<location::in_play>(Rhs.location_)) ? std::make_optional<int>(Rhs.card_.rank_.index() + 1) : std::nullopt;
							return LhsVal < RhsVal;
						});

					return (std::holds_alternative<Color>(max->card_.color_) && std::holds_alternative<location::in_play>(max->location_)) ? std::make_optional<card<Configuration>> (max->card_) : std::nullopt;
				};

				std::optional<card<Configuration>> dis;

				((dis = display_on_color_or_blank(colors), (dis.has_value()) ? dis->display_card(std::cout) << ' ' : std::cout << "  "), ...);
				std::cout << std::endl;

			}, typename Configuration::template colors<std::tuple>{});
		}

		static void display_mistakes_and_hints(const game_state<Configuration>& state_to_display)
		{
			std::cout << "num available hints: " << state_to_display.num_available_hints_ << ", num mistakes made: " << state_to_display.num_mistakes_ << '\n';
		}

		static void display_discard(const game_state<Configuration>& state_to_display)
		{

		}

		static void display_state(const game_state<Configuration>& state_to_display)
		{
			std::cout << "-------------\n";
			std::cout << "player " << state_to_display.player_turn_ << "s turn\n";
			display_hand_of(state_to_display, (state_to_display.player_turn_ + 1) % 2);
			display_played_cards(state_to_display);
			display_mistakes_and_hints(state_to_display);
			display_discard(state_to_display);

		}
		template <typename Gen>
		void run(Gen& gen, bool display) //rng for seeding
		{
			if (display)
			{
				std::cout << "start: \n";
				display_hand_of(game_states_.back().first, 0);
				display_hand_of(game_states_.back().first, 1);
				std::cout << "\n\n";
			}

			std::tuple<controller::player_controller<controller::random_ai<Configuration>>, controller::player_controller<controller::human<Configuration>>> player_controllers = { gen, gen };

			while (!game_is_over(game_states_.back().first))
			{
				const auto& state = game_states_.back().first;

				if (display) display_state(state);

				auto pc_action = controller::choose_player_controller_action(state, player_controllers);

				std::visit([&](const auto& action)
				{
					if (display) action.display_action(std::cout, state);
					game_states_.emplace_back(action.perform(state), action);
				}, pc_action);
			}

			if (display) display_state(game_states_.back().first);
			final_score_ = score_of(game_states_.back ().first);
		}

		constexpr std::optional<int> final_score () const
		{
			return final_score_;
		}
	private:

		std::vector<std::pair<game_state<Configuration>, std::optional<typename configuration_t::template actions<std::variant>>>> game_states_;
		std::optional<int> final_score_;
	};
}

int main()
{
	using hanabi_game = hanabi::game<>;
	
	std::random_device rd;

	std::random_device::result_type seed = 3517219547; //15 point game

	auto best_score_so_far = 0;

	//while (best_score_so_far < 15)
	//{
	//	seed = default_seed.value_or(rd());
	//	std::mt19937 gen(seed);

	//	hanabi_game game;

	//	game.init(gen);
	//	game.run(gen, false);

	//	best_score_so_far = std::max(game.final_score().value_or(0), best_score_so_far);
	//	std::cout << "score: " << game.final_score().value() << '\n';
	//}

	std::mt19937 best_gen(seed);
	hanabi_game game;

	game.init(best_gen);
	game.run(best_gen, true);
	std::cout << "best score: " << game.final_score().value() << '\n';
	std::cout << "best seed: " << seed << '\n';
}
