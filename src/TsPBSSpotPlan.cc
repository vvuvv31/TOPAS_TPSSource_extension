// Extra Class for PencilBeamScanning spot plan

#include "TsPBSSpotPlan.hh"

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

}

TsPBSSpotPlan::TsPBSSpotPlan()
: fZeroWeightCount(0)
{}

void TsPBSSpotPlan::Load(const std::string& fileName, bool skipZeroWeight)
{
	fFileName = fileName;
	fSpots.clear();
	fZeroWeightCount = 0;

	std::ifstream in(fileName.c_str());
	if (!in)
		throw std::runtime_error("cannot open spot plan file: " + fileName);

	std::string line;
	std::size_t lineNumber = 0;
	while (std::getline(in, line)) {
		++lineNumber;
		const std::string trimmed = Trim(line);
		if (trimmed.empty() || trimmed[0] == '#')
			continue;

		const auto header = SplitCsv(trimmed);
		const int xCol = FindColumn(header, {"x", "x_mm"});
		const int yCol = FindColumn(header, {"y", "y_mm"});
		const int eCol = FindColumn(header, {"energy", "energy_mev"});
		const int wCol = FindColumn(header, {"weight"});
		const int idCol = FindColumn(header, {"spot_id", "id"});
		if (xCol < 0 || yCol < 0 || eCol < 0 || wCol < 0)
			throw std::runtime_error("spot plan is missing required columns x,y,energy,weight: " + fileName);

		std::size_t nextId = 0;
		while (std::getline(in, line)) {
			++lineNumber;
			const std::string row = Trim(line);
			if (row.empty() || row[0] == '#')
				continue;
			const auto fields = SplitCsv(row);
			const int need = std::max(std::max(xCol, yCol), std::max(eCol, wCol));
			if (static_cast<int>(fields.size()) <= need)
				throw std::runtime_error("line " + std::to_string(lineNumber) + ": too few columns");

			TsPBSSpot spot;
			spot.xIsoMm = ParseNumber(fields[xCol], lineNumber, "x");
			spot.yIsoMm = ParseNumber(fields[yCol], lineNumber, "y");
			spot.energyMeV = ParseNumber(fields[eCol], lineNumber, "energy");
			spot.weight = ParseNumber(fields[wCol], lineNumber, "weight");
			if (idCol >= 0 && idCol < static_cast<int>(fields.size()) && !fields[idCol].empty()) {
				const double rawId = ParseNumber(fields[idCol], lineNumber, "spot_id");
				if (rawId < 0 || rawId != std::floor(rawId))
					throw std::runtime_error("line " + std::to_string(lineNumber) + ": spot_id must be a non-negative integer");
				spot.id = static_cast<std::size_t>(rawId);
			} else {
				spot.id = nextId;
			}
			++nextId;

			if (spot.energyMeV <= 0.)
				throw std::runtime_error("line " + std::to_string(lineNumber) + ": energy must be positive");
			if (spot.weight < 0.)
				throw std::runtime_error("line " + std::to_string(lineNumber) + ": weight must be >= 0");
			if (spot.weight == 0.) {
				++fZeroWeightCount;
				if (skipZeroWeight)
					continue;
			}
			fSpots.push_back(spot);
		}
		if (fSpots.empty())
			throw std::runtime_error("spot plan contains no usable spots: " + fileName);
		return;
	}

	throw std::runtime_error("spot plan has no header: " + fileName);
}

const TsPBSSpot& TsPBSSpotPlan::At(std::size_t index) const
{
	return fSpots.at(index);
}
