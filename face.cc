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

int encode(int cubelet[], int len, int ori) {
  int res = 0;
  for (int i = 0; i < len; i++)
    res = N_COLORS * res + cubelet[mod(i - ori, len)];
  return res;
}

void maybeInit() {
  if (inited)
    return;

  for (int color = 0; color < N_COLORS; color++)
    colors[kColorNames[color]] = color;

  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int ori = 0; ori < 3; ori++)
      corners[encode(cornlets[corner], 3, ori)] = std::make_pair(corner, ori);
  }
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int ori = 0; ori < 2; ori++)
      edges[encode(edgelets[edge], 2, ori)] = std::make_pair(edge, ori);
  }

  inited = true;
}

bool faceToCubie(const std::string &s, CubieCube &cubiecube) {
  maybeInit();

  int facecube[N_FACELETS];
  for (int i = 0; i < N_FACELETS; i++) {
    if (colors.find(s[i]) == colors.end())
      return false;
    facecube[i] = colors[s[i]];
  }

  for (int corner = 0; corner < N_CORNERS; corner++) {
    int cornlet[3];
    for (int i = 0; i < 3; i++)
      cornlet[i] = facecube[cornlets[corner][i]];
    auto tmp = corners.find(encode(cornlet, 3, 0));
    if (tmp == corners.end())
      return false;
    cubiecube.cp[corner] = tmp->second.first;
    cubiecube.co[corner] = tmp->second.second;
  }

  for (int edge = 0; edge < N_EDGES; edge++) {
    int edgelet[2];
    for (int i = 0; i < 2; i++)
      edgelet[i] = facecube[edgelets[edge][i]];
    auto tmp = edges.find(encode(edgelet, 2, 0));
    if (tmp == edges.end())
      return false;
    cubiecube.ep[edge] = tmp->second.first;
    cubiecube.eo[edge] = tmp->second.second;
  }
 
  return true;
}

std::string cubieToFace(const CubieCube &cube) {
  maybeInit();

  std::string s;
  s.reserve(N_FACELETS);

  for (int corner = 0; corner < N_CORNERS; corner++) {
    for (int i = 0; i < 3; i++)
      s[cornlets[corner][i]] = kCornerNames[corner][mod(i - cube.co[i], 3)];
  }
  for (int edge = 0; edge < N_EDGES; edge++) {
    for (int i = 0; i < 2; i++)
      s[edgelets[edge][i]] = kEdgeNames[edge][mod(i - cube.co[i], 2)];
  }

  return s;
}

