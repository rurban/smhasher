#include <cmath>
#include <vector>

double CalcStdv(std::vector<double> &values);
void FilterOutliers(std::vector<double> &values);

int main()
{
  std::vector<double> sample = {1., 2., 3.};
  if (std::fabs(CalcStdv(sample) - std::sqrt(2. / 3.)) > 1e-12) return 1;
  std::vector<double> singleton(1, 5.);
  FilterOutliers(singleton);
  if (singleton.size() != 1 || CalcStdv(singleton) != 0.) return 1;
  std::vector<double> equal(10, 2.);
  FilterOutliers(equal);
  if (equal.size() != 10) return 1;
  std::vector<double> outlier(99, 0.);
  outlier.push_back(1000.);
  FilterOutliers(outlier);
  if (outlier.size() != 99) return 1;
  std::vector<double> empty;
  return std::isnan(CalcStdv(empty)) ? 0 : 1;
}
