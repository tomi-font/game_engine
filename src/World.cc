#include <World.hh>
#include <System/SInput.hh>
#include <System/SPhysics.hh>
#include <System/SRender.hh>

#include <SFML/Window/Event.hpp>

World::World() : m_running(true), m_systems(System::COUNT)
{
	m_systems[System::Input] = std::make_unique<SInput>();
	m_systems[System::Physics] = std::make_unique<SPhysics>();
	m_systems[System::Render] = std::make_unique<SRender>();
}

Archetype*	World::getArchetype(CsComp comp)
{
	for (Archetype& a : m_archs)
	{
		if (a.getComp() == comp)
			return &a;
	}

// if we didn't find an archetype matching the composition,
// it means it doesn't exist so we have to create it
// we use emplace_back() so that no useless copy is made
	Archetype*	arch = &m_archs.emplace_back(comp);

// then we must iterate through systems to see if they're interested
	for (const auto& system : m_systems)
	{
	// each system may have several groups of interest
		for (ComponentGroup& group : system->getGroups())
		{
			if ((comp & group.inc) == group.inc && !(comp & group.exc))
			{
				group.archs.push_back(arch);
			}
		}
	}
	return arch;
}

void	World::update(sf::RenderWindow& window)
{
	float	elapsedTime = m_clock.restart().asSeconds();

	for (const auto& system : m_systems)
	{
		if (!m_running)
			break;
		m_running = system->update(window, elapsedTime);
	}
}
