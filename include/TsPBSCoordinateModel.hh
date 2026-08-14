#ifndef TsPBSCoordinateModel_hh
#define TsPBSCoordinateModel_hh

#include <string>

// Maps TPS isocenter-plane (x, y) onto the source component frame.
//
// Default TPS convention (axis map and source-plane offset are applied here):
//   component origin = isocenter
//   component +X = TPS scan X
//   component +Y = beam (gantry 0: y- -> y+)
//   component +Z = TPS scan Y
//   source plane at y = -VirtualSourceToIsocenterDistance, aimed at the origin
//
// ComponentLocal leaves +Z as the beam so a user-supplied component
// rotation can remap the axes.

enum class TsPBSSpotConvention {
	ComponentLocal,
	TPS
};

struct TsPBSSourceRay {
	double xSrcMm;
	double ySrcMm;
	double zSrcMm;
	double thetaXRad;
	double thetaYRad;
	double dirX;
	double dirY;
	double dirZ;
	TsPBSSpotConvention convention;
};

class TsPBSCoordinateModel
{
public:
	static TsPBSSpotConvention ParseConvention(const std::string& name);

	static TsPBSSourceRay Compute(double xIsoMm, double yIsoMm,
		double virtualSadXMm, double virtualSadYMm,
		double sourceToIsocenterMm, TsPBSSpotConvention convention);

	static void DirectionFromAngles(const TsPBSSourceRay& ray,
		double thetaXRad, double thetaYRad,
		double& dirX, double& dirY, double& dirZ);
};

#endif
