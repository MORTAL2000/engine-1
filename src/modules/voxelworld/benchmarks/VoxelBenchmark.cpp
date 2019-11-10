#include "core/benchmark/AbstractBenchmark.h"
#include "voxelworld/WorldPager.h"
#include "voxel/PagedVolume.h"
#include "voxel/CubicSurfaceExtractor.h"
#include "voxelworld/BiomeManager.h"
#include "voxel/Constants.h"
#include "voxel/IsQuadNeeded.h"
#include "voxelformat/VolumeCache.h"

class PagedVolumeBenchmark: public core::AbstractBenchmark {
protected:
	voxelworld::BiomeManager _biomeManager;
	voxelformat::VolumeCache _volumeCache;

public:
	void onCleanupApp() override {
		_biomeManager.shutdown();
		_volumeCache.shutdown();
	}

	bool onInitApp() override {
		voxel::initDefaultMaterialColors();
		const io::FilesystemPtr& filesystem = io::filesystem();
		const std::string& luaBiomes = filesystem->load("biomes.lua");
		Log::info("%s", luaBiomes.c_str());
		_biomeManager.init(luaBiomes);
		_volumeCache.init();
		return true;
	}
};

BENCHMARK_DEFINE_F(PagedVolumeBenchmark, pageIn) (benchmark::State& state) {
	const uint16_t chunkSideLength = state.range(0);
	const uint32_t volumeMemoryMegaBytes = chunkSideLength * 2;
	voxelworld::WorldPager pager;
	pager.setSeed(0l);
	pager.setPersist(false);
	voxel::PagedVolume *volumeData = new voxel::PagedVolume(&pager, volumeMemoryMegaBytes * 1024 * 1024, chunkSideLength);
	const io::FilesystemPtr& filesystem = io::filesystem();
	const std::string& luaParameters = filesystem->load("worldparams.lua");
	pager.init(volumeData, &_biomeManager, &_volumeCache, luaParameters);
	const glm::ivec3 meshSize(16, 128, 16);
	int x = 0;
	while (state.KeepRunning()) {
		glm::ivec3 mins(x, 0, 0);
		x += meshSize.x;
		voxel::Region region(mins, mins + meshSize);
		voxel::Mesh mesh(0, 0, true);
		voxel::Mesh waterMesh(0, 0, true);
		voxel::extractAllCubicMesh(volumeData, region, &mesh, &waterMesh, voxel::IsQuadNeeded(), voxel::IsWaterQuadNeeded(), voxel::MAX_WATER_HEIGHT);
	}
	delete volumeData;
}

BENCHMARK_REGISTER_F(PagedVolumeBenchmark, pageIn)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_MAIN();