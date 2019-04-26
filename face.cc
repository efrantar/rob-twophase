#include "face.h"

#include <string>
#include <unordered_map>
#include "misc.h"

int encode(const std::string &cubelet, int ori) {
  int res = 0;
  for (int i = 0; i < cubelet.size(); i++)
    res = N_COLORS * res + kNameToColor.at(cubelet[mod(i - ori, cubelet.size())]);
  return res;
}

auto createCorners() {
  std::unordered_map<int, std::pair<int, int>> corners;
  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int ori = 0; ori < 3; ori++)
      corners[encode(kCornerNames[corner], ori)] = std::make_pair(corner, ori);
  }
  return corners;
}

auto createEdges() {
  std::unordered_map<int, std::pair<int, int>> edges;
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int ori = 0; ori < 2; ori++)
      edges[encode(kEdgeNames[edge], ori)] = std::make_pair(edge, ori);
  }
  return edges;
}

auto corners = createCorners();
auto edges = createEdges();

int faceToCubie(const std::string &s, CubieCube &cube) {
  for (int i = 0; i < N_FACELETS; i++) {
    if (kNameToColor.find(s[i]) == kNameToColor.end())
      return 1;
    if ((i - 4) % 9 == 0 && kNameToColor.at(s[i]) != i / 9)
      return 2;
  }

  for (int corner = 0; corner < N_CORNERS; corner++) {
    char cornlet[3];
    for (int i = 0; i < 3; i++)
      cornlet[i] = s[kCornlets[corner][i]];
    auto tmp = corners.find(encode(std::string(cornlet, 3), 0));
    if (tmp == corners.end())
      return 3;
    cube.cp[corner] = tmp->second.first;
    cube.co[corner] = tmp->second.second;
  }

  for (int edge = 0; edge < N_EDGES; edge++) {
    char edgelet[2];
    for (int i = 0; i < 2; i++)
      edgelet[i] = s[kEdgelets[edge][i]];
    auto tmp = edges.find(encode(std::string(edgelet, 2), 0));
    if (tmp == edges.end())
      return 4;
    cube.ep[edge] = tmp->second.first;
    cube.eo[edge] = tmp->second.second;
  }
 
  return 0;
}

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
