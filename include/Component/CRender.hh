#ifndef CRENDER_HH
#define CRENDER_HH

#include <SFML/Graphics/Vertex.hpp>
#include <array>

class	CRender
{
public:

	CRender(const sf::Vector2f& pos, const sf::Vector2f& size) noexcept;

	const sf::Vertex*	getVertices() const noexcept { return &m_vertices[0]; }

	void	setPosition(const sf::Vector2f&) noexcept;

private:

	std::array<sf::Vertex, 4>	m_vertices;
};

#endif