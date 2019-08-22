#include "kSTL.hpp"
#include <exception>
#include <sstream>
#include <QFile>
#include <QTextStream>
#include <iostream>

using namespace kSTL;

Mesh::Mesh(QString path)
{
    this->load(path);
}

Mesh::Mesh(Mesh* base) {
    if (base != nullptr) {
        this->mMax = base->mMax;
        this->mMin = base->mMin;
        this->mSize = base->mSize;
        this->mFacets = base->mFacets;
        this->mHeader = base->mHeader;
        this->mOldFacets = base->mOldFacets;
    }
}

Mesh::~Mesh() {

}

void Mesh::load(QString path)
{
    if (this->is_binary(path)) {
        this->load_binary(path);
    } else {
        this->load_ascii(path);
    }
    this->mOldFacets = this->mFacets;
    this->compute_stats();
}

bool Mesh::is_binary(QString path) {
    QFile fp(path);

    if (!fp.open(QIODevice::ReadOnly)) {
        fp.close();
        throw std::invalid_argument("Couldn't open file " + path.toStdString());
    }

    char charValues[128];
    fp.read(charValues, sizeof (charValues));
    fp.close();

    for (unsigned int i = 0; i < sizeof (charValues); ++i) {
        if (static_cast<int>(charValues[i]) < 0) {
            return true;
        }
    }
    return false;
}

void Mesh::load_ascii(QString path) {
    QFile fp(path);

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fp.close();
        throw std::invalid_argument("Couldn't open file " + path.toStdString());
    }

    QTextStream in(&fp);
    std::vector<std::string> tokens;
    int lineCount = 1;
    std::size_t maxTokensCount = 0;
    std::size_t curVertexCount = 0;
    float points[9] = { 0.0f };
    float normal[3] = { 0.0f };

    this->mFacets.clear();

    while (!in.atEnd()) {
        QString buffer = in.readLine();

        std::istringstream line(buffer.toStdString());
        std::size_t tokenCount = 0;

        while (!(line.eof() || line.fail())) {
            if (tokenCount >= maxTokensCount) {
                maxTokensCount = tokenCount + 1;
                tokens.resize(maxTokensCount);
            }
            line >> tokens[tokenCount];
            ++tokenCount;
        }

        if (tokenCount > 0) {
            std::string& tok = tokens[0];
            if (tok.compare("vertex") == 0) {

                if (tokenCount < 4) {
                    fp.close();
                    throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": vertex is not specified correctly at line " + std::to_string(lineCount));
                }

                if (curVertexCount >= 3) {
                    fp.close();
                    throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": too many vertex at line " + std::to_string(lineCount));
                }

                for (std::size_t i = 0; i < 3; ++i) {
                    points[(3 * curVertexCount) + i] = static_cast<float>(atof(tokens[i + 1].c_str()));
                }

                ++curVertexCount;
            }
            else if (tok.compare("facet") == 0) {

                if (tokenCount < 5) {
                    fp.close();
                    throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": triangle is not specified correctly at line " + std::to_string(lineCount));
                }

                if (tokens[1].compare("normal") != 0) {
                    fp.close();
                    throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": normal keyword missing at line " + std::to_string(lineCount));
                }

                for (std::size_t i = 0; i < 3; ++i) {
                    normal[i] = static_cast<float>(atof(tokens[i + 2].c_str()));
                }

                curVertexCount = 0;
            }
            else if (tok.compare("outer") == 0) {

                if (tokenCount < 2 || (tokens[1].compare("loop") != 0)) {
                    fp.close();
                    throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": expecting outer loop at line " + std::to_string(lineCount));
                }

            }
            else if (tok.compare("endfacet") == 0) {

                if (curVertexCount != 3) {
                    fp.close();
                    throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": bad number of vertices for face at line " + std::to_string(lineCount));
                }
                this->mFacets.push_back(Facet({ points[0], points[1], points[2] },
                                              { points[3], points[4], points[5] },
                                              { points[6], points[7], points[8] },
                                              { normal[0], normal[1], normal[2] },
                                              { 0, 0 }));

            }
            else if (tok.compare("endsolid") == 0) {

                QString header = buffer;
                header.remove(0, QString("endsolid ").length()).truncate(80);
                this->mHeader = header;

            }
            else if (tok.compare("solid") != 0 && tok.compare("endloop") != 0) {

                fp.close();
                throw std::invalid_argument("Bad ASCII format for " + path.toStdString() + ": bad keyword at line " + std::to_string(lineCount));

            }
        }
        lineCount++;
    }

    fp.close();
}

