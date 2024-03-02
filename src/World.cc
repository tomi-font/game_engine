#include <World.hh>
#include <System/Types.hh>
#include <utility/variant/indexToCompileTime.hh>
#include <cstdint>
#include <SFML/Window/Event.hpp>

inline std::size_t std::hash<EntityContext>::operator()(const EntityContext& ec) const
{
	constexpr auto shift = 8 * (sizeof(std::size_t) - sizeof(ec.m_idx));
	static_assert(shift > 0);

	const std::size_t index = ec.m_idx;
	const std::uintptr_t archAddr = reinterpret_cast<std::uintptr_t>(ec.m_arch);

	return (index << shift) | archAddr;
}

World::World(sf::RenderWindow* window) :
	m_systems(std::tuple_size_v<Systems>),
	m_running(true),
	m_window(window)
{
	m_systems[SId<SInput>] = std::make_unique<SInput>();
	m_systems[SId<SPhysics>] = std::make_unique<SPhysics>();
	m_systems[SId<SRender>] = std::make_unique<SRender>();

	for (const auto& system: m_systems)
	{
		system->setEventManager(&m_eventManager);
		system->listenToEvents();
		system->setWindow(window);
	}

	// Restart the clock to not count the setup time.
	m_clock.restart();
}

void World::updateEntities()
{
	for (const EntityContext& entity : m_entitiesToRemove)
	{
		entity.m_arch->remove(entity);
	}
	m_entitiesToRemove.clear();

	for (const Template& temp : m_entitiesToInstantiate)
	{
		getArchetype(temp.comp())->instantiate(temp);
	}
	m_entitiesToInstantiate.clear();
}

void World::update()
{
	const float elapsedTime = m_clock.restart().asSeconds();

	// First remove and add existing entities' components.
	updateEntitiesComponents();

	// Then remove and instantiate the entities that are waiting for that.
	updateEntities();

	for (const auto& system : m_systems)
	{
		system->update(elapsedTime);
	}
}

Archetype* World::findArchetype(ComponentComposition comp)
{
	for (Archetype& arch : m_archs)
	{
		if (arch.comp() == comp)
			return &arch;
	}
	return nullptr;
}

Archetype* World::getArchetype(ComponentComposition comp)
{
	Archetype* arch = findArchetype(comp);
	if (arch)
		return arch;

	// If we didn't find an archetype matching the composition,
	// it means it doesn't exist so we have to create it.
	arch = &m_archs.emplace_back(comp, this);

	// Then we must iterate the systems to see whether they're interested.
	for (const auto& system : m_systems)
	{
		system->match(arch);
	}
	return arch;
}

EntityContext World::findEntity(const std::string& name)
{
	for (Archetype& arch : m_archs)
	{
		if (arch.has<CName>())
		{
			const std::span cNames = arch.getAll<CName>();
			const auto cNameIt = std::find(cNames.begin(), cNames.end(), name);
			
			if (cNameIt != cNames.end())
				return { arch.comp(), &arch, static_cast<unsigned int>(cNameIt - cNames.begin()) };
		}
	}
	return {};
}

void World::remove(EntityContext&& entity)
{
	m_componentsToRemove.erase(entity);
	m_componentsToAdd.erase(entity);
	m_entitiesToRemove.insert(std::move(entity));
}

void World::addComponentTo(EntityContext* entity, ComponentVariant&& component)
{	
	const ComponentId cid = static_cast<ComponentId>(component.index());
	assert(!entity->has(cid));
	entity->m_comp += cid;

	const EntityContext& ec = *entity;
	assert(!m_entitiesToRemove.contains(ec));

	if (m_componentsToRemove.contains(ec))
	{
		// A component remove() + add() + get() would return the one that was originally removed.
		assert(!m_componentsToRemove.at(ec).contains(cid));
	}
	
	const bool added = m_componentsToAdd[ec].emplace(cid, std::move(component)).second;
	assert(added);
}

void World::removeComponentFrom(EntityContext* entity, ComponentId cid)
{
	assert(entity->has(cid));
	entity->m_comp -= cid;

	const EntityContext& ec = *entity;
	assert(!m_entitiesToRemove.contains(ec));
	
	if (m_componentsToAdd.contains(ec))
	{
		const bool removed = m_componentsToAdd.at(ec).erase(cid);
		if (removed)
			return;
	}
	const bool removed = m_componentsToRemove[ec].insert(cid).second;
	assert(removed);
}

ComponentVariant* World::getStagedComponentOf(const EntityContext& entity, ComponentId cid)
{
	assert(entity.has(cid));
	assert(m_componentsToAdd.contains(entity));
	auto& components = m_componentsToAdd.at(entity);
	
	assert(components.contains(cid));
	return &components.at(cid);
}

void World::updateEntitiesComponents()
{
	std::unordered_set<EntityContext> entities;
	for (const auto& [entity, components] : m_componentsToAdd)
		entities.insert(entity);
	for (const auto& [entity, components] : m_componentsToRemove)
		entities.insert(entity);

	for (const EntityContext& entity : entities)
	{
		Archetype* origArch = entity.m_arch;
		const ComponentComposition origComp = origArch->comp();
	
		// TODO: Do this without going through a Template, by appending directly to the new Archetype?
		Template temp;

		const auto& componentsToRemove = m_componentsToRemove[entity];
	
		// Start with the entity's original components that are kept.
		for (ComponentId cid : origComp)
		{
			if (!componentsToRemove.contains(cid))
			{
				variantIndexToCompileTime<ComponentVariant>(cid,
					[&](auto I)
					{
						temp.add(origArch->getAll<std::tuple_element_t<I, Components>>()[entity.m_idx]);
					});
			}
		}
		// To which components to be added are added.
		for (auto& [cid, component] : m_componentsToAdd[entity])
			temp.add(std::move(component));

		// And replace the old entity by its updated version.
		origArch->remove(entity);
		getArchetype(temp.comp())->instantiate(temp);
	}
	m_componentsToAdd.clear();
	m_componentsToRemove.clear();
}
