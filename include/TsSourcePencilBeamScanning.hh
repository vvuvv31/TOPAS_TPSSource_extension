#ifndef TsSourcePencilBeamScanning_hh
#define TsSourcePencilBeamScanning_hh

#include "TsSource.hh"
#include "TsPBSBeamModel.hh"
#include "TsPBSCoordinateModel.hh"
#include "TsPBSSpotPlan.hh"

#include <vector>

struct TsPBSPreparedSpot {
	TsPBSSpot spot;
	G4long histories;
	TsPBSBeamOptics optics;
	TsPBSSourceRay ray;
};

class TsSourcePencilBeamScanning : public TsSource
{
public:
	TsSourcePencilBeamScanning(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	~TsSourcePencilBeamScanning();

	void ResolveParameters();

	const std::vector<TsPBSPreparedSpot>& PreparedSpots() const { return fPreparedSpots; }
	G4long TotalHistories() const { return fTotalHistories; }

private:
	TsPBSSpotPlan fPlan;
	TsPBSBeamModel fBeamModel;
	std::vector<TsPBSPreparedSpot> fPreparedSpots;
	G4long fTotalHistories;
};

#endif
