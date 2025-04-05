#include "components/fluid_simulation.hpp"
#include <optional>

using namespace engine;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcCoreFactory;

void findPointsRecursive(const Alembic::Abc::IObject &obj) {
	const auto &header = obj.getHeader();
	std::string schema = header.getMetaData().get("schema");

	std::cout << "checking: " << obj.getFullName() << " (" << schema << ")\n";

	if (IPoints::matches(header)) {
		IPoints points(obj, Alembic::Abc::kWrapExisting);
		IPointsSchema::Sample sample;
		points.getSchema().get(sample);
		P3fArraySamplePtr positions = sample.getPositions();

		if (positions) {
			std::cout << "  >> found points: " << obj.getFullName() << "\n";
			std::cout << "  >> num points: " << positions->size() << "\n";
		} else {
			std::cout << "  >> points object has no position data\n";
		}
	}

	for (size_t i = 0; i < obj.getNumChildren(); ++i) {
		findPointsRecursive(obj.getChild(i));
	}
}

void extractPointsFrames(const Alembic::AbcGeom::IPoints &points,
						 std::vector<std::vector<Vec3f>> &allFrames) {
	const auto &schema = points.getSchema();
	size_t numSamples = schema.getNumSamples();

	for (size_t i = 0; i < numSamples; ++i) {
		Alembic::AbcGeom::IPointsSchema::Sample sample;
		schema.get(sample, i);

		auto positions = sample.getPositions();
		std::vector<Vec3f> frameVertices;

		if (positions) {
			for (size_t j = 0; j < positions->size(); ++j) {
				auto p = (*positions)[j];
				frameVertices.emplace_back(p.x, p.y, p.z);
			}
		}

		allFrames.push_back(std::move(frameVertices));
	}
	std::cout << "  >> got " << allFrames.size() << " frames from "
			  << points.getFullName() << std::endl;
}

void findAndExtractPointsRecursive(const Alembic::Abc::IObject &obj,
								   std::vector<std::vector<Vec3f>> &allFrames) {

	const auto &header = obj.getHeader();
	if (IPoints::matches(header)) {
		IPoints points(obj, Alembic::Abc::kWrapExisting);
		extractPointsFrames(points, allFrames);
		return;
	}

	for (size_t i = 0; i < obj.getNumChildren(); ++i) {
		findAndExtractPointsRecursive(obj.getChild(i), allFrames);
	}
}

std::optional<BakedPointDataComponent>
BakedPointDataComponent::create(const std::string &path) {
	IFactory factory;
	IFactory::CoreType coreType;
	IArchive archive = factory.getArchive(path, coreType);

	if (!archive.valid()) {
		std::cerr << "failed to open Alembic file: " << path << std::endl;
		return std::nullopt;
	}

	std::cout << "opened Alembic file: " << path << std::endl;

	return BakedPointDataComponent(archive);
}

BakedPointDataComponent::BakedPointDataComponent(const IArchive &archive) {
	std::vector<std::vector<Vec3f>> allFrames;
	findAndExtractPointsRecursive(archive.getTop(), allFrames);

	std::cout << "total frames read: " << allFrames.size() << std::endl;
	std::cout << "points in first frame: "
			  << (allFrames.empty() ? 0 : allFrames[0].size()) << std::endl;

	std::vector<Vec3f> allFrameData;
	currentFrame = 0;
	numPoints = allFrames[0].size();
	numFrames = allFrames.size();

	for (const auto &frame : allFrames) {
		allFrameData.insert(allFrameData.end(), frame.begin(), frame.end());
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glBufferData(GL_ARRAY_BUFFER, allFrameData.size() * sizeof(Vec3f),
				 allFrameData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void *)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BakedPointDataComponent::Bind() { glBindVertexArray(vao); }

void BakedPointDataComponent::Update(double dt) {
	timer += dt;
	if (timer >= (1. / 60.)) {
		currentFrame++;
		if (currentFrame >= numFrames)
			currentFrame = 0;
		timer = 0;
	}
}

void FluidSimulationComponent::Bind() {}
void FluidSimulationComponent::Update(double) {}
void FluidSimulationComponent::Draw() {}
