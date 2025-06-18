#include "voronoicell.h"
#include "voronoidiagram.h"

#include <cmath>
#include <omp.h>
#include <unordered_map>
#include <vector>
#include <iostream>


struct Moments {
  float moment00;
  float moment10;
  float moment01;
  float moment11;
  float moment20;
  float moment02;
};

struct LocalAccum {
  uint32_t area = 0;
  float sumDensity = 0.0f;
  Moments m{};
};

std::vector<VoronoiCell> accumulateCells(const IndexMap& map,
                                         const QImage& density) {
  // compute voronoi cell moments
  std::vector<VoronoiCell> cells = std::vector<VoronoiCell>(map.count());
  std::vector<Moments> moments = std::vector<Moments>(map.count());

  #pragma omp parallel
  {

    //int id = omp_get_thread_num();
    //std::cout << "Hello from thread " << id << "\n";

    // Thread-local accumulation map
    std::unordered_map<uint32_t, LocalAccum> local;

    #pragma omp for nowait
    for (int x = 0; x < map.width; ++x) {
      for (int y = 0; y < map.height; ++y) {
        uint32_t index = map.get(x, y);

        QRgb densityPixel = density.pixel(x, y);
        float densityVal = std::max(1.0f - qGray(densityPixel) / 255.0f,
                                    std::numeric_limits<float>::epsilon());

        LocalAccum& acc = local[index];
        acc.area++;
        acc.sumDensity += densityVal;

        acc.m.moment00 += densityVal;
        acc.m.moment10 += x * densityVal;
        acc.m.moment01 += y * densityVal;
        acc.m.moment11 += x * y * densityVal;
        acc.m.moment20 += x * x * densityVal;
        acc.m.moment02 += y * y * densityVal;
      }
    }

    // Merge thread-local results into global arrays
    #pragma omp critical
    {
      for (const auto& [index, acc] : local) {
        cells[index].area += acc.area;
        cells[index].sumDensity += acc.sumDensity;

        Moments& m = moments[index];
        m.moment00 += acc.m.moment00;
        m.moment10 += acc.m.moment10;
        m.moment01 += acc.m.moment01;
        m.moment11 += acc.m.moment11;
        m.moment20 += acc.m.moment20;
        m.moment02 += acc.m.moment02;
      }
    }
  }

  // compute cell quantities
  for (size_t i = 0; i < cells.size(); ++i) {
    VoronoiCell& cell = cells[i];
    if (cell.sumDensity <= 0.0f) continue;

    auto [m00, m10, m01, m11, m20, m02] = moments[i];

    // centroid
    cell.centroid.setX(m10 / m00);
    cell.centroid.setY(m01 / m00);

    // orientation
    float x = m20 / m00 - cell.centroid.x() * cell.centroid.x();
    float y = 2.0f * (m11 / m00 - cell.centroid.x() * cell.centroid.y());
    float z = m02 / m00 - cell.centroid.y() * cell.centroid.y();
    cell.orientation = std::atan2(y, x - z) / 2.0f;

    cell.centroid.setX((cell.centroid.x() + 0.5f) / density.width());
    cell.centroid.setY((cell.centroid.y() + 0.5f) / density.height());
  }
  return cells;
}
