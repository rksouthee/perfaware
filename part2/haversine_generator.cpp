#include <cxxopts.hpp>

#include <format>
#include <fstream>
#include <iostream>
#include <random>

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

namespace
{
	struct Cluster_info
	{
		int count;
		int size;
		double x;
		double y;
		double radius_x;
		double radius_y;
	};

	struct Position
	{
		double x0;
		double y0;
		double x1;
		double y1;
	};

	const double cluster_radius = 20.0;
	const double cluster_x_min = -180.0;
	const double cluster_x_max = 180.0;
	const double cluster_y_min = -90.0;
	const double cluster_y_max = 90.0;
	const double earth_radius = 6372.8;

	double random_degree(std::mt19937& gen, double center, double radius)
	{
		std::uniform_real_distribution<double> dist(center - radius, center + radius);
		return dist(gen);
	}

	Position generate_random_data_cluster(std::mt19937& gen, std::uniform_real_distribution<double>& x_dist, std::uniform_real_distribution<double>& y_dist, Cluster_info& cluster)
	{
		static std::uniform_real_distribution<double> s_cluster_dist(-cluster_radius, cluster_radius);
		static std::uniform_real_distribution<double> s_cluster_radius_x(0, cluster_x_max);
		static std::uniform_real_distribution<double> s_cluster_radius_y(0, cluster_y_max);

		if (cluster.count == 0)
		{
			cluster.count = cluster.size;
			cluster.x = x_dist(gen);
			cluster.y = y_dist(gen);
			cluster.radius_x = s_cluster_radius_x(gen);
			cluster.radius_y = s_cluster_radius_y(gen);
		}
		--cluster.count;
		const double x0 = std::clamp(random_degree(gen, cluster.x, cluster.radius_x), cluster_x_min, cluster_x_max);
		const double y0 = std::clamp(random_degree(gen, cluster.y, cluster.radius_y), cluster_y_min, cluster_y_max);
		const double x1 = std::clamp(random_degree(gen, cluster.x, cluster.radius_x), cluster_x_min, cluster_x_max);
		const double y1 = std::clamp(random_degree(gen, cluster.y, cluster.radius_y), cluster_y_min, cluster_y_max);
		return { x0, y0, x1, y1 };
	}

	Position generate_random_data_uniform(std::mt19937& gen, std::uniform_real_distribution<double>& x_dist, std::uniform_real_distribution<double>& y_dist, Cluster_info&)
	{
		const double x0 = x_dist(gen);
		const double y0 = y_dist(gen);
		const double x1 = x_dist(gen);
		const double y1 = y_dist(gen);

		return { x0, y0, x1, y1 };
	}
}

int main(int argc, char** argv)
{
	cxxopts::Options options("Haversine Generator", "Generate random haversine data");
	options.add_options()
		("t,type", "Type of data to generate", cxxopts::value<std::string>())
		("s,seed", "Seed for random number generator", cxxopts::value<int>())
		("n,number", "Number of data points to generate", cxxopts::value<int>())
		("h,help", "Print help");

	cxxopts::ParseResult result = options.parse(argc, argv);
	std::ofstream json("haversine_data.json");
	std::ofstream haversine("haversine_data.bin", std::ios::binary);
	json << "{\n\t\"pairs\": [\n";
	const int number = result["n"].as<int>();
	const auto generate_random_data = result["t"].as<std::string>() == "cluster" ? generate_random_data_cluster : generate_random_data_uniform;
	double sum = 0.0;
	Cluster_info cluster = {};
	if (number > 0)
	{
		cluster.size = 1 + (number / 64);
		std::mt19937 gen(result["s"].as<int>());
		std::uniform_real_distribution<double> x_dist(cluster_x_min, cluster_x_max);
		std::uniform_real_distribution<double> y_dist(cluster_y_min, cluster_y_max);
		Position position = generate_random_data(gen, x_dist, y_dist, cluster);
		json << std::format("\t\t{{\"x0\": {}, \"y0\": {}, \"x1\": {}, \"y1\": {}}}", position.x0, position.y0, position.x1, position.y1);
		double distance = ReferenceHaversine(position.x0, position.y0, position.x1, position.y1, earth_radius);
		sum += (1.0 / number) * distance;

		for (int i = 1; i < number; i++)
		{
			json << ",\n";
			position = generate_random_data(gen, x_dist, y_dist, cluster);
			distance = ReferenceHaversine(position.x0, position.y0, position.x1, position.y1, earth_radius);
			haversine.write(reinterpret_cast<const char*>(&distance), sizeof(double));
			json << std::format("\t\t{{\"x0\": {}, \"y0\": {}, \"x1\": {}, \"y1\": {}}}", position.x0, position.y0, position.x1, position.y1);
			sum += (1.0 / number) * distance;
		}
	}
	std::cout << "Expected sum: " << sum << std::endl;
	json << "\n\t]\n}\n";
}
