#pragma once

// Converts an input Creature JSON into a Creature FlatData Binary file
bool ConvertToFlatData(const std::string& json_filename_in,
	const std::string& flat_filename_out);