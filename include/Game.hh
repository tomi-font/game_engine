#pragma once

#include <Entity/Factory.hh>
#include <World.hh>
#include <SFML/Graphics/RenderWindow.hpp>

class Game
{
public:

	Game();

	void create();
	void play();

	auto* world() { return &m_world; }
	auto* window() { return &m_window; }

private:

	// Factory must be destroyed after World so that its context is valid when components containing references get destroyed.
	EntityFactory m_factory;
	World m_world;

	sf::RenderWindow m_window;
};