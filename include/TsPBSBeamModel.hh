#ifndef TsPBSBeamModel_hh
#define TsPBSBeamModel_hh

#include <string>
#include <vector>

struct TsPBSBeamOptics {
	double energyMeV;
	double sigmaXMm;
	double sigmaXpRad;
	double corrX;
	double sigmaYMm;
	double sigmaYpRad;
	double corrY;
	double energySpreadPercent;
};

class TsPBSBeamModel
{
public:
	TsPBSBeamModel();

	void Load(const std::string& fileName);

	TsPBSBeamOptics Lookup(double energyMeV, bool interpolate) const;
	const std::vector<TsPBSBeamOptics>& Rows() const { return fRows; }
	const std::string& FileName() const { return fFileName; }

private:
	std::vector<TsPBSBeamOptics> fRows;
	std::string fFileName;

	static bool EnergiesMatch(double a, double b);
};

#endif
