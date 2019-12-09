#include <random>
#include <variant>
#include <array>
#include <tuple>
#include <optional>

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

	namespace configuration
	{
		struct default
		{
			static constexpr auto num_players = 2;
			static constexpr auto hand_size = 5;
			static constexpr auto max_num_hints = 8;
			static constexpr auto starting_num_hints = 8;
			static constexpr auto max_num_mistakes = 3;


			template <template <typename... TArgs> typename Container> //for use with std::variant, std::tuple, etc.
			using ranks = Container<rank<0>, rank<1>, rank<2>, rank<3>, rank<4>>;

			template <template <typename... TArgs> typename Container>
			using colors = Container<color<0>, color<1>, color<2>, color<3>, color<4>, color<5>>;

			template <template <typename... TArgs> typename Container>
			using locations = Container<location::hand, location::draw_pile, location::discard_pile, location::in_play>;

			template <template <typename... TArgs> typename Container>
			using actions = Container<action<play>, action<discard>,
				hint<color<0>>, hint<color<1>>, hint<color<2>>, hint<color<3>>, hint<color<4>>,
				hint<rank<0>>, hint<rank<1>>, hint<rank<2>>, hint<rank<3>>, hint<rank<4>>>;

			static constexpr size_t deck_size() { return 50; }
		};
	}

	template <typename Configuration>
	struct card
	{
		typename Configuration::template ranks<std::variant> rank_;
		typename Configuration::template colors<std::variant> color_;
	};

	template <typename Property>
	struct possibility
	{
		bool possibile_;
	};

	template <typename... Properties>
	using possibility_tuple = std::tuple<possibility<Properties>...>;
	
	template <typename Configuration>
	struct knowledge
	{
		typename Configuration::template ranks<possibility_tuple> hinted_ranks_;
		typename Configuration::template colors<possibility_tuple> hinted_colors_;
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

	template <typename Action> struct action
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

	struct play
	{ 
		int card_;

		template <typename Configuration>
		constexpr bool is_playable(const game_state<Configuration>& source) const
		{
			const auto this_color = source.deck_.cards_[card_].card_.color_;
			Configuration::ranks<possibility_tuple> played_ranks_this_color;


		}

		template <typename Configuration>
		constexpr bool validate(const game_state<Configuration>& check) const
		{
			return std::get<location::hand>(check.deck_.cards_[card_].location_).player_ == check.player_turn_;
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
			}
		}
	};
	template <typename Property> struct hint { int player_; };
	struct discard { int card_; };

	template<typename Configuration = hanabi::configuration::default>
	class game
	{
		std::vector<game_state<Configuration>> game_states_;
	};
}


int main()
{
	using hanabi_game = hanabi::game<>;
	
	std::random_device rd;
	std::mt19937 gen(rd());

	hanabi_game game;
}