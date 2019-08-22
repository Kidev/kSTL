#pragma once

#include <QString>
#include <vector>
#include "Facet.hpp"

namespace kSTL {

/**
 * @brief The Mesh class
 *
 * This class loads, transforms, merges, saves and stores STL files
 * They can be ASCII, binary, and with Unicode names
 * It saves STLs as binaries, which is way more efficient than ASCII
 */
class Mesh {

public:

    /**
     * @brief Convinience class to handle STL objects, with Unicode filenames
     * @param path
     */
    Mesh(QString path);

    /**
     * @brief Init by copy
     * @param base
     */
    Mesh(Mesh* base);

    /**
     * @brief There is no pointers involved, the default copy operator works just fine
     */
    Mesh& operator=(const Mesh&) = default;

    /**
     * @brief Loads the stl file (binary or ascii). Unicode file names can be loaded
     * @param path
     */
    void load(QString path);

    /**
     * @brief Saves the mesh in binary stl format. Filename can be Unicode
     * @param path: the path to the file
     * @param header: the additional data you can add to the file (80 bytes max)
     */
    void save(QString path, const char* header = nullptr);

    /**
     * Destructor
     */
    ~Mesh();

    /**
     * @brief Resets the mesh to how it was when load() was called
     */
    void reset();

    /**
     * @brief Translates the mesh along x, y, z
     * @param x
     * @param y
     * @param z
     */
    void translate(float x, float y, float z);

    /**
     * @brief Rotates the mesh, with z->y->x order
     *          x rotates around the x axis, and so on
     * @param x
     * @param y
     * @param z
     */
    void rotate(float x, float y, float z);

    /**
     * @brief Rescale the mesh along x, y, z
     * @param x
     * @param y
     * @param z
     */
    void scale(float x, float y, float z);

    /**
     * @brief Adds the triangles of with to this.
     *        The header and history are kept from this, with's are discarded
     * @param The mesh to merge with
     */
    void merge(kSTL::Mesh* with);

    /**
     * @brief Returns the minimum x,y,z of the mesh
     * @return std::array<float, 3>
     */
    Facet::Point min() const;

    /**
     * @brief Returns the maximum x,y,z of the mesh
     * @return std::array<float, 3>
     */
    Facet::Point max() const;

    /**
     * @brief Returns the size of the mesh
     * @return std::array<float, 3>
     */
    Facet::Point size() const;

    /**
     * @brief TODO Fixes the mesh (holes, duplicates, normals, etc...)
     */
    void fix();

    /**
     * @brief Get the number of triangles in the mesh
     * @return
     */
    std::size_t num_triangles() const;

    /**
     * @brief Returns the coordinates of the specified vertex
     * @param ti: Triangle id (0 .. num_triangles - 1)
     * @param ci: Corner id (0 .. 2)
     * @return
     */
    const Facet::Point* coords_vertex(const std::size_t ti, const std::size_t ci) const;

    /**
     * @brief Returns the coordinates of the specified normal
     * @param ti: Triangle id (0 .. num_triangles - 1)
     * @return
     */
    const Facet::Point* coords_normal(const std::size_t ti) const;

protected:

    bool is_binary(QString path);
    void load_binary(QString path);
    void compute_stats();
    void load_ascii(QString path);

    std::vector<Facet> mFacets;
    std::vector<Facet> mOldFacets;

    QString mHeader;

    Facet::Point mMin = { std::numeric_limits<float>::max() };
    Facet::Point mMax = { std::numeric_limits<float>::min() };
    Facet::Point mSize = { 0.0f, 0.0f, 0.0f };

};

}
