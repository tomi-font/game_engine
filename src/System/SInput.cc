#include <System/SInput.hh>
#include <Component/CPlayer.hh>
#include <Component/CMove.hh>
#include <SFML/Window/Event.hpp>

// indices for m_groups
enum	G
{
	Player,
	COUNT
};

SInput::SInput()
{
	m_groups.reserve(G::COUNT);
	m_groups.emplace_back(C(Component::Player) | C(Component::Move));
}

static void	playerControls(sf::Keyboard::Key keyCode, const std::vector<Archetype*>& archs)
{
	if (!archs.empty())
	{
		Archetype*	arch = archs[0];
		const auto&	controls = arch->get<CPlayer>()[0].getControls();
		
		if (keyCode == controls[CPlayer::Left] || keyCode == controls[CPlayer::Right]
		|| keyCode == controls[CPlayer::Up] || keyCode == controls[CPlayer::Down])
		{
			CMove*	move = &arch->get<CMove>()[0];
			sf::Vector2i	direction(
				sf::Keyboard::isKeyPressed(controls[CPlayer::Right]) - sf::Keyboard::isKeyPressed(controls[CPlayer::Left]),
				sf::Keyboard::isKeyPressed(controls[CPlayer::Down]) - sf::Keyboard::isKeyPressed(controls[CPlayer::Up]));
			bool	moving = (direction.x || direction.y);

			if (moving)
				move->setVelocity(sf::Vector2f(direction * static_cast<int>(move->getSpeed())));
			move->setMoving(moving);
		}
	}
}

bool	SInput::readInput(sf::RenderWindow& window)
{
	bool	running = true;

	for (sf::Event event; window.pollEvent(event);)
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::Q || event.key.code == sf::Keyboard::Escape)
			{
				running = false;
				break;
			}
			// else: calling playerControls() below
		case sf::Event::KeyReleased:
			playerControls(event.key.code, m_groups[G::Player].archs);
			break;

		case sf::Event::Closed:
			running = false;
			break;
		}
	}

	return running;
}
