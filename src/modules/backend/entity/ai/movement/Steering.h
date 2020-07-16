/**
 * @file
 * @brief Defines some basic movement algorithms like Wandering, Seeking and Fleeing.
 */
#pragma once

#include "backend/entity/ai/AI.h"
#include "backend/entity/ai/zone/Zone.h"
#include "backend/entity/ai/IAIFactory.h"
#include "backend/entity/ai/common/Math.h"
#include "backend/entity/ai/common/MoveVector.h"
#include "backend/entity/ai/common/MemoryAllocator.h"
#include "backend/entity/ai/ICharacter.h"

namespace backend {
namespace movement {

#define STEERING_FACTORY(SteeringName) \
public: \
	class Factory: public ISteeringFactory { \
	public: \
		SteeringPtr create (const SteeringFactoryContext *ctx) const override { \
			return std::make_shared<SteeringName>(ctx->parameters); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

#define STEERING_FACTORY_SINGLETON \
public: \
	class Factory: public ISteeringFactory { \
		SteeringPtr create (const SteeringFactoryContext */*ctx*/) const { \
			return get(); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief Steering interface
 */
class ISteering : public MemObject {
public:
	virtual ~ISteering() {}
	/**
	 * @brief Calculates the @c MoveVector
	 *
	 * @return If the @c MoveVector contains @c glm::vec3::VEC3_INFINITE as vector, the result should not be used
	 * because there was an error.
	 */
	virtual MoveVector execute (const AIPtr& ai, float speed) const = 0;
};

/**
 * @brief @c IFilter steering interface
 */
class SelectionSteering : public ISteering {
protected:
	glm::vec3 getSelectionTarget(const AIPtr& entity, size_t index) const {
		const FilteredEntities& selection = entity->getFilteredEntities();
		if (selection.empty() || selection.size() <= index) {
			return VEC3_INFINITE;
		}
		const Zone* zone = entity->getZone();
		const ai::CharacterId characterId = selection[index];
		const AIPtr& ai = zone->getAI(characterId);
		const ICharacterPtr character = ai->getCharacter();
		return character->getPosition();
	}

public:
	virtual ~SelectionSteering() {}
};

}
}
