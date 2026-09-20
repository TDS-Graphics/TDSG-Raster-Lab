#include "io/obj_loader.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static bool resolve_index(long raw, std::size_t count, std::uint32_t& out) {
    if (raw > 0) {
        const std::size_t index = raw - 1;
        if (index >= count) return false;
        out = static_cast<std::uint32_t>(index);
        return true;
    }
    if (raw < 0) {
        const long index = static_cast<long>(count) + raw;
        if (index < 0) return false;
        out = static_cast<std::uint32_t>(index);
        return true;
    }
    return false;
}

static bool parse_face_token(const std::string& token, std::uint32_t& vi, std::uint32_t& ti, std::uint32_t& ni,
                      std::size_t vcount, std::size_t tcount, std::size_t ncount) {
    long v = 0;
    const std::size_t first = token.find('/');
    if (first == std::string::npos) {
        try { v = std::stol(token); } catch (...) { return false; }
        if (!resolve_index(v, vcount, vi)) return false;
        ti = 0;
        ni = 0;
        return true;
    }

    try { v = std::stol(token.substr(0, first)); } catch (...) { return false; }

    const std::size_t second = token.find('/', first + 1);
    const std::string tex = (second == std::string::npos) ? token.substr(first + 1)
                                                          : token.substr(first + 1, second - first - 1);
    const std::string nor = (second == std::string::npos) ? std::string() : token.substr(second + 1);

    if (!resolve_index(v, vcount, vi)) return false;
    if (!tex.empty() && !resolve_index(std::stol(tex), tcount, ti)) return false;
    if (!nor.empty() && !resolve_index(std::stol(nor), ncount, ni)) return false;
    return true;
}

ObjLoadResult load_obj(const std::string& filename) {
    ObjLoadResult result;

    std::ifstream in(filename);
    if (!in.is_open()) {
        result.error = "cannot open '" + filename + "'";
        return result;
    }

    std::vector<Vec3f> positions;
    std::vector<Vec2f> texcoords;
    std::vector<Vec3f> normals;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<std::uint32_t> face;

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ls(line);
        std::string tag;
        ls >> tag;

        if (tag == "v") {
            Vec3f p;
            ls >> p.x >> p.y >> p.z;
            positions.push_back(p);
        } else if (tag == "vt") {
            Vec2f uv;
            ls >> uv.x >> uv.y;
            texcoords.push_back(uv);
        } else if (tag == "vn") {
            Vec3f n;
            ls >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (tag == "f") {
            face.clear();
            std::string token;
            while (ls >> token) {
                std::uint32_t vi = 0, ti = 0, ni = 0;
                if (!parse_face_token(token, vi, ti, ni, positions.size(), texcoords.size(), normals.size())) {
                    result.error = "malformed face on line " + std::to_string(line_number);
                    return result;
                }
                Vertex vertex;
                vertex.position = positions[vi];
                if (!texcoords.empty()) vertex.uv = texcoords[ti];
                if (!normals.empty()) vertex.normal = normals[ni];
                vertex.color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
                face.push_back(static_cast<std::uint32_t>(vertices.size()));
                vertices.push_back(vertex);
            }
            if (face.size() < 3) {
                result.error = "face with fewer than 3 vertices on line " + std::to_string(line_number);
                return result;
            }
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                indices.push_back(face[0]);
                indices.push_back(face[i]);
                indices.push_back(face[i + 1]);
            }
        }
    }

    if (vertices.empty() || indices.empty()) {
        result.error = "no triangles found in '" + filename + "'";
        return result;
    }

    result.mesh = Mesh(std::move(vertices), std::move(indices));
    if (normals.empty()) result.mesh.compute_normals();
    result.ok = true;
    return result;
}
