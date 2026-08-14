// Extra Class for PencilBeamScanning coordinate model

#include "TsPBSCoordinateModel.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace {

std::string ToLower(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value;
}

}

TsPBSSpotConvention TsPBSCoordinateModel::ParseConvention(const std::string& name)
{
	const std::string lower = ToLower(name);
	if (lower == "componentlocal")
		return TsPBSSpotConvention::ComponentLocal;
	if (lower == "tps" || lower == "iec61217")
		return TsPBSSpotConvention::TPS;
	throw std::runtime_error(
		"unknown SpotCoordinateConvention '" + name + "' (use ComponentLocal or TPS)");
}

void TsPBSCoordinateModel::DirectionFromAngles(const TsPBSSourceRay& ray,
	double thetaXRad, double thetaYRad,
	double& dirX, double& dirY, double& dirZ)
{
	const double tx = std::tan(thetaXRad);
	const double ty = std::tan(thetaYRad);
	if (ray.convention == TsPBSSpotConvention::TPS) {
		// Built-in Rx(90): beam along component +Y, scan Y along +Z.
		const double norm = std::sqrt(tx * tx + 1.0 + ty * ty);
		dirX = tx / norm;
		dirY = 1.0 / norm;
		dirZ = ty / norm;
		return;
	}

	const double norm = std::sqrt(tx * tx + ty * ty + 1.0);
	dirX = tx / norm;
	dirY = ty / norm;
	dirZ = 1.0 / norm;
}

TsPBSSourceRay TsPBSCoordinateModel::Compute(double xIsoMm, double yIsoMm,
	double virtualSadXMm, double virtualSadYMm,
	double sourceToIsocenterMm, TsPBSSpotConvention convention)
{
	if (!(virtualSadXMm > 0.) || !(virtualSadYMm > 0.))
		throw std::runtime_error("VirtualScanningMagneticX/Y must be > 0");
	if (!(sourceToIsocenterMm > 0.))
		throw std::runtime_error("VirtualSourceToIsocenterDistance must be > 0");

	const double thetaX = std::atan(xIsoMm / virtualSadXMm);
	const double thetaY = std::atan(yIsoMm / virtualSadYMm);
	const double xSrc = xIsoMm * (virtualSadXMm - sourceToIsocenterMm) / virtualSadXMm;
	const double ySrc = yIsoMm * (virtualSadYMm - sourceToIsocenterMm) / virtualSadYMm;

	TsPBSSourceRay ray;
	ray.thetaXRad = thetaX;
	ray.thetaYRad = thetaY;
	ray.convention = convention;

	if (convention == TsPBSSpotConvention::TPS) {
		ray.xSrcMm = xSrc;
		ray.ySrcMm = -sourceToIsocenterMm;
		ray.zSrcMm = ySrc;
	} else {
		ray.xSrcMm = xSrc;
		ray.ySrcMm = ySrc;
		ray.zSrcMm = -sourceToIsocenterMm;
	}

	DirectionFromAngles(ray, thetaX, thetaY, ray.dirX, ray.dirY, ray.dirZ);
	return ray;
}
