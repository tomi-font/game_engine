#pragma once

#include <Component/CRender.hh>

class CHudRender : public CRender
{
public:

	using CRender::CRender;

	const sf::Vector2f& position() const { return m_vertices[0].position; }
};