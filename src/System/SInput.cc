#include <System/SInput.hh>
#include <Component/CPlayer.hh>
#include <Component/CMove.hh>
#include <Component/CRigidbody.hh>
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

static void	playerControls(sf::Keyboard::Key keyCode, Archetype* arch)
{
	const auto&	controls = arch->get<CPlayer>()[0].getControls();
	
	if (keyCode == controls[CPlayer::Left] || keyCode == controls[CPlayer::Right])
	{
		CMove*	move = &arch->get<CMove>()[0];
		int		direction = sf::Keyboard::isKeyPressed(controls[CPlayer::Right]) - sf::Keyboard::isKeyPressed(controls[CPlayer::Left]);

		if (direction)
			move->setVelocity(sf::Vector2f(static_cast<float>(direction * move->getSpeed()), 0.f));
		move->setMoving(direction);
	}
	else if (keyCode == controls[CPlayer::Jump])
	{
		CRigidbody*	rig = &arch->get<CRigidbody>()[0];

		if (rig->isGrounded())
		{
			rig->setGrounded(false);
			rig->setVelocity(-1000);
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
			playerControls(event.key.code, m_groups[G::Player].archs[0]);
			break;

		case sf::Event::Closed:
			running = false;
			break;
		}
	}

	return running;
}