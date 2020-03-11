/**
 * @file
 */

#include "ClientEntityRenderer.h"
#include "video/Renderer.h"
#include "frontend/ClientEntity.h"
#include "frontend/Colors.h"
#include "core/Trace.h"
#include "core/Color.h"
#include "core/GLM.h"
#include "core/collection/List.h"
#include "core/ArrayLength.h"
#include "voxel/MaterialColor.h"
#include "video/Camera.h"
#include "render/Shadow.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace frontend {

ClientEntityRenderer::ClientEntityRenderer() :
		_skeletonShadowMapShader(shader::SkeletonshadowmapShader::getInstance()) {
}

void ClientEntityRenderer::construct() {
	_shadowMap = core::Var::getSafe(cfg::ClientShadowMap);
}

bool ClientEntityRenderer::init() {
	if (!_chrShader.setup()) {
		Log::error("Failed to setup the post skeleton shader");
		return false;
	}
	if (!_skeletonShadowMapShader.setup()) {
		Log::error("Failed to init skeleton shadowmap shader");
		return false;
	}
	const int shaderMaterialColorsArraySize = lengthof(shader::SkeletonData::MaterialblockData::materialcolor);
	const int materialColorsArraySize = (int)voxel::getMaterialColors().size();
	if (shaderMaterialColorsArraySize != materialColorsArraySize) {
		Log::error("Shader parameters and material colors don't match in their size: %i - %i",
				shaderMaterialColorsArraySize, materialColorsArraySize);
		return false;
	}

	shader::SkeletonData::MaterialblockData materialBlock;
	memcpy(materialBlock.materialcolor, &voxel::getMaterialColors().front(), sizeof(materialBlock.materialcolor));
	_materialBlock.create(materialBlock);

	{
		video::ScopedShader scoped(_chrShader);
		_chrShader.setDiffuseColor(diffuseColor);
		_chrShader.setAmbientColor(ambientColor);
		_chrShader.setFogcolor(clearColor);
		_chrShader.setNightColor(nightColor);
		_chrShader.setMaterialblock(_materialBlock);
		_chrShader.setShadowmap(video::TextureUnit::One);
	}

	return true;
}

void ClientEntityRenderer::shutdown() {
	_chrShader.shutdown();
	_skeletonShadowMapShader.shutdown();
}

void ClientEntityRenderer::update(const glm::vec3& focusPos, float seconds) {
	_focusPos = focusPos;
	_seconds = seconds;
}

void ClientEntityRenderer::renderShadows(const core::List<ClientEntity*>& entities, render::Shadow& shadow) {
	_skeletonShadowMapShader.activate();
	shadow.render([this, entities] (int i, const glm::mat4& lightViewProjection) {
		_skeletonShadowMapShader.setLightviewprojection(lightViewProjection);
		for (const auto& ent : entities) {
			_skeletonShadowMapShader.setBones(ent->bones()._items);
			_skeletonShadowMapShader.setModel(ent->modelMatrix());
			const uint32_t numIndices = ent->bindVertexBuffers(_chrShader);
			video::drawElements<animation::IndexType>(video::Primitive::Triangles, numIndices);
			ent->unbindVertexBuffers();
		}
		return true;
	}, true);
	_skeletonShadowMapShader.deactivate();
}

int ClientEntityRenderer::renderEntityDetails(const core::List<ClientEntity*>& entities, const video::Camera& camera) {
	if (entities.empty()) {
		return 0;
	}
	alignas(16) static const struct {
		glm::vec3 start;
		glm::vec3 end;
		glm::vec4 color;
	} verticesAxis[] = {
		{glm::vec3(0.0f), glm::right, core::Color::Red},
		{glm::vec3(0.0f), glm::up, core::Color::Green},
		{glm::vec3(0.0f), glm::forward, core::Color::Blue}
	};
	int drawCallsEntities = 0;
#if 0
	for (frontend::ClientEntity* ent : entities) {
		const glm::mat4 model = glm::rotate(glm::translate(ent->position()), ent->orientation(), glm::up);
		// TODO: draw health bar
		// TODO: draw debug orientation
	}
#endif
	return drawCallsEntities;
}

int ClientEntityRenderer::renderEntities(const core::List<ClientEntity*>& entities, const glm::mat4& viewProjectionMatrix, const glm::vec4& clipPlane, const render::Shadow& shadow) {
	if (entities.empty()) {
		return 0;
	}
	core_trace_gl_scoped(ClientEntityRendererEntities);

	int drawCallsEntities = 0;

	video::enable(video::State::DepthTest);
	video::ScopedShader scoped(_chrShader);
	_chrShader.setFogrange(_fogRange);
	_chrShader.setFocuspos(_focusPos);
	_chrShader.setLightdir(shadow.sunDirection());
	_chrShader.setTime(_seconds);
	_chrShader.setClipplane(clipPlane);
	_chrShader.setViewprojection(viewProjectionMatrix);

	const bool shadowMap = _shadowMap->boolVal();
	if (shadowMap) {
		_chrShader.setDepthsize(glm::vec2(shadow.dimension()));
		_chrShader.setCascades(shadow.cascades());
		_chrShader.setDistances(shadow.distances());
	}
	for (frontend::ClientEntity* ent : entities) {
		// TODO: apply the clipping plane to the entity frustum culling
		_chrShader.setModel(ent->modelMatrix());
		core_assert_always(_chrShader.setBones(ent->bones()._items));
		const uint32_t numIndices = ent->bindVertexBuffers(_chrShader);
		++drawCallsEntities;
		video::drawElements<animation::IndexType>(video::Primitive::Triangles, numIndices);
		ent->unbindVertexBuffers();
	}
	return drawCallsEntities;
}

}