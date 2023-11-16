#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/features/gasd.h>

#include "assert.hpp"

#pragma once
class GASDDescriptor
{
    private:
        float descriptor_[512];
    public:
        constexpr static bool needsNormals() { return false; }
        constexpr static size_t size = sizeof(descriptor_);
        inline static std::string name = "gasd";

        int compute(const std::vector<glm::vec3>& points)
        {
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
            cloud->points.reserve(points.size());

            for(size_t i = 0; i < points.size(); ++i)
                cloud->points.push_back(pcl::PointXYZ(points[i].x, points[i].y, points[i].z));

            pcl::GASDEstimation<pcl::PointXYZ, pcl::GASDSignature512> gasd;
            gasd.setInputCloud(cloud);
            pcl::PointCloud<pcl::GASDSignature512> gasd_descriptor;
            gasd.compute(gasd_descriptor);

            ASSERT(sizeof(gasd_descriptor[0].histogram) == sizeof(descriptor_));
            memcpy(descriptor_, gasd_descriptor[0].histogram, sizeof(descriptor_));

            return 0;
        }

        const float* data() const
        {
            return descriptor_;
        }

        double distance(const GASDDescriptor& other) const
        {
            double distance = 0;
            for (uint32_t i = 0; i < 512; ++i)
                distance +=  abs(descriptor_[i] - other.data()[i]);
            return distance;
        }
};