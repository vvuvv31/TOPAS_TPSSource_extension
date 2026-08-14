#ifndef TsPBSSpotPlan_hh
#define TsPBSSpotPlan_hh

#include <cstddef>
#include <string>
#include <vector>

struct TsPBSSpot {
	std::size_t id;
	double xIsoMm;
	double yIsoMm;
	double energyMeV;
	double weight;
};

class TsPBSSpotPlan
{
public:
	TsPBSSpotPlan();

	void Load(const std::string& fileName, bool skipZeroWeight);

	const std::vector<TsPBSSpot>& Spots() const { return fSpots; }
	std::size_t Size() const { return fSpots.size(); }
	const TsPBSSpot& At(std::size_t index) const;
	std::size_t ZeroWeightCount() const { return fZeroWeightCount; }
	const std::string& FileName() const { return fFileName; }

private:
	std::vector<TsPBSSpot> fSpots;
	std::size_t fZeroWeightCount;
	std::string fFileName;
};

#endif
