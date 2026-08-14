// Extra Class for PencilBeamScanning beam model

#include "TsPBSBeamModel.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::string ToLower(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value;
}

std::string Trim(const std::string& value)
{
	const auto first = value.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return "";
	const auto last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

std::vector<std::string> SplitCsv(const std::string& line)
{
	std::vector<std::string> fields;
	std::string field;
	std::istringstream in(line);
	while (std::getline(in, field, ','))
		fields.push_back(Trim(field));
	return fields;
}

int FindColumn(const std::vector<std::string>& header, const std::vector<std::string>& names)
{
	for (const auto& name : names) {
		for (std::size_t i = 0; i < header.size(); ++i) {
			if (ToLower(header[i]) == name)
				return static_cast<int>(i);
		}
	}
	return -1;
}

double ParseNumber(const std::string& token, std::size_t lineNumber, const std::string& field)
{
	if (token.empty())
		throw std::runtime_error("line " + std::to_string(lineNumber) + ": empty " + field);
	char* end = nullptr;
	const double value = std::strtod(token.c_str(), &end);
	if (end == token.c_str() || *end != '\0' || !std::isfinite(value))
		throw std::runtime_error("line " + std::to_string(lineNumber) + ": invalid " + field + " '" + token + "'");
	return value;
}

TsPBSBeamOptics InterpolateOptics(const TsPBSBeamOptics& a, const TsPBSBeamOptics& b, double energy)
{
	const double t = (energy - a.energyMeV) / (b.energyMeV - a.energyMeV);
	TsPBSBeamOptics out;
	out.energyMeV = energy;
	out.sigmaXMm = a.sigmaXMm + t * (b.sigmaXMm - a.sigmaXMm);
	out.sigmaXpRad = a.sigmaXpRad + t * (b.sigmaXpRad - a.sigmaXpRad);
	out.corrX = a.corrX + t * (b.corrX - a.corrX);
	out.sigmaYMm = a.sigmaYMm + t * (b.sigmaYMm - a.sigmaYMm);
	out.sigmaYpRad = a.sigmaYpRad + t * (b.sigmaYpRad - a.sigmaYpRad);
	out.corrY = a.corrY + t * (b.corrY - a.corrY);
	out.energySpreadPercent = a.energySpreadPercent + t * (b.energySpreadPercent - a.energySpreadPercent);
	return out;
}

}

TsPBSBeamModel::TsPBSBeamModel()
{}

bool TsPBSBeamModel::EnergiesMatch(double a, double b)
{
	return std::fabs(a - b) <= 1e-6 * std::max(1.0, std::max(std::fabs(a), std::fabs(b)));
}

