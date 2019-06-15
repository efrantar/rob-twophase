#include "face.h"

#include <string>
#include <unordered_map>

int mod(int a, int m) {
  return a > 0 ? a % m : (a % m + m) % m;
}

int encode(const std::string &cubelet, int ori) {
  int res = 0;
  for (int i = 0; i < cubelet.size(); i++)
    res = N_COLORS * res + kNameToColor.at(cubelet[mod(i - ori, cubelet.size())]);
  return res;
}

// Map code to cubie & ori
std::unordered_map<int, std::pair<int, int>> corners;
std::unordered_map<int, std::pair<int, int>> edges;

static bool init() {
  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int ori = 0; ori < 3; ori++)
      corners[encode(kCornerNames[corner], ori)] = std::make_pair(corner, ori);
  }
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int ori = 0; ori < 2; ori++)
      edges[encode(kEdgeNames[edge], ori)] = std::make_pair(edge, ori);
  }
  return true;
}
static bool inited = init();

int faceToCubie(const std::string &s, CubieCube &cube) {
  for (int i = 0; i < N_FACELETS; i++) {
    if (kNameToColor.find(s[i]) == kNameToColor.end())
      return 1; // invalid color
    if ((i - 4) % 9 == 0 && kNameToColor.at(s[i]) != i / 9)
      return 2; // invalid center facelet (they are always fixed)
  }

  for (int corner = 0; corner < N_CORNERS; corner++) {
    char cornlet[3];
    for (int i = 0; i < 3; i++)
      cornlet[i] = s[kCornlets[corner][i]];
    auto tmp = corners.find(encode(std::string(cornlet, 3), 0));
    if (tmp == corners.end())
      return 3; // invalid corner cubie
    cube.cp[corner] = tmp->second.first;
    cube.co[corner] = tmp->second.second;
  }

  for (int edge = 0; edge < N_EDGES; edge++) {
    char edgelet[2];
    for (int i = 0; i < 2; i++)
      edgelet[i] = s[kEdgelets[edge][i]];
    auto tmp = edges.find(encode(std::string(edgelet, 2), 0));
    if (tmp == edges.end())
      return 4; // invalid edge cubie
    cube.ep[edge] = tmp->second.first;
    cube.eo[edge] = tmp->second.second;
  }
 
  return 0;
}

// Assumes `cube` to be valid
std::string cubieToFace(const CubieCube &cube) {
  char s[N_FACELETS];

  for (int color = 0; color < N_COLORS; color++)
    s[9 * color + 4] = kColorNames[color];
  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int i = 0; i < 3; i++)
      s[kCornlets[corner][i]] = kCornerNames[cube.cp[corner]][mod(i - cube.co[corner], 3)];
  }
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int i = 0; i < 2; i++)
      s[kEdgelets[edge][i]] = kEdgeNames[cube.ep[edge]][mod(i - cube.eo[edge], 2)];
  }

  return std::string(s, N_FACELETS);
}
