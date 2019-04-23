#include "face.h"

#include <string>
#include <unordered_map>
#include "misc.h"

bool inited = false;

int cornlets[][3] = {
  {8, 9, 20}, {6, 18, 38}, {0, 36, 47}, {2, 45, 11},
  {29, 26, 15}, {27, 44, 24}, {33, 53, 42}, {35, 17, 51}
};

int edgelets[][2] = {
  {5, 10}, {7, 19}, {3, 37}, {1, 46}, {32, 16}, {28, 25},
  {30, 43}, {34, 52}, {23, 12}, {21, 41}, {50, 39}, {48, 14}
};

std::unordered_map<char, int> colors;
std::unordered_map<int, std::pair<int, int>> corners;
std::unordered_map<int, std::pair<int, int>> edges;

int encode(std::string cubelet, int ori) {
  int res = 0;
  for (int i = 0; i < cubelet.size(); i++)
    res = N_COLORS * res + colors[cubelet[mod(i - ori, cubelet.size())]];
  return res;
}

void maybeInit() {
  if (inited)
    return;

  for (int color = 0; color < N_COLORS; color++)
    colors[kColorNames[color]] = color;

  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int ori = 0; ori < 3; ori++)
      corners[encode(kCornerNames[corner], ori)] = std::make_pair(corner, ori);
  }
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int ori = 0; ori < 2; ori++)
      edges[encode(kEdgeNames[edge], ori)] = std::make_pair(edge, ori);
  }

  inited = true;
}

int faceToCubie(const std::string &s, CubieCube &cube) {
  maybeInit();

  for (int i = 0; i < N_FACELETS; i++) {
    if (colors.find(s[i]) == colors.end())
      return 1;
  }

  for (int corner = 0; corner < N_CORNERS; corner++) {
    char cornlet[3];
    for (int i = 0; i < 3; i++)
      cornlet[i] = s[cornlets[corner][i]];
    auto tmp = corners.find(encode(std::string(cornlet, 3), 0));
    if (tmp == corners.end())
      return 2;
    cube.cp[corner] = tmp->second.first;
    cube.co[corner] = tmp->second.second;
  }

  for (int edge = 0; edge < N_EDGES; edge++) {
    char edgelet[2];
    for (int i = 0; i < 2; i++)
      edgelet[i] = s[edgelets[edge][i]];
    auto tmp = edges.find(encode(std::string(edgelet, 2), 0));
    if (tmp == edges.end())
      return 3;
    cube.ep[edge] = tmp->second.first;
    cube.eo[edge] = tmp->second.second;
  }
 
  return 0;
}

std::string cubieToFace(const CubieCube &cube) {
  maybeInit();

  char s[N_FACELETS];
  for (int color = 0; color < N_COLORS; color++)
    s[9 * color + 4] = kColorNames[color];
  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int i = 0; i < 3; i++)
      s[cornlets[corner][i]] = kCornerNames[cube.cp[corner]][mod(i - cube.co[corner], 3)];
  }
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int i = 0; i < 2; i++)
      s[edgelets[edge][i]] = kEdgeNames[cube.ep[edge]][mod(i - cube.eo[edge], 2)];
  }

  return std::string(s, N_FACELETS);
}

