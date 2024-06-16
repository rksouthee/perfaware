#include "json/json.h"

#include <cassert>
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

int main()
{
	std::ifstream ifs("haversine_data.json");
	const std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	const json::Json json_data = json::parse(content);
	assert(json_data.is_object());
	const json::Object& obj = std::get<json::Object>(json_data);
	assert(obj.size() == 1 && obj[0].first == "pairs" && obj[0].second.is_array());
	const json::Array& pairs = obj[0].second.get_array();
	const std::size_t number = pairs.size();

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
	std::cout << sum << std::endl;
}
