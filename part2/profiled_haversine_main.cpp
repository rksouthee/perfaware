#include "json/json.h"
#include "platform_metrics.h"
#include "profiler.h"

#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

static double Square(double A)
{
	double Result = (A*A);
	return Result;
}

static double RadiansFromDegrees(double Degrees)
{
	double Result = 0.01745329251994329577 * Degrees;
	return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double EarthRadius)
{
	/* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
	   Instead, it attempts to follow, as closely as possible, the formula used in the real-world
	   question on which these homework exercises are loosely based.
	   */

	double lat1 = Y0;
	double lat2 = Y1;
	double lon1 = X0;
	double lon2 = X1;

	double dLat = RadiansFromDegrees(lat2 - lat1);
	double dLon = RadiansFromDegrees(lon2 - lon1);
	lat1 = RadiansFromDegrees(lat1);
	lat2 = RadiansFromDegrees(lat2);

	double a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
	double c = 2.0*asin(sqrt(a));

	double Result = EarthRadius * c;

	return Result;
}

void print_elapsed(std::uint64_t start, std::uint64_t end, const char* name, std::uint64_t total, std::uint64_t cpu_frequency)
{
	std::uint64_t elapsed = end - start;
	double percentage = 100.0 * elapsed / total;
	double seconds = static_cast<double>(elapsed) / cpu_frequency;
	std::cout << std::format("{}: {} cycles ({:.2f}%, {:.9f} seconds)\n", name, elapsed, percentage, seconds);
}

std::string read_file(const char* file_name)
{
	TIME_FUNCTION;
	std::ifstream ifs("haversine_data.json");
	const std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	return content;
}

const json::Json parse_json(const std::string& content)
{
	TIME_BYTES_PROCESSED("parse_json", content.size());
	const json::Json json_data = json::parse(content);
	return json_data;
}

double calculate_sum(const json::Array& pairs, std::size_t number)
{
	TIME_BYTES_PROCESSED("sum", pairs.size() * 4 * sizeof(double));
	double sum = 0.0;
	for (const json::Json& pair : pairs)
	{
		assert(pair.is_object());
		const json::Object& o = pair.get_object();
		assert(o.size() == 4);
		const double x0 = o[0].second.get_number();
		const double y0 = o[1].second.get_number();
		const double x1 = o[2].second.get_number();
		const double y1 = o[3].second.get_number();
		const double distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
		sum += (1.0 / number) * distance;
	}
	return sum;
}

int main()
{
	profiler::g_profiler.begin();
	const std::string content = read_file("haversine_data.json");
	const json::Json json_data = parse_json(content);

	assert(json_data.is_object());
	const json::Object& obj = std::get<json::Object>(json_data);
	assert(obj.size() == 1 && obj[0].first == "pairs" && obj[0].second.is_array());
	const json::Array& pairs = obj[0].second.get_array();
	const std::size_t number = pairs.size();

	std::cout << calculate_sum(pairs, number) << std::endl;

	profiler::g_profiler.end();
	profiler::g_profiler.dump();
}