void TsPBSBeamModel::Load(const std::string& fileName)
{
	fFileName = fileName;
	fRows.clear();

	std::ifstream in(fileName.c_str());
	if (!in)
		throw std::runtime_error("cannot open beam model file: " + fileName);

	std::string line;
	std::size_t lineNumber = 0;
	while (std::getline(in, line)) {
		++lineNumber;
		const std::string trimmed = Trim(line);
		if (trimmed.empty() || trimmed[0] == '#')
			continue;

		const auto header = SplitCsv(trimmed);
		const int eCol = FindColumn(header, {"energy", "energy_mev"});
		const int sxCol = FindColumn(header, {"sigma_x_mm", "sigmax"});
		const int sxpCol = FindColumn(header, {"sigma_xp_rad", "sigmaxprime"});
		const int cxCol = FindColumn(header, {"corr_x", "correlationx"});
		const int syCol = FindColumn(header, {"sigma_y_mm", "sigmay"});
		const int sypCol = FindColumn(header, {"sigma_yp_rad", "sigmayprime"});
		const int cyCol = FindColumn(header, {"corr_y", "correlationy"});
		const int esCol = FindColumn(header, {"energy_spread_percent", "energyspread"});
		if (eCol < 0 || sxCol < 0 || sxpCol < 0 || cxCol < 0 ||
			syCol < 0 || sypCol < 0 || cyCol < 0 || esCol < 0) {
			throw std::runtime_error(
				"beam model is missing required columns energy,sigma_x_mm,sigma_xp_rad,corr_x,"
				"sigma_y_mm,sigma_yp_rad,corr_y,energy_spread_percent: " + fileName);
		}

		while (std::getline(in, line)) {
			++lineNumber;
			const std::string row = Trim(line);
			if (row.empty() || row[0] == '#')
				continue;
			const auto fields = SplitCsv(row);
			const int need = std::max(std::max(std::max(eCol, sxCol), std::max(sxpCol, cxCol)),
				std::max(std::max(syCol, sypCol), std::max(cyCol, esCol)));
			if (static_cast<int>(fields.size()) <= need)
				throw std::runtime_error("beam model line " + std::to_string(lineNumber) + ": too few columns");

			TsPBSBeamOptics optics;
			optics.energyMeV = ParseNumber(fields[eCol], lineNumber, "energy");
			optics.sigmaXMm = ParseNumber(fields[sxCol], lineNumber, "sigma_x_mm");
			optics.sigmaXpRad = ParseNumber(fields[sxpCol], lineNumber, "sigma_xp_rad");
			optics.corrX = ParseNumber(fields[cxCol], lineNumber, "corr_x");
			optics.sigmaYMm = ParseNumber(fields[syCol], lineNumber, "sigma_y_mm");
			optics.sigmaYpRad = ParseNumber(fields[sypCol], lineNumber, "sigma_yp_rad");
			optics.corrY = ParseNumber(fields[cyCol], lineNumber, "corr_y");
			optics.energySpreadPercent = ParseNumber(fields[esCol], lineNumber, "energy_spread_percent");

			if (optics.energyMeV <= 0.)
				throw std::runtime_error("beam model line " + std::to_string(lineNumber) + ": energy must be positive");
			if (optics.sigmaXMm < 0. || optics.sigmaYMm < 0. || optics.sigmaXpRad < 0. || optics.sigmaYpRad < 0.)
				throw std::runtime_error("beam model line " + std::to_string(lineNumber) + ": sigmas must be >= 0");
			if (optics.corrX < -1. || optics.corrX > 1. || optics.corrY < -1. || optics.corrY > 1.)
				throw std::runtime_error("beam model line " + std::to_string(lineNumber) + ": correlation must be in [-1, 1]");
			if (optics.energySpreadPercent < 0.)
				throw std::runtime_error("beam model line " + std::to_string(lineNumber) + ": energy_spread_percent must be >= 0");

			for (const auto& existing : fRows) {
				if (EnergiesMatch(existing.energyMeV, optics.energyMeV))
					throw std::runtime_error("beam model has duplicate energy " + std::to_string(optics.energyMeV));
			}
			fRows.push_back(optics);
		}

		if (fRows.empty())
			throw std::runtime_error("beam model contains no rows: " + fileName);

		std::sort(fRows.begin(), fRows.end(),
			[](const TsPBSBeamOptics& a, const TsPBSBeamOptics& b) {
				return a.energyMeV < b.energyMeV;
			});
		return;
	}

	throw std::runtime_error("beam model has no header: " + fileName);
}

TsPBSBeamOptics TsPBSBeamModel::Lookup(double energyMeV, bool interpolate) const
{
	if (fRows.empty())
		throw std::runtime_error("beam model is empty");

	for (const auto& row : fRows) {
		if (EnergiesMatch(row.energyMeV, energyMeV))
			return row;
	}

	if (!interpolate) {
		throw std::runtime_error("spot energy " + std::to_string(energyMeV) +
			" MeV is not in the beam model (enable InterpolateBeamModel to interpolate)");
	}

	if (energyMeV < fRows.front().energyMeV || energyMeV > fRows.back().energyMeV) {
		throw std::runtime_error("spot energy " + std::to_string(energyMeV) +
			" MeV is outside the beam-model range [" +
			std::to_string(fRows.front().energyMeV) + ", " +
			std::to_string(fRows.back().energyMeV) + "] MeV");
	}

	for (std::size_t i = 1; i < fRows.size(); ++i) {
		if (energyMeV <= fRows[i].energyMeV)
			return InterpolateOptics(fRows[i - 1], fRows[i], energyMeV);
	}

	throw std::runtime_error("internal error interpolating beam model");
}