void Mesh::load_binary(QString path)
{
    QFile fp(path);

    if (!fp.open(QIODevice::ReadOnly)) {
        fp.close();
        throw std::invalid_argument("Couldn't open file " + path.toStdString());
    }

    char header[81] = { 0 };
    if (fp.read(header, 80) < 0) {
        fp.close();
        throw std::invalid_argument("Invalid header for file " + path.toStdString());
    }
    mHeader = QString(header);

    unsigned int numTris = 0;
    if (fp.read(reinterpret_cast<char*>(&numTris), 4) < 0) {
        fp.close();
        throw std::invalid_argument("Invalid triangles number for file " + path.toStdString());
    }

    this->mFacets.clear();
    this->mFacets.reserve(numTris);

    for (std::size_t tri = 0; tri < numTris; ++tri) {
        float data[12];
        char add[2];

        if (fp.read(reinterpret_cast<char*>(data), 12 * 4) < 0) {
            fp.close();
            throw std::invalid_argument("Invalid triangle in file " + path.toStdString());
        }

        if (fp.read(add, 2) < 0) {
            fp.close();
            throw std::invalid_argument("Error while parsing additional triangle data in binary stl file " + path.toStdString());
        }

        this->mFacets.push_back(Facet({ data[3], data[4], data[5] },
                                      { data[6], data[7], data[8] },
                                      { data[9], data[10], data[11] },
                                      { data[0], data[1], data[2] },
                                      { add[0], add[1] }));
    }
    fp.close();
}

void Mesh::save(QString path, const char* header)
{
    // TODO
}

void Mesh::reset()
{
    this->mFacets = this->mOldFacets;
    this->compute_stats();
}

void Mesh::translate(float x, float y, float z)
{
    for (std::size_t i = 0; i < this->mFacets.size(); ++i) {
        this->mFacets[i].translate(x, y, z);
    }
    this->mMin[0] += x;
    this->mMin[1] += y;
    this->mMin[2] += z;

    this->mMax[0] += x;
    this->mMax[1] += y;
    this->mMax[2] += z;
}

void Mesh::rotate(float x, float y, float z)
{
    for (std::size_t i = 0; i < this->mFacets.size(); ++i) {
        this->mFacets[i].rotateZYX(x, y, z);
    }
    this->compute_stats();
}

void Mesh::scale(float x, float y, float z)
{
    for (std::size_t i = 0; i < this->mFacets.size(); ++i) {
        this->mFacets[i].scale(x, y, z);
    }
    this->mMin[0] *= x;
    this->mMin[1] *= y;
    this->mMin[2] *= z;

    this->mMax[0] *= x;
    this->mMax[1] *= y;
    this->mMax[2] *= z;

    this->mSize[0] *= x;
    this->mSize[1] *= y;
    this->mSize[2] *= z;
}

void Mesh::fix()
{
    // TODO
}

void Mesh::merge(kSTL::Mesh* with) {
    this->mFacets.reserve(this->mFacets.size() + with->mFacets.size());
    this->mFacets.insert(this->mFacets.end(), with->mFacets.begin(), with->mFacets.end());
    this->compute_stats();
}

Facet::Point Mesh::min() const {
    return this->mMin;
}

Facet::Point Mesh::max() const {
    return this->mMax;
}

Facet::Point Mesh::size() const {
    return this->mSize;
}

std::size_t Mesh::num_triangles() const {
    return this->mFacets.size();
}

const Facet::Point* Mesh::coords_vertex(const std::size_t ti, const std::size_t ci) const {
    return this->mFacets[ti].get_corner(ci);
}

const Facet::Point* Mesh::coords_normal(const std::size_t ti) const {
    return this->mFacets[ti].get_normal();
}

void Mesh::compute_stats() {

    for (std::size_t i = 0; i < this->mFacets.size(); ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            this->mMin[j] = std::min(this->mMin[j], this->mFacets[i].get_min(j));
            this->mMax[j] = std::max(this->mMax[j], this->mFacets[i].get_max(j));
        }
    }

    this->mSize[0] = this->mMax[0] - this->mMin[0];
    this->mSize[1] = this->mMax[1] - this->mMin[1];
    this->mSize[2] = this->mMax[2] - this->mMin[2];
}
