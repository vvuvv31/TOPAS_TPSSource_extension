#ifndef TsGeneratorPencilBeamScanning_hh
#define TsGeneratorPencilBeamScanning_hh

#include "TsVGenerator.hh"

#include <cstddef>

class TsSourcePencilBeamScanning;

class TsGeneratorPencilBeamScanning : public TsVGenerator
{
public:
	TsGeneratorPencilBeamScanning(TsParameterManager* pM, TsGeometryManager* gM,
		TsGeneratorManager* pgM, G4String sourceName);
	~TsGeneratorPencilBeamScanning();

	void ResolveParameters();
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	void GeneratePrimaries(G4Event* anEvent);

private:
	void ResetScheduler();
	void AdvanceToNextSpotWithHistories();

	TsSourcePencilBeamScanning* fPBS;
	std::size_t fCurrentSpot;
	G4long fRemainingInSpot;
};

#endif
