#include <random>
#include <variant>
#include <array>
#include <tuple>
#include <optional>

template <typename T>
struct dependent_false : std::false_type {};

namespace hanabi
{
	template <size_t> struct rank {};
	template <size_t> struct color {};

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

			static constexpr size_t deck_size() 
			{
				return std::apply([] <typename... Colors, typename... Ranks, size_t... Freqs> (card_frequency<Colors, Ranks, Freqs>&&...)
				{
					return (Freqs + ... + 0);
				}, card_frequencies{});
			}


		};
	}

	template <typename Configuration>
	struct card
	{
		typename Configuration::template colors<std::variant> color_;
		typename Configuration::template ranks<std::variant> rank_;
	};

	template <typename Property>
	struct possibility
	{
		bool possible_ = true;
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
		std::array<card_state<Configuration>, Configuration::deck_size()> cards_;
	};

	template <typename Configuration>
	struct game_state
	{
		deck_state<Configuration> deck_;
		int player_turn_;
		int num_available_hints_;
		int num_mistakes_;
		std::optional<int> next_card_to_draw;
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

			if (target.next_card_to_draw.has_value())
			{
				if (!std::holds_alternative<location::draw_pile>(target.deck_.cards_[target.next_card_to_draw.value()].location_))
				{
					throw std::runtime_error("Next card to draw was not in the deck");
				}

				target.deck_.cards_[target.next_card_to_draw.value()].location_ = location::hand{ target.player_turn_ };

				++target.next_card_to_draw.value();
				if (target.next_card_to_draw.value() >= target.deck_.cards_.size()) target.next_card_to_draw = std::nullopt;
			}

			++target.player_turn_;
			target.player_turn_ %= 2;
			return target;
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

			++target.player_turn_;
			target.player_turn_ %= 2;
			return target;
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

			if (target.next_card_to_draw.has_value())
			{
				if (!std::holds_alternative<location::draw_pile>(target.deck_.cards_[target.next_card_to_draw.value()].location_))
				{
					throw std::runtime_error("Next card to draw was not in the deck");
				}

				target.deck_.cards_[target.next_card_to_draw.value()].location_ = location::hand{ target.player_turn_ };

				++target.next_card_to_draw.value();
				if (target.next_card_to_draw.value() >= target.deck_.cards_.size()) target.next_card_to_draw = std::nullopt;
			}

			++target.player_turn_;
			target.player_turn_ %= 2;
			return target;
		}
	};


	template<typename Configuration = hanabi::configuration::default_t>
	class game
	{
		std::vector<game_state<Configuration>> game_states_;

	public:

		template <typename Gen>
		void init(Gen& gen)
		{
			auto initial_card_list = std::apply([] <typename... Colors, typename... Ranks, size_t... Freqs> (card_frequency<Colors, Ranks, Freqs>&&...)
			{
				return std::apply([] <typename... Colors, typename... Ranks> (card_info<Colors, Ranks>&&...)
				{
					return std::array{ card<Configuration>{Colors{}, Ranks{}}... };
				}, std::tuple_cat(typename card_frequency<Colors, Ranks, Freqs>::tuple{}...));
			}, typename Configuration::card_frequencies{});

			static_assert(std::tuple_size_v<decltype(initial_card_list)> == Configuration::deck_size(), "Total number of cards in deck does not match frequency count.");

			std::shuffle(initial_card_list.begin(), initial_card_list.end(), gen);

			game_state<Configuration> init_state;

			std::transform(initial_card_list.begin(), initial_card_list.end(), init_state.deck_.cards_.begin(), [](const auto& card_info)
				{
					card_state<Configuration> init_state{};
					init_state.card_ = card_info;

					return init_state;
				});


			init_state.num_available_hints_ = Configuration::max_num_hints;
			init_state.num_mistakes_ = 0;
			init_state.player_turn_ = 0;

			auto deal_out_hand_to_player = [player=0](const auto begin) mutable
			{
				return std::for_each_n(begin, Configuration::hand_size, [p = player++](auto& dealt_card)
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

			init_state.next_card_to_draw = Configuration::hand_size * 2;

			game_states_.push_back(init_state);
		}

		template <typename Gen>
		void run(Gen&) //rng for seeding
		{
			while (true)
			{
				//std::vector<Configuration::actions<std::variant>> potential_actions_for_the_turn;

				hint<rank<2>> this_action{ 1 };

				this_action.validate(game_states_.back());

				game_states_.push_back(this_action.perform(game_states_.back()));

				discard this_action2{ 5 };

				this_action2.validate(game_states_.back());

				game_states_.push_back(this_action2.perform(game_states_.back()));
			}
		}
	};
}


//Test explicit instantiations

//Play action
template bool hanabi::play::is_playable<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;
template bool hanabi::play::validate<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;
template hanabi::game_state<hanabi::configuration::default_t> hanabi::play::perform<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;

//Discard action
template bool hanabi::discard::validate<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;
template hanabi::game_state<hanabi::configuration::default_t> hanabi::discard::perform<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;

//Hint action
template bool hanabi::hint<hanabi::color<0>>::validate<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;
template hanabi::game_state<hanabi::configuration::default_t> hanabi::hint<hanabi::color<0>>::perform<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;
template bool hanabi::hint<hanabi::rank<0>>::validate<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;
template hanabi::game_state<hanabi::configuration::default_t> hanabi::hint<hanabi::rank<0>>::perform<hanabi::configuration::default_t>(const hanabi::game_state<hanabi::configuration::default_t>&) const;

int main()
{
	using hanabi_game = hanabi::game<>;
	
	std::random_device rd;
	std::mt19937 gen(rd());

	hanabi_game game;

	game.init(gen);
	game.run(gen);
}
