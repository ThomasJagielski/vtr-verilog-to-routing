#ifndef DEVICE_GRID
#define DEVICE_GRID

#include <string>
#include <vector>
#include "vtr_ndmatrix.h"
#include "physical_types.h"

///@brief s_grid_tile is the minimum tile of the fpga
struct t_grid_tile {
    t_physical_tile_type_ptr type = nullptr; ///<Pointer to type descriptor, NULL for illegal
    int width_offset = 0;                    ///<Number of grid tiles reserved based on width (right) of a block
    int height_offset = 0;                   ///<Number of grid tiles reserved based on height (top) of a block
    const t_metadata_dict* meta = nullptr;
};

class DeviceGrid {
  public:
    DeviceGrid() = default;
    DeviceGrid(std::string grid_name, std::vector<vtr::Matrix<t_grid_tile>> grid);
    DeviceGrid(std::string grid_name, std::vector<vtr::Matrix<t_grid_tile>> grid, std::vector<t_logical_block_type_ptr> limiting_res);

    const std::string& name() const { return name_; }

    size_t width(int layer_num = 0) const { return grid_[layer_num].dim_size(0); }
    size_t height(int layer_num = 0) const { return grid_[layer_num].dim_size(1); }

    //Note: supports 2-d indexing [0..width()-1][0..height()-1] yielding a t_grid_tile
    auto operator[](size_t index) const { return grid_[index]; }
    auto operator[](size_t index) { return grid_[index]; }

    const vtr::Matrix<t_grid_tile>& matrix(int layer_num = 0) const {
        return grid_[layer_num];
    }

    inline size_t grid_size(int layer_num = 0) const {
        return grid_[layer_num].size();
    }

    inline size_t grid_dim_size(int dim, int layer_num = 0) const {
        return grid_[layer_num].dim_size(dim);
    }


    void clear();

    size_t num_instances(t_physical_tile_type_ptr type, int layer_num = 0) const;

    /**
     * @brief Returns the block types which limits the device size (may be empty if
     *        resource limits were not considered when selecting the device).
     */
    std::vector<t_logical_block_type_ptr> limiting_resources() const { return limiting_resources_; }

    inline t_physical_tile_type_ptr get_physical_type(size_t x, size_t y, int layer_num = 0) const {
        return grid_[layer_num][x][y].type;
    }

    inline int get_width_offset(size_t x, size_t y, int layer_num = 0) const {
        return grid_[layer_num][x][y].width_offset;
    }

    inline int get_height_offset(size_t x, size_t y, int layer_num = 0) const {
        return grid_[layer_num][x][y].height_offset;
    }

    inline const t_metadata_dict* get_metadata(size_t x, size_t y, int layer_num = 0) const {
        return grid_[layer_num][x][y].meta;
    }

  private:
    void count_instances();

    std::string name_;

    int num_layers_ = 1;

    //Note that vtr::Matrix operator[] returns and intermediate type
    //which can be used or indexing in the second dimension, allowing
    //traditional 2-d indexing to be used
    std::vector<vtr::Matrix<t_grid_tile>> grid_;

    std::vector<std::map<t_physical_tile_type_ptr, size_t>> instance_counts_;

    std::vector<t_logical_block_type_ptr> limiting_resources_;
};

#endif
