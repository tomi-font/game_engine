#ifndef CPLAYER_HH
#define CPLAYER_HH

#include <SFML/Window/Keyboard.hpp>


class	CPlayer
{
public:

	enum	Control
	{
		Left,
		Right,
		Jump,
		COUNT
	};

	using Controls = std::array<sf::Keyboard::Key, Control::COUNT>;

	CPlayer(const Controls& controls) noexcept : m_controls(controls) {}

	const Controls&	getControls() const noexcept { return m_controls; }


private:
	
	const Controls	m_controls;
};

#endif